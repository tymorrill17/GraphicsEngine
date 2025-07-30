#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct Particle2D {
	vec2 position;
	vec2 velocity;
	vec4 color;
};

layout (set = 0, binding = 0) uniform GlobalParticleInfo {
	vec4 defaultColor;
	float radius;
	int numParticles;
} globalParticleInfo;

layout (set = 0, binding = 1) readonly buffer ParticleData {
	Particle2D particles[];
} particleData;

void main() 
{
	float dist = dot(fragOffset,fragOffset);
	float radiusSquared = globalParticleInfo.radius * globalParticleInfo.radius;

	outColor = fragColor;
	if (dist > 1) discard;

}
