#include "application/application.h"

Application::Application() : 
	window({ APPLICATION_WIDTH, APPLICATION_HEIGHT }, "VulkanEngineV2"),
	renderer(window),
	inputManager(window) {}

struct GlobalUBO {
	glm::mat4 projection;
	glm::mat4 view;
	float aspectRatio;
};

void Application::run() {
	static Logger& logger = Logger::getLogger();
	static Timer& timer = Timer::getTimer();
	static Gui& gui = Gui::getGui();

	// Initialize the descriptor pool
	std::vector<PoolSizeRatio> renderDescriptorSetSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		//{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
		//{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
	};
	static std::vector<PoolSizeRatio> computeDescriptorSetSizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}
	};
	DescriptorPool globalDescriptorPool(renderer.device(), 10, renderDescriptorSetSizes);

	//glm::vec4 particleColor = glm::vec4{ glm::normalize(glm::vec3{ 25.0f, 118.0f, 210.0f }), 1.0f };
	float particleColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// The particle info struct contains the Particle struct (pos and vel), as well as color and radius of each particle
	GlobalParticleInfo particleInfo{
		.defaultColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		.radius = 0.03f,
		.spacing = 0.1f,
		.numParticles = 500
	};

	GlobalPhysicsInfo physicsInfo{
		.gravity = 0.f,
		.boundaryDampingFactor = 0.9f,
		.collisionDampingFactor = 0.9f,
		.densitySmoothingRadius = 1.f,
		.pressureConstant = 5.f,
		.restDensity = 0.f,
		.nSubsteps = 1,
	};

	BoundingBox box{};

	float handRadius = 0.4f;
	float interactionStrength = 4.f;
	Hand mouseInteraction(handRadius, interactionStrength, coordinateScale);

	// The constructor of the particle system initializes the positions of the particles to a grid
	ParticleSystem2D fluidParticles(particleInfo, physicsInfo, box, inputManager, &mouseInteraction);

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
	ParticleRenderSystem particleRenderSystem(renderer, std::vector<VkDescriptorSetLayout>{particleLayouts, globalLayout}, std::vector<VkDescriptorSet>{particleDescriptor, globalDescriptor}, fluidParticles);
	renderer.addRenderSystem(&particleRenderSystem);

	// Set up the camera
	Camera camera{};

	GlobalUBO globalBufferObject{};

	GuiRenderSystem guiRenderSystem(renderer, window);
	renderer.addRenderSystem(&guiRenderSystem);

	logger.print("Starting the main loop!");

	// Start physics when this becomes true;
	bool letThereBeLight = false;
	glm::vec2 mousePosition;

	// Main application loop
	while (!window.shouldClose()) {
		inputManager.processInputs(); // Poll the user inputs
		if (window.pauseRendering()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		timer.update();
		guiRenderSystem.getNewFrame();

		// Timer Info Display
		gui.addWidget("Info", [&]() {
			ImGui::Text("FrameTime: %.8f ms", timer.frameTime());
			ImGui::Text("FPS: %.2f", timer.framesPerSecond());
			ImGui::Text("Mouse Position: (%.2f, %.2f)", mousePosition.x, mousePosition.y);
		});

		gui.addWidget("Controls", [&]() {
			if (ImGui::Button("Start")) {
				letThereBeLight = true;
			}
			if (ImGui::Button("Reset")) {
				letThereBeLight = false;
			}
		});

		// Particle Info Display
		gui.addWidget("Particle Info", [&]() {
			ImGui::DragFloat("Radius", &particleInfo.radius, 0.001, 0.0f, 1000000.0f);
			ImGui::DragFloat("Spacing", &particleInfo.spacing, 0.001, 0.0f, 1000000.0f);
			ImGui::DragInt("# Particles", &particleInfo.numParticles, 1, 0, MAX_PARTICLES);
			ImGui::ColorEdit4("Default Color", particleInfo.defaultColor);
		});

		// Physics Info Display
		gui.addWidget("Physics Info", [&]() {
			ImGui::DragFloat("Gravity", &physicsInfo.gravity, 0.01, 0.0f, 1000000.0f);
			ImGui::DragFloat("Boundary Damping", &physicsInfo.boundaryDampingFactor, 0.001, 0.0f, 1.0f);
			ImGui::DragFloat("Collision Damping", &physicsInfo.collisionDampingFactor, 0.001, 0.0f, 1.0f);
			ImGui::DragFloat("Density Smoothing", &physicsInfo.densitySmoothingRadius, 0.001, 0.01f, 10.f);
			ImGui::DragFloat("Pressure Constant", &physicsInfo.pressureConstant, 0.01, 0.01f, 1000.f);
			ImGui::DragFloat("Rest Density", &physicsInfo.restDensity, 0.01, 0.01f, 10000.f);
			ImGui::DragInt("# Substeps", &physicsInfo.nSubsteps, 1, 1, 100);
		});

		gui.addWidget("Interaction", [&]() {
			ImGui::DragFloat("Radius", &handRadius, 0.001f, 0.001f, 1000000.0f);
			ImGui::DragFloat("Strength", &interactionStrength, 0.001f, 0.001f, 1000000.0f);
			ImGui::Text("Position: (%.2f, %.2f)", mouseInteraction.position().x, mouseInteraction.position().y);
		});

		// Set the camera projection with the current aspect ratio
		float aspect = renderer.aspectRatio();
		box.left = -aspect * coordinateScale; box.right = aspect * coordinateScale;
		box.bottom = -1.0f * coordinateScale; box.top = 1.0f * coordinateScale;
		//camera.setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.1f, 10.0f);
		camera.setOrthographicProjection(box.left, box.right, box.bottom, box.top, 0.1f, 10.0f);
		camera.setViewDirection(glm::vec3{ 0.0f, 0.0f, -2.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f });

		// Update camera info in the global buffer
		globalBufferObject.aspectRatio = renderer.aspectRatio();
		globalBufferObject.projection = camera.projectionMatrix();
		globalBufferObject.view = camera.viewMatrix();

		mousePosition = inputManager.mousePosition();
		mouseInteraction.setPosition(mousePosition);
		mouseInteraction.radius = handRadius;
		mouseInteraction.strengthFactor = interactionStrength;

		if (letThereBeLight) {
			fluidParticles.update(); // Update the particle systems
		} else {
			fluidParticles.arrangeParticles();
		}

		// Update/fill buffers
		globalBuffer.writeBuffer(&globalBufferObject);
		globalParticleBuffer.writeBuffer(&particleInfo);
		particleBuffer.writeBuffer(fluidParticles.particles());

		renderer.renderAll(); // Have the renderer render all the render systems

		renderer.resizeCallback(); // Check for window resize and call the window resize callback function

		guiRenderSystem.endFrame();
	}

	// TODO: Make sure the engine waits for all fences before quitting. Currently it is exiting in the middle of a render and throwing a validation warning
	for (uint32_t i = 0; i < renderer.swapchain().framesInFlight(); i++) {
		VkFence endFence = renderer.getFrame(i).renderFence().handle();
		vkWaitForFences(renderer.device().device(), 1, &endFence, true, 10000000);
	}

	logger.print("Shutting Down... Bye Bye!");
}
