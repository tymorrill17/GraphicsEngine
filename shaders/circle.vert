#version 450
#extension GL_KHR_vulkan_glsl : enable

const int MAX_PARTICLES = 100000;

const int NUM_OFFSETS = 6;
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

struct Particle2D {
	vec2 position;
	vec2 velocity;
	vec3 color;
};

layout (set = 1, binding = 0) uniform GlobalUBO {
	mat4 projection;
	mat4 view;
	float aspectRatio;
} globalBuffer;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragOffset;

layout (set = 0, binding = 0) uniform GlobalParticleInfo {
	vec3 defaultColor;
	float radius;
} globalParticleInfo;

layout (set = 0, binding = 1) readonly buffer ParticleData {
	Particle2D particles[MAX_PARTICLES];
} particleData;


void main() 
{
	int particleIndex = gl_VertexIndex / NUM_OFFSETS;
	fragOffset = OFFSETS[gl_VertexIndex % NUM_OFFSETS];
	vec3 cameraRightWorld = {globalBuffer.view[0][0], globalBuffer.view[1][0], globalBuffer.view[2][0]};
	vec3 cameraUpWorld = {globalBuffer.view[0][1], globalBuffer.view[1][1], globalBuffer.view[2][1]};
	fragColor = vec4(particleData.particles[particleIndex].color, 1.0f);
	vec3 worldPosition = vec3(particleData.particles[particleIndex].position, 0.0);
	worldPosition = worldPosition.xyz 
		+ globalParticleInfo.radius * fragOffset.x * cameraRightWorld 
		+ globalParticleInfo.radius * fragOffset.y * cameraUpWorld;
	gl_Position = globalBuffer.projection * globalBuffer.view * vec4(worldPosition, 1.0);
}
