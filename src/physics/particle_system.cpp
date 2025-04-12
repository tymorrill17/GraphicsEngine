#include "physics/particle_system.h"
#include <random>

static const glm::vec2 down{ 0.0f, -0.1f };
static const double pi = 3.14159265358979323846;
static bool usePredictedPositions = true;

static long double norm(glm::vec2 v) {
	return glm::sqrt(v.x * v.x + v.y * v.y);
}

ParticleSystem2D::ParticleSystem2D(
	GlobalParticleInfo& particleInfo, 
	GlobalPhysicsInfo& physicsInfo,
	BoundingBox& box,
	InputManager& inputManager,
	Hand* hand
	) :
	_globalParticleInfo(particleInfo),
	_globalPhysics(physicsInfo),
	_bbox(box),
	_inputManager(inputManager),
	_interactionHand(hand),
	_simulationPaused(false),
	_doOneFrame(false) {

	_particles = new Particle2D[MAX_PARTICLES];
	_densities = new float[MAX_PARTICLES];
	_predictedParticlePositions = new glm::vec2[MAX_PARTICLES];
	_particleIndices = new uint32_t[MAX_PARTICLES];
	_spatialLookup = new uint32_t[MAX_PARTICLES];
	_startIndices = new uint32_t[MAX_PARTICLES];
	// Initialize all entries to 0 in case we add more
	for (int i = 0; i < MAX_PARTICLES; i++) {
		_densities[i] = 0.f;
		_predictedParticlePositions[i] = { 0.f, 0.f };
		_particleIndices[i] = i;
		_spatialLookup[i] = 0;
		_startIndices[i] = INT_MAX;
	}
	arrangeParticles();
	assignInputEvents();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
	delete _densities;
	delete _particleIndices;
	delete _spatialLookup;
	delete _startIndices;
}

static glm::ivec2 getGridCell(glm::vec2 position, int cellSize) {
	int cellX = static_cast<int>(position.x / cellSize);
	int cellY = static_cast<int>(position.y / cellSize);
	//std::cout << "Grid Cell Coordinates: (" << cellX << ", " << cellY << ")" << std::endl;
	return glm::vec2{ cellX, cellY };
}

static uint32_t hashGridCell(glm::ivec2 gridCell, uint32_t hashSize) {
	const uint32_t p1 = 73856093;
	const uint32_t p2 = 19349663;
	// const int p3 = 83492791;

	return (static_cast<uint32_t>(gridCell.x) * p1 + static_cast<uint32_t>(gridCell.y) * p2) % hashSize;
}

void ParticleSystem2D::arrangeParticles() {
	// TODO: Add these to an ImGui window later to play with on the fly
	float spacing = _globalParticleInfo.radius + _globalParticleInfo.spacing;
	glm::vec2 offset = glm::vec2();

	// Random number generator for randomizing velocity (or position)
	//std::default_random_engine generator;
	//std::uniform_real_distribution<double> distributiony(_bbox.bottom, _bbox.top);
	//std::uniform_real_distribution<double> distributionx(_bbox.left, _bbox.right);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_globalParticleInfo.numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;
		//_particles[i].position.x = distributionx(generator);
		//_particles[i].position.y = distributiony(generator);

		// Initialize the color of the particle to the default
		_particles[i].color = glm::vec4{ _globalParticleInfo.defaultColor[0], _globalParticleInfo.defaultColor[1], _globalParticleInfo.defaultColor[2], _globalParticleInfo.defaultColor[3] };

		// Set a random starting velocity
		// _particles[i].velocity = glm::vec2{ distribution(generator), distribution(generator) };
		_particles[i].velocity = glm::vec2{ 0.f, 0.f };
	}
}

static const int numThreads = 16;

void ParticleSystem2D::update() {
	if (_simulationPaused && !_doOneFrame) {
		return;
	}

	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	float predictionStep = 1.f / 120.f; // Used to gain some stability with the position-prediction code. I should refine this later on.

	// Parallel batching setup
	std::vector<int> batchSizes;
	batchSizes.reserve(numThreads);
	int batchSize = _globalParticleInfo.numParticles / numThreads;
	int oddBatchOut = _globalParticleInfo.numParticles - (numThreads - 1) * batchSize; // In case numThreads doesn't divide numParticles evenly
	for (int i = 0; i < numThreads-1; i++) {
		batchSizes.push_back(batchSize);
	}
	batchSizes.push_back(oddBatchOut);

	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			applyGravity(i, subDeltaTime);
			_predictedParticlePositions[i] = _particles[i].position + _particles[i].velocity * predictionStep;
		}

		// Update the spatial lookup arrays for use in calculating densities and forces
		updateSpatialLookup();

		// Calculate the density of the fluid at each particle and populate _densities[]
		//calculateParticleDensities();
		calculateParticleDensitiesParallel(batchSizes);

		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();

		// For each particle, first apply acceleration to the velocity
		//for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			// getAcceleration applies gravity, interaction force, and pressure force at once
			//_particles[i].velocity += getForces(i) * subDeltaTime;
		//}
		applyForcesToVelocityParallel(batchSizes, subDeltaTime);
		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();

		// Then apply velocity to position
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].position += _particles[i].velocity * subDeltaTime;
		}

		// Resolve collisions between particles
		// resolveParticleCollisions();

		// Resolve collisions with the walls of the bounding box
		resolveBoundaryCollisions();
	}
	frameDone();
}

void ParticleSystem2D::applyGravity(int particleIndex, float deltaTime) {
	_particles[particleIndex].velocity += _globalPhysics.gravity * down * deltaTime;
}

void ParticleSystem2D::resolveBoundaryCollisions() {
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		if (_particles[i].position.y < (_bbox.bottom + _globalParticleInfo.radius)) {
			_particles[i].position.y = _bbox.bottom + _globalParticleInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * _globalPhysics.boundaryDampingFactor;
		}
		else if (_particles[i].position.y > (_bbox.top - _globalParticleInfo.radius)) {
			_particles[i].position.y = _bbox.top - _globalParticleInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * _globalPhysics.boundaryDampingFactor;
		}
		if (_particles[i].position.x > (_bbox.right - _globalParticleInfo.radius)) {
			_particles[i].position.x = _bbox.right - _globalParticleInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * _globalPhysics.boundaryDampingFactor;
		}
		else if (_particles[i].position.x < (_bbox.left + _globalParticleInfo.radius)) {
			_particles[i].position.x = _bbox.left + _globalParticleInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * _globalPhysics.boundaryDampingFactor;
		}
	}
}

float ParticleSystem2D::calculateDensity(glm::vec2 position) {
	float density = 0.0f;
	// Use the locations of each particle to calculate the density at position, with the smoothing function lessening the impact of particles further away
	loopThroughNearbyPoints(position, [&](glm::vec2 dist, int particleIndex) {
		float squareDst = glm::dot(dist, dist);
		density += SmoothingKernels2D::smooth(squareDst, _globalPhysics.densitySmoothingRadius);
	});
	return density;
}

void ParticleSystem2D::calculateParticleDensities() {
	// We want to calculate the density at each particle location all at once.
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		if (usePredictedPositions) {
			_densities[i] = calculateDensity(_predictedParticlePositions[i]);
		} else {
			_densities[i] = calculateDensity(_particles[i].position);
		}
	}
}

void ParticleSystem2D::calculateParticleDensitiesParallel(std::vector<int> batchSizes) {
	// We want to calculate the density at each particle location all at once.
	int start = 0;
	int end = 0;
	for (int i = 0; i < numThreads; i++) {
		start = end;
		end = start + batchSizes[i];
		_futures.push_back(std::async(std::launch::async, [&](int startIndex, int endIndex) {
			for (int i = startIndex; i < endIndex; i++) {
				if (usePredictedPositions) {
					_densities[i] = calculateDensity(_predictedParticlePositions[i]);
				}
				else {
					_densities[i] = calculateDensity(_particles[i].position);
				}
			}
		}, start, end));
	}
}

glm::vec2 ParticleSystem2D::getForces(int particleIndex) {
	static Timer& timer = Timer::getTimer();

	// initialize each acceleration type
	glm::vec2 handAcceleration{ 0.f, 0.f };
	glm::vec2 pressureAcceleration{ 0.f, 0.f };

	// Input actions modify gravity
	if (_interactionHand->isInteracting()) {
		float interactionStrength = _interactionHand->action() == HandAction::pulling ? _interactionHand->strengthFactor : -_interactionHand->strengthFactor;
		// Hand is interacting, so find the vector from the hand to the particle and find its squared distance
		glm::vec2 particleToHand = _interactionHand->position() - _particles[particleIndex].position;
		float sqrDst = glm::dot(particleToHand, particleToHand);

		// If particle is in hand radius, change acceleration on particle
		if (sqrDst < _interactionHand->radius * _interactionHand->radius) {
			float dst = glm::sqrt(sqrDst);
			// Adding acceleration based on how far away the hand is... Could potentially use one of our smoothing functions for this
			float centerFactor = 1 - dst / _interactionHand->radius;
			particleToHand = particleToHand / dst; // Normalize the direction vector
			handAcceleration += (particleToHand * interactionStrength - _particles[particleIndex].velocity) * centerFactor;
		}
	}

	// Get force due to pressure and convert it to acceleration by dividing by density
	pressureAcceleration = calculatePressureForce(particleIndex) / _densities[particleIndex];
	//std::cout << "density after pressure force calculation: " << _densities[particleIndex] << std::endl;

	//std::cout << "gravityAcceleration: " << gravityAcceleration.x << ", " << gravityAcceleration.y << std::endl;
	//std::cout << "handAcceleration: " << handAcceleration.x << ", " << handAcceleration.y << std::endl;
	//std::cout << "pressureAcceleration: " << pressureAcceleration.x << ", " << pressureAcceleration.y << std::endl;
	return handAcceleration + pressureAcceleration;
}

void ParticleSystem2D::applyForcesToVelocityParallel(std::vector<int> batchSizes, float deltaTime) {
	int start = 0;
	int end = 0;
	for (int i = 0; i < numThreads; i++) {
		start = end;
		end = start + batchSizes[i];
		_futures.push_back(std::async(std::launch::async, [&](int startIndex, int endIndex) {
			for (int i = startIndex; i < endIndex; i++) {
				// getAcceleration applies gravity, interaction force, and pressure force at once
				_particles[i].velocity += getForces(i) * deltaTime;
			}
		}, start, end));
	}
}

static glm::vec2 getRandomDirection() {
	// Random number generator for randomizing velocity (or position)
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);

	return glm::normalize(glm::vec2{ distribution(generator), distribution(generator) });
}

float ParticleSystem2D::getPressure(float density) {
	return (density - _globalPhysics.restDensity) * _globalPhysics.pressureConstant;
}

float ParticleSystem2D::getSharedPressure(float density, float otherDensity) {
	float pressure = getPressure(density);
	float otherPressure = getPressure(otherDensity);
	return (pressure + otherPressure) * 0.5f;
}

glm::vec2 ParticleSystem2D::calculatePressureForce(int index) {
	glm::vec2 force{ 0.0f, 0.0f };
	glm::vec2 pos = usePredictedPositions ? _predictedParticlePositions[index] : _particles[index].position;
	// We are finding a field quantity like density, so we use the SPH equation. This involves looping over each particle that contributes to the quantity
	loopThroughNearbyPoints(pos, [&](glm::vec2 dist, int particleIndex) {
		if (index == particleIndex) return; // The particle itself doesn't contribute to the pressure force it feels

		float squareDst = glm::dot(dist, dist);
		// If particles are on top of each other, pick a random normal direction
		glm::vec2 direction = (squareDst == 0.0f) ? getRandomDirection() : dist / glm::sqrt(squareDst);

		// The pressure force needs to follow newton's third law, so instead of using the particles full pressure, take the average between particle index and particle j
		// Then multiply with the opposite direction to
		force += getSharedPressure(_densities[index], _densities[particleIndex]) * direction * SmoothingKernels2D::spikeyDerivative(squareDst, _globalPhysics.densitySmoothingRadius) / _densities[particleIndex];
	});
	return force;
}

static const std::vector<glm::ivec2> gridCellOffsets {
	{1, 1}, {1, 0}, {1, -1},
	{0, 1}, {0, -1}, {0, 0},
	{-1, 0}, {-1, 1}, {-1, -1}
};

void ParticleSystem2D::loopThroughNearbyPoints(glm::vec2 particlePosition, std::function<void(glm::vec2, uint32_t)> callback) {
	// Get the center grid cell
	glm::ivec2 center = getGridCell(particlePosition, _globalPhysics.densitySmoothingRadius);
	float squareSmoothingRadius = _globalPhysics.densitySmoothingRadius * _globalPhysics.densitySmoothingRadius;

	for (auto& offset : gridCellOffsets) {
		uint32_t gridKey = hashGridCell(center + offset, _globalParticleInfo.numParticles);
		uint32_t cellStartIndex = _startIndices[gridKey];

		// Loop through the rest of the particles in the grid cell
		for (int i = cellStartIndex; i < _globalParticleInfo.numParticles; i++) {
			if (_spatialLookup[i] != gridKey) break;

			uint32_t particleIndex = _particleIndices[i];
			glm::vec2 dist{ 0.f, 0.f };
			if (usePredictedPositions) {
				dist = _predictedParticlePositions[particleIndex] - particlePosition;
			} else {
				dist = _particles[particleIndex].position - particlePosition;
			}
			float squareDst = glm::dot(dist, dist);
			if (squareDst <= squareSmoothingRadius) {
				callback(dist, particleIndex);
			}
		}
	}
}

void ParticleSystem2D::resolveParticleCollisions() {
	// Brute-force collision detection
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		for (int j = i + 1; j < _globalParticleInfo.numParticles; j++) {

			// Calculate the normalized vector pointing from particle i to j
			glm::vec2 itojDirection = _particles[j].position - _particles[i].position;
			float distance = norm(itojDirection); // Distance between the two particles

			// Check to see if the particles collide
			if (distance < 2.0f * _globalParticleInfo.radius) {
				itojDirection = itojDirection / distance; // normalize the vector

				// Update the colliding particles' positions
				float posCorrection = 0.5f * (2.0f * _globalParticleInfo.radius - distance); // how much to move the particles after colliding
				_particles[i].position += -posCorrection * itojDirection;
				_particles[j].position += posCorrection * itojDirection;

				// Update the colliding particles' velocities
				// Compute the velocities in the direction of the collision
				float v1 = glm::dot(_particles[i].velocity, itojDirection);
				float v2 = glm::dot(_particles[j].velocity, itojDirection);
				// Update the particles' velocities
				_particles[i].velocity += ((0.5f * (v1 + v2 - (v1 - v2) * _globalPhysics.collisionDampingFactor)) - v1) * itojDirection;
				_particles[j].velocity += ((0.5f * (v1 + v2 - (v2 - v1) * _globalPhysics.collisionDampingFactor)) - v2) * itojDirection;
			}
		}
	}
}

void ParticleSystem2D::sortSpatialArrays() {
	
	int numParticles = _globalParticleInfo.numParticles;
	uint32_t* indices = new uint32_t[numParticles];

	// Fill indices array
	for (int i = 0; i < numParticles; i++) {
		indices[i] = i;
	}

	// Sort the indices array based on the given lambda function comparing the spatial lookup array
	std::sort(indices, indices + numParticles, [&](uint32_t i, uint32_t j) {
			return _spatialLookup[i] < _spatialLookup[j];
		});

	// Create temporary arrays to store sorted results
	uint32_t* sortedParticleIndices = new uint32_t[numParticles];
	uint32_t* sortedSpatialLookup = new uint32_t[numParticles];

	for (int i = 0; i < numParticles; i++) {
		sortedParticleIndices[i] = _particleIndices[indices[i]];
		sortedSpatialLookup[i] = _spatialLookup[indices[i]];
	}

	for (int i = 0; i < numParticles; i++) {
		_particleIndices[i] = sortedParticleIndices[i];
		_spatialLookup[i] = sortedSpatialLookup[i];
	}
	
	delete[] indices;
	delete[] sortedParticleIndices;
	delete[] sortedSpatialLookup;
}

void ParticleSystem2D::updateSpatialLookup() {
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// First, get the spatial grid cell index and its hash value
		glm::ivec2 gridCellIndex{ 0, 0 };
		if (usePredictedPositions) {
			gridCellIndex = getGridCell(_predictedParticlePositions[i], _globalPhysics.densitySmoothingRadius);
		} else {
			gridCellIndex = getGridCell(_particles[i].position, _globalPhysics.densitySmoothingRadius);
		}
		uint32_t gridCellHashValue = hashGridCell(gridCellIndex, _globalParticleInfo.numParticles);

		_particleIndices[i] = i;
		_spatialLookup[i] = gridCellHashValue;
		_startIndices[i] = INT_MAX; // reset the start indices
	}

	// Sort _particleIndices and _spatialLookup based on _spatialLookup
	sortSpatialArrays();

	// Calculate the start indices for each non-empty grid cell
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		uint32_t gridKey = _spatialLookup[i];
		uint32_t prevGridKey = i == 0 ? INT_MAX : _spatialLookup[i - 1];
		if (gridKey != prevGridKey) {
			_startIndices[gridKey] = i;
		}
	}
}

void ParticleSystem2D::assignInputEvents() {
	if (!_interactionHand) return;
	_inputManager.addListener(InputEvent::leftMouseDown, [&]() {
		_interactionHand->setAction(HandAction::pushing);
	});
	_inputManager.addListener(InputEvent::leftMouseUp, [&]() {
		_interactionHand->setAction(HandAction::idle);
	});
	_inputManager.addListener(InputEvent::rightMouseUp, [&]() {
		_interactionHand->setAction(HandAction::idle);
	});
	_inputManager.addListener(InputEvent::rightMouseDown, [&]() {
		_interactionHand->setAction(HandAction::pulling);
	});
	_inputManager.addListener(InputEvent::spacebarDown, [&]() {
		_simulationPaused = _simulationPaused ? false : true;
	});
	_inputManager.addListener(InputEvent::rightArrowDown, [&]() {
		if (_simulationPaused) {
			proceedFrame();
		}
	});
}

void ParticleSystem2D::proceedFrame() {
	_doOneFrame = true;
}

void ParticleSystem2D::frameDone() {
	_doOneFrame = false;
}

// ----------------------------------------------- SMOOTHING KERNELS --------------------------------------------- //

float SmoothingKernels2D::smooth(float squareDst, float smoothingRadius) {
	if (squareDst > smoothingRadius*smoothingRadius)
		return 0;

	return 4.f / (pi * pow(smoothingRadius, 8)) * pow(smoothingRadius*smoothingRadius - squareDst, 3);
}

float SmoothingKernels2D::smoothDerivative(float squareDst, float smoothingRadius) {
	float rmag = sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;

	return -24.f / (pi * pow(smoothingRadius, 8)) * rmag * pow(smoothingRadius * smoothingRadius - squareDst, 2);
}

float SmoothingKernels2D::spikey(float squareDst, float smoothingRadius) {
	float rmag = glm::sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;

	return 10.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 3);
}

float SmoothingKernels2D::spikeyDerivative(float squareDst, float smoothingRadius) {
	float rmag = glm::sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;
	
	return -30.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 2);
}


