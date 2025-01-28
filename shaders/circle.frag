#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

const int MAX_PARTICLES = 100000;
const int NUM_PARTICLES = 1;

struct Particle2D {
	vec2 position;
	vec2 velocity;
	vec3 color;
};

layout (set = 0, binding = 0) uniform GlobalParticleInfo {
	vec3 defaultColor;
	float radius;
} globalParticleInfo;

layout (set = 0, binding = 1) readonly buffer ParticleData {
	Particle2D particles[MAX_PARTICLES];
} particleData;

void main() 
{
	float dist = dot(fragOffset,fragOffset);
	float radiusSquared = globalParticleInfo.radius * globalParticleInfo.radius;

	if (dist > 1) discard;

	outColor = fragColor;
}
