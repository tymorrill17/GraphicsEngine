#pragma once
#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "utility/timer.h"
#include <vector>

#define MAX_PARTICLES 100000

struct BoundingBox {
	float left;
	float right;
	float bottom;
	float top;
};

struct GlobalParticleInfo {
	glm::vec3 defaultColor;
	float radius;
};

struct Particle2D {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
	glm::vec4 color{ 1.0f };
};

class ParticleSystem2D : public NonCopyable {
public:
	ParticleSystem2D(const int numParticles, const float radius, BoundingBox box);
	~ParticleSystem2D();

	// @brief initialize the particles in a grid
	void arrangeParticles();
	// @brief Runs every frame and updates the positions of the particles
	void update();

	void setBoundingBox(BoundingBox box) { _bbox = box; }

	Particle2D* particles() {
		return _particles;
	}

protected:
	int _numParticles;
	float _particleRadius;
	Particle2D* _particles; // Array of 2D particles
	BoundingBox _bbox;

	// @brief Resolves collisions with the bouding box and between particles
	void resolveCollisions();

	// @brief applies acceleration due to gravity to the velocities of the particles
	void applyGravity();
};