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

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		// Calculate the density of the fluid at each particle and populate _densities[]
		calculateParticleDensities();

		// For each particle, first apply acceleration to the velocity
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			// getAcceleration applies gravity, interaction force, and pressure force at once
			_particles[i].velocity += getAcceleration(i) * subDeltaTime;
		}

		// Apply pressure force
		/*for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].velocity += calculatePressureForce(i) * subDeltaTime / _densities[i];
		}*/

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
	// Use the locations of each particle to calculate the density at position, with the smoothing function lessening the impact of particles further away
	for (int j = 0; j < _globalParticleInfo.numParticles; j++) {
		// The smoothing function returns 0 if the distance from position to the particle is greater than the smoothing radius
		density += SmoothingKernels2D::smooth(position - _particles[j].position, _globalPhysics.densitySmoothingRadius);
	}
	return density;
}

void ParticleSystem2D::calculateParticleDensities() {
	// We want to calculate the density at each particle location all at once.
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		_densities[i] = calculateDensity(_particles[i].position);
	}
}

glm::vec2 ParticleSystem2D::getAcceleration(int particleIndex) {
	static Timer& timer = Timer::getTimer();

	// initialize each acceleration type
	glm::vec2 gravityAcceleration = _globalPhysics.gravity * down;
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
	glm::vec2 force{ 0.0f, 0.0f };
	// We are finding a field quantity like density, so we use the SPH equation. This involves looping over each particle that contributes to the quantity
	for (int j = 0; j < _globalParticleInfo.numParticles; j++) {
		if (index == j) continue; // The particle itself doesn't contribute to the pressure force it feels

		// Get the distance vector from the j-th particle to the current one. The force should be pointing in the OPPOSITE direction
		glm::vec2 jtoiDistance = _particles[index].position - _particles[j].position;
		float distance = glm::sqrt(glm::dot(jtoiDistance,jtoiDistance));
		//if (distance > _globalPhysics.densitySmoothingRadius) continue;

		// If particles are on top of each other, pick a random normal direction
		glm::vec2 jtoiDirection = (distance == 0.0f) ? getRandomDirection() : jtoiDistance / distance;

		// The pressure force needs to follow newton's third law, so instead of using the particles full pressure, take the average between particle index and particle j
		// Then multiply with the opposite direction to 
		force += -getSharedPressure(_densities[index], _densities[j]) * jtoiDirection * SmoothingKernels2D::spikeyDerivative(jtoiDistance, _globalPhysics.densitySmoothingRadius) / _densities[j];
	}
	return force;
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
	float r2 = glm::dot(r, r);
	float rmag = sqrt(r2);
	if (rmag > smoothingRadius)
		return 0;

	return 4.f / (pi * pow(smoothingRadius, 8)) * pow(smoothingRadius*smoothingRadius - r2, 3);
}

float SmoothingKernels2D::smoothDerivative(glm::vec2 r, float smoothingRadius) {
	float r2 = glm::dot(r, r);
	float rmag = sqrt(r2);
	if (rmag > smoothingRadius)
		return 0;

	return -24.f / (pi * pow(smoothingRadius, 8)) * rmag * pow(smoothingRadius * smoothingRadius - r2, 2);
}

float SmoothingKernels2D::spikey(glm::vec2 r, float smoothingRadius) {
	float rmag = glm::sqrt(glm::dot(r, r));
	if (rmag > smoothingRadius)
		return 0;

	return 10.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 3);
}

float SmoothingKernels2D::spikeyDerivative(glm::vec2 r, float smoothingRadius) {
	float rmag = glm::sqrt(glm::dot(r, r));
	if (rmag > smoothingRadius)
		return 0;
	
	return -30.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 2);
}


