#include "physics/particle_system.h"
#include <random>

static glm::vec2 down{ 0.0f, -0.1f };

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
	arrangeParticles();
	assignInputEvents();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
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
	}
}

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		// For each particle, first apply acceleration to velocity
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			_particles[i].velocity += getAcceleration(i) * subDeltaTime;
		}
		// Then apply velocity to position
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

glm::vec2 ParticleSystem2D::getAcceleration(int particleIndex) {
	static Timer& timer = Timer::getTimer();

	glm::vec2 gravityAcceleration = _globalPhysics.gravity * down;
	glm::vec2 handAcceleration{ 0.f, 0.f };

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
			/*float gravityWeight = 1 - (centerFactor * glm::clamp(interactionStrength / _globalPhysics.gravity, 0.0f, 1.0f));
			glm::vec2 acceleration = gravityAcceleration * gravityWeight + particleToHand * centerFactor * interactionStrength;
			acceleration -= _particles[particleIndex].velocity * centerFactor;
			return acceleration;*/
		}
	}
	return gravityAcceleration + handAcceleration;
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

