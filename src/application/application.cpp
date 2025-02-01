#include "application/application.h"



static std::vector<PoolSizeRatio> renderDescriptorSetSizes = {
	{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
	{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
	//{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
};

static std::vector<PoolSizeRatio> computeDescriptorSetSizes = {
	{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
};

Application::Application() : 
	window({ APPLICATION_WIDTH, APPLICATION_HEIGHT }, "VulkanEngineV2"),
	renderer(window) {

	// Get static logger
	logger = Logger::get_logger();
}

struct GlobalUBO {
	glm::mat4 projection;
	glm::mat4 view;
	float aspectRatio;
};

void Application::run() {
	// Initialize the descriptor pool
	DescriptorPool globalDescriptorPool(renderer.device(), 10, renderDescriptorSetSizes);

	glm::vec3 particleColor{ 1.0f, 1.0f, 1.0f };
	float particleRadius = 0.5f;

	int numParticles = 1;

	// The particle info struct contains the Particle struct (pos and vel), as well as color and radius of each particle
	GlobalParticleInfo particleInfo{
	.defaultColor = particleColor,
	.radius = particleRadius
	};

	// The constructor of the particle system initializes the positions of the particles to a grid
	ParticleSystem2D fluidParticles(numParticles, particleRadius);

	// We will use a uniform buffer for the global particle info 
	Buffer globalParticleBuffer(renderer.device(), renderer.allocator(), sizeof(GlobalParticleInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, renderer.device().physicalDeviceProperies().limits.minUniformBufferOffsetAlignment);
	globalParticleBuffer.map();

	// For the actual particle info, we want to use a storage buffer
	Buffer particleBuffer(renderer.device(), renderer.allocator(), sizeof(Particle2D) * MAX_PARTICLES, 1, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, renderer.device().physicalDeviceProperies().limits.minStorageBufferOffsetAlignment);
	particleBuffer.map();

	Buffer globalBuffer(renderer.device(), renderer.allocator(), sizeof(GlobalUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, renderer.device().physicalDeviceProperies().limits.minUniformBufferOffsetAlignment);
	globalBuffer.map();

	int bufferSize = sizeof(Particle2D) * MAX_PARTICLES;

	VkDescriptorSetLayout particleLayouts = renderer.descriptorLayoutBuilder().addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
	VkDescriptorSet particleDescriptor = globalDescriptorPool.allocateDescriptorSet(particleLayouts);
	// TODO: Refactor DescriptorWriter class to work properly. I don't want to clear the descriptor writer each time I want to write a buffer. It should just be that I can chain together the writes and then update.
	renderer.descriptorWriter().writeBuffer(0, globalParticleBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).updateDescriptorSet(particleDescriptor);
	renderer.descriptorWriter().clear();
	renderer.descriptorWriter().writeBuffer(1, particleBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).updateDescriptorSet(particleDescriptor);
	renderer.descriptorWriter().clear();
	renderer.descriptorLayoutBuilder().clear();

	VkDescriptorSetLayout globalLayout = renderer.descriptorLayoutBuilder().addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
	VkDescriptorSet globalDescriptor = globalDescriptorPool.allocateDescriptorSet(globalLayout);
	renderer.descriptorWriter().writeBuffer(0, globalBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).updateDescriptorSet(globalDescriptor);

	// Create the render systems and add them to the renderer
	ParticleRenderSystem particleRenderSystem(renderer, std::vector<VkDescriptorSetLayout>{particleLayouts,globalLayout}, std::vector<VkDescriptorSet>{particleDescriptor, globalDescriptor}, numParticles);
	renderer.addRenderSystem(&particleRenderSystem);

	// Set up the camera
	Camera camera{};

	GlobalUBO globalBufferObject{};
	
	// Main application loop
	while (!window.shouldClose()) {
		window.process_inputs(); // Poll the user inputs
		if (window.pauseRendering()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		// Set the camera projection with the current aspect ratio
		float aspect = renderer.aspectRatio();
		camera.setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.1f, 10.0f);
		camera.setViewDirection(glm::vec3{ 0.0f, 0.0f, 2.0f }, glm::vec3{ 0.0f, 0.0f, -1.0f });

		// Update camera info in the global buffer
		globalBufferObject.aspectRatio = renderer.aspectRatio();
		globalBufferObject.projection = camera.projectionMatrix();
		globalBufferObject.view = camera.viewMatrix();

		//particleSystem.update(); // Update the particle systems

		// Update/fill buffers
		globalBuffer.writeBuffer(&globalBufferObject);
		globalParticleBuffer.writeBuffer(&particleInfo);
		particleBuffer.writeBuffer(fluidParticles.particles());

		renderer.renderAll(); // Have the renderer render all the render systems

		renderer.resizeCallback(); // Check for window resize and call the window resize callback function
	}

	// TODO: Make sure the engine waits for all fences before quitting. Currently it is exiting in the middle of a render and throwing a validation warning
	logger->print("Shutting Down... Bye Bye!");
}
