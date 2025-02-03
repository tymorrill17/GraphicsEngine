#include "physics/particle_system.h"
#include <random>

static float gravity = 9.8f;
static float collisionDampingFactor = 0.6;
static glm::vec2 down{ 0.0f, -0.1f };

ParticleSystem2D::ParticleSystem2D(int numParticles, float radius, BoundingBox box) :
	_numParticles(numParticles),
	_particleRadius(radius) {

	_particles = new Particle2D[MAX_PARTICLES];
	arrangeParticles();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete _particles;
}

void ParticleSystem2D::arrangeParticles() {
	// TODO: Add these to an ImGui window later to play with on the fly
	float spacing = _particleRadius;
	glm::vec2 offset = glm::vec2(0.0f);

	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;
		//_particles[i].velocity = 0.1f * glm::normalize(glm::vec2{ distribution(generator), distribution(generator) });
	}

	
}

void ParticleSystem2D::update() {
	
	static Timer& timer = Timer::getTimer();

	applyGravity();

	// For each particle, apply velocity to position
	for (int i = 0; i < _numParticles; i++) {
		_particles[i].position += _particles[i].velocity * timer.frameTime();
	}

	// Resolve collisions with the walls of the bounding box for now
	resolveCollisions();
}

void ParticleSystem2D::resolveCollisions() {
	for (int i = 0; i < _numParticles; i++) {
		if (_particles[i].position.y < (_bbox.bottom + _particleRadius)) {
			_particles[i].position.y = _bbox.bottom + _particleRadius;
			_particles[i].velocity.y = -_particles[i].velocity.y * collisionDampingFactor;
		}
		else if (_particles[i].position.y > (_bbox.top - _particleRadius)) {
			_particles[i].position.y = _bbox.top - _particleRadius;
			_particles[i].velocity.y = -_particles[i].velocity.y * collisionDampingFactor;
		}
		if (_particles[i].position.x > (_bbox.right - _particleRadius)) {
			_particles[i].position.x = _bbox.right - _particleRadius;
			_particles[i].velocity.x = -_particles[i].velocity.x * collisionDampingFactor;
		}
		else if (_particles[i].position.x < (_bbox.left + _particleRadius)) {
			_particles[i].position.x = _bbox.left + _particleRadius;
			_particles[i].velocity.x = -_particles[i].velocity.x * collisionDampingFactor;
		}
	}
}

void ParticleSystem2D::applyGravity() {
	static Timer& timer = Timer::getTimer();
	for (int i = 0; i < _numParticles; i++) {
		_particles[i].velocity += (gravity * timer.frameTime()) * down;
	}
}
