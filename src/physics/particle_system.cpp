#include "physics/particle_system.h"
#include <random>

static float gravity = 9.8f;
static float collisionDampingFactor = 0.9;
static glm::vec2 down{ 0.0f, -0.1f };

ParticleSystem2D::ParticleSystem2D(GlobalParticleInfo globalInfo, BoundingBox box) :
	_globalInfo(globalInfo),
	_bbox(box) {

	_particles = new Particle2D[MAX_PARTICLES];
	arrangeParticles();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
}

void ParticleSystem2D::arrangeParticles() {
	// TODO: Add these to an ImGui window later to play with on the fly
	float spacing = _globalInfo.radius + _globalInfo.spacing;
	glm::vec2 offset = glm::vec2();

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_globalInfo.numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _globalInfo.numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;

		// Initialize the color of the particle to the default
		_particles[i].color = _globalInfo.defaultColor;

		// Set a random starting velocity
		//_particles[i].velocity = 0.1f * glm::normalize(glm::vec2{ distribution(generator), distribution(generator) });
		_particles[i].velocity = glm::vec2{ distribution(generator), distribution(generator) };
	}
}

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();

	applyGravity();

	// For each particle, apply velocity to position
	for (int i = 0; i < _globalInfo.numParticles; i++) {
		_particles[i].position += _particles[i].velocity * timer.frameTime();
	}

	// Resolve collisions with the walls of the bounding box for now
	resolveCollisions();
}

void ParticleSystem2D::resolveCollisions() {
	for (int i = 0; i < _globalInfo.numParticles; i++) {
		if (_particles[i].position.y < (_bbox.bottom + _globalInfo.radius)) {
			_particles[i].position.y = _bbox.bottom + _globalInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * collisionDampingFactor;
		}
		else if (_particles[i].position.y > (_bbox.top - _globalInfo.radius)) {
			_particles[i].position.y = _bbox.top - _globalInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * collisionDampingFactor;
		}
		if (_particles[i].position.x > (_bbox.right - _globalInfo.radius)) {
			_particles[i].position.x = _bbox.right - _globalInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * collisionDampingFactor;
		}
		else if (_particles[i].position.x < (_bbox.left + _globalInfo.radius)) {
			_particles[i].position.x = _bbox.left + _globalInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * collisionDampingFactor;
		}
	}
}

void ParticleSystem2D::applyGravity() {
	static Timer& timer = Timer::getTimer();
	for (int i = 0; i < _globalInfo.numParticles; i++) {
		_particles[i].velocity += (gravity * timer.frameTime()) * down;
	}
}
