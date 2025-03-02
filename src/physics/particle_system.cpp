#include "physics/particle_system.h"
#include <random>

static const glm::vec2 down{ 0.0f, -0.1f };
static const double pi = 3.14159265358979323846;

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
	_interactionHand(hand) {

	_particles = new Particle2D[MAX_PARTICLES];
	_densities = new float[MAX_PARTICLES];
	// Initialize all entries to 0 in case we add more
	for (int i = 0; i < MAX_PARTICLES; i++) {
		_densities[i] = 0.f;
	}
	arrangeParticles();
	assignInputEvents();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
	delete _densities;
}

void ParticleSystem2D::arrangeParticles() {
	// TODO: Add these to an ImGui window later to play with on the fly
	float spacing = _globalParticleInfo.radius + _globalParticleInfo.spacing;
	glm::vec2 offset = glm::vec2();

	// Random number generator for randomizing velocity (or position)
	//std::default_random_engine generator;
	//std::uniform_real_distribution<double> distribution(-1, 1);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_globalParticleInfo.numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;

		// Initialize the color of the particle to the default
		_particles[i].color = glm::vec4{ _globalParticleInfo.defaultColor[0], _globalParticleInfo.defaultColor[1], _globalParticleInfo.defaultColor[2], _globalParticleInfo.defaultColor[3] };

		// Set a random starting velocity
		// _particles[i].velocity = glm::vec2{ distribution(generator), distribution(generator) };
		_particles[i].velocity = glm::vec2{ 0.f, 0.f };
	}
}

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		// Calculate the density of the fluid at each particle
		calculateParticleDensities();

		// For each particle, first apply acceleration to velocity
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].velocity += getAcceleration(i) * subDeltaTime;
		}

		// Apply pressure force
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].velocity += calculatePressureForce(i) * subDeltaTime / _densities[i];
		}

		// Then apply velocity to position
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].position += _particles[i].velocity * subDeltaTime;
		}

		// Resolve collisions between particles
		// resolveParticleCollisions();

		// Resolve collisions with the walls of the bounding box
		resolveBoundaryCollisions();


	}
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
	for (int j = 0; j < _globalParticleInfo.numParticles; j++) {
		density += SmoothingKernels2D::smooth(position - _particles[j].position, _globalPhysics.densitySmoothingRadius);
	}
	return density;
}

void ParticleSystem2D::calculateParticleDensities() {
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		_densities[i] = calculateDensity(_particles[i].position);
	}
}

glm::vec2 ParticleSystem2D::getAcceleration(int particleIndex) {
	static Timer& timer = Timer::getTimer();

	glm::vec2 gravityAcceleration = _globalPhysics.gravity * down;
	glm::vec2 handAcceleration{ 0.f, 0.f };
	glm::vec2 pressureAcceleration{ 0.f, 0.f };

	// Input actions modify gravity
	if (_interactionHand->isInteracting()) {
		float interactionStrength = _interactionHand->action() == HandAction::pulling ? _interactionHand->strengthFactor : -_interactionHand->strengthFactor;
		// Hand is interacting, so find the vector from the hand to the particle
		glm::vec2 particleToHand = _interactionHand->position() - _particles[particleIndex].position;
		float sqrDst = glm::dot(particleToHand, particleToHand);

		// If particle is in hand radius, change acceleration on particle
		if (sqrDst < _interactionHand->radius * _interactionHand->radius) {
			float dst = glm::sqrt(sqrDst);
			float centerFactor = 1 - dst / _interactionHand->radius;
			particleToHand = particleToHand / dst;

			handAcceleration += (particleToHand * interactionStrength - _particles[particleIndex].velocity) * centerFactor;
		}
	}

	// Get acceleration due to pressure
	pressureAcceleration = calculatePressureForce(particleIndex) / _densities[particleIndex];

	return gravityAcceleration + handAcceleration + pressureAcceleration;
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
	glm::vec2 accel{ 0.0f, 0.0f };
	float pressure = getPressure(_densities[index]);
	for (int j = 0; j < _globalParticleInfo.numParticles; j++) {
		if (index == j) continue;

		glm::vec2 itojDistance = _particles[index].position - _particles[j].position;
		float distance = glm::length(itojDistance);
		// If particles are on top of each other, pick a random normal direction
		glm::vec2 itojDirection = (distance == 0.0f) ? getRandomDirection() : itojDistance / distance;
		accel += -getSharedPressure(_densities[index], _densities[j]) * itojDirection * SmoothingKernels2D::spikeyDerivative(itojDistance, _globalPhysics.densitySmoothingRadius) / _densities[j];
	}
	return accel;
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

void ParticleSystem2D::assignInputEvents() {
	if (!_interactionHand) return;
	_inputManager.addListener(InputEvent::leftMouseDown, [&]() {
		_interactionHand->setAction(HandAction::pushing);
		float densityAtMouse = calculateDensity(_interactionHand->position());
		std::cout << "Density at mouse: " << densityAtMouse << std::endl;
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
}


// ----------------------------------------------- SMOOTHING KERNELS --------------------------------------------- //

float SmoothingKernels2D::smooth(glm::vec2 r, float smoothingRadius) {
	float r2 = r.x * r.x + r.y * r.y;
	float rmag = sqrt(r2);
	if (rmag > smoothingRadius)
		return 0;

	return 4.f / (pi * pow(smoothingRadius, 8)) * pow(smoothingRadius*smoothingRadius - r2, 3);
}

float SmoothingKernels2D::smoothDerivative(glm::vec2 r, float smoothingRadius) {
	float r2 = r.x * r.x + r.y * r.y;
	float rmag = sqrt(r2);
	if (rmag > smoothingRadius)
		return 0;

	return -24.f / (pi * pow(smoothingRadius, 8)) * rmag * pow(smoothingRadius * smoothingRadius - r2, 2);
}

float SmoothingKernels2D::spikey(glm::vec2 r, float smoothingRadius) {
	float rmag = glm::length(r);
	if (rmag > smoothingRadius)
		return 0;

	return 10.f / (pi * pow(smoothingRadius, 6)) * pow(smoothingRadius - rmag, 3);
}

float SmoothingKernels2D::spikeyDerivative(glm::vec2 r, float smoothingRadius) {
	float rmag = glm::length(r);
	if (rmag > smoothingRadius)
		return 0;

	return -30.f / (pi * pow(smoothingRadius, 6)) * pow(smoothingRadius - rmag,2);
}


