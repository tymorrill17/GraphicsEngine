#pragma once
#include "glm/glm.hpp"
#include "NonCopyable.h"
#include "utility/timer.h"
#include "utility/input_manager.h"
#include "physics/hand.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <iostream>
#include <future>

#define MAX_PARTICLES 50000

struct BoundingBox {
	float left;
	float right;
	float bottom;
	float top;
};

struct GlobalParticleInfo {
	float defaultColor[4];
	float radius;
	float spacing;
	int numParticles;
};

struct Particle2D {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
};

struct RenderedParticle2D : Particle2D {
	glm::vec4 color{ 1.0f };
};

struct GlobalPhysicsInfo {
	float gravity = 9.8f;
	float boundaryDampingFactor;
	float collisionDampingFactor;
	float densitySmoothingRadius;
	float pressureConstant;
	float restDensity;
	int nSubsteps;
};

class ParticleSystem2D : public NonCopyable {
public:
	ParticleSystem2D(
		GlobalParticleInfo& particleInfo,
		GlobalPhysicsInfo& physicsInfo,
		BoundingBox& box,
		InputManager& inputManager,
		Hand* hand = nullptr
	);
	~ParticleSystem2D();

	// @brief initialize the particles in a grid
	void arrangeParticles();
	// @brief Runs every frame and updates the positions of the particles
	void update();

	void setBoundingBox(BoundingBox box) { _bbox = box; }
	void setParticleInfo(GlobalParticleInfo particleInfo) { _globalParticleInfo = particleInfo; }
	void setPhysicsInfo(GlobalPhysicsInfo physicsInfo) { _globalPhysics = physicsInfo; }
	void setHand(Hand* interactionHand) { _interactionHand = interactionHand; }

	RenderedParticle2D* particles() { return _particles; }
	GlobalParticleInfo& particleInfo() { return _globalParticleInfo; }
	GlobalPhysicsInfo& physicsInfo() { return _globalPhysics; }

protected:
	RenderedParticle2D* _particles; // Array of 2D particles
	BoundingBox& _bbox;
	GlobalParticleInfo& _globalParticleInfo;
	GlobalPhysicsInfo& _globalPhysics;
	InputManager& _inputManager;
	Hand* _interactionHand;
	//float** _densities;
	float* _densities;
	glm::vec2* _acceleration;
	bool _simulationPaused;
	bool _doOneFrame;

	Particle2D* _particles2;
	// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
	//Particle2D* _particles3;
	//Particle2D* _particles4;

	// Compact Hashing
	uint32_t* _particleIndices;
	uint32_t* _spatialLookup;
	uint32_t* _startIndices;

	// Concurrency
	std::vector<std::future<void>> _futures;

	template<typename ParticleType>
	void updateSpatialLookup(ParticleType* particles);

	void sortSpatialArrays();

	template<typename ParticleType>
	void loopThroughNearbyPoints(glm::vec2 particlePosition, ParticleType* particles, std::function<void(glm::vec2, uint32_t)> callback);

	// @brief Resolves collisions between particles
	void resolveParticleCollisions();

	// @brief Resolves collisions with the bouding box
	void resolveBoundaryCollisions();

	// @brief Calculates the density at each particle
	template<typename ParticleType>
	void calculateParticleDensitiesParallel(std::vector<int> batchSizes, ParticleType* particles);

	// @brief Calculates the density at given position
	template<typename ParticleType>
	float calculateDensity(uint32_t particleIndex, ParticleType* particles);

	// @brief applies acceleration due to gravity to the velocities of the particles
	template<typename ParticleType>
	glm::vec2 getAcceleration(uint32_t particleIndex, ParticleType* particles);
	template<typename ParticleType>
	void getAccelerationParallel(std::vector<int> batchSizes, glm::vec2* outputAccel, ParticleType* particles);

	template<typename ParticleType>
	glm::vec2 calculatePressureForce(int particleIndex, ParticleType* particles, float* densities);

	void applyGravity(int particleIndex, float deltaTime);

	// Converts density to pressure using the ideal gas equation
	float getPressure(float density);

	float getSharedPressure(float density, float otherDensity);

	void assignInputEvents();

	void proceedFrame();
	void frameDone();
};

class SmoothingKernels2D {
public:
	// @brief Poly6 polynomial interpolant that is smooth and has near-zero derivatives near the center. Should be used for density calculations e.g.
	static float smooth(float squareDst, float smoothingRadius);
	static float smoothDerivative(float squareDst, float smoothingRadius);

	// @brief This smoothing kernel has increasing derivatives near the center, the center being a sharp point having no derivative.
	static float spikey(float squareDst, float smoothingRadius);
	static float spikeyDerivative(float squareDst, float smoothingRadius);
}; 