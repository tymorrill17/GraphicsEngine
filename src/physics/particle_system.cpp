#include "physics/particle_system.h"
#include <random>

static glm::vec2 down{ 0.0f, -0.1f };

static long double norm(glm::vec2 v) {
	return glm::sqrt(v.x * v.x + v.y * v.y);
}

ParticleSystem2D::ParticleSystem2D(GlobalParticleInfo particleInfo, GlobalPhysicsInfo physicsInfo, BoundingBox box) :
	_globalParticleInfo(particleInfo),
	_globalPhysics(physicsInfo),
	_bbox(box) {

	_particles = new Particle2D[MAX_PARTICLES];
	arrangeParticles();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
}

void ParticleSystem2D::arrangeParticles() {
	// TODO: Add these to an ImGui window later to play with on the fly
	float spacing = _globalParticleInfo.radius + _globalParticleInfo.spacing;
	glm::vec2 offset = glm::vec2();

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_globalParticleInfo.numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;

		// Initialize the color of the particle to the default
		_particles[i].color = _globalParticleInfo.defaultColor;

		// Set a random starting velocity
		//_particles[i].velocity = 0.1f * glm::normalize(glm::vec2{ distribution(generator), distribution(generator) });
		_particles[i].velocity = glm::vec2{ distribution(generator), distribution(generator) };
	}
}

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		applyGravity(subDeltaTime);

		// For each particle, apply velocity to position
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].position += _particles[i].velocity * subDeltaTime;
		}

		// Resolve collisions between particles
		resolveParticleCollisions();

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

void ParticleSystem2D::applyGravity(float deltaTime) {
	static Timer& timer = Timer::getTimer();
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		_particles[i].velocity += (_globalPhysics.gravity * deltaTime) * down;
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

