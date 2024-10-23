file(GLOB_RECURSE PROJECT_SOURCES
	main.cpp
	application/application.cpp
	logger/logger.cpp
	logger/debug_messenger.cpp
	renderer/command.cpp
	renderer/image.cpp
	renderer/device.cpp
	renderer/engine.cpp
	renderer/frame.cpp
	renderer/instance.cpp
	renderer/pipeline.cpp
	renderer/pipeline_builder.cpp
	renderer/queue_family.cpp
	renderer/shader.cpp
	renderer/swapchain.cpp
	renderer/sync.cpp
	utility/allocator.cpp
	utility/window.cpp)

add_executable(run_application
	${PROJECT_SOURCES}
	${PROJECT_HEADERS})

set_property(TARGET run_application PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:run_application>")

## TODO: Perhaps delete later if shader compilation is moved to runtime?
add_dependencies(run_application Shaders) # Ensures shaders are built before application is

# Link all of the 3rd party libraries
target_link_libraries(run_application vma glm imgui stb_image)
# Link sdl and Vulkan libraries
target_link_libraries(run_application Vulkan::Vulkan sdl2)
