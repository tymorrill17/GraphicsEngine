file(GLOB_RECURSE PROJECT_HEADERS
	NonCopyable.h
	application/application.h
	logger/logger.h
	logger/debug_messenger.h
	renderer/command.h
	renderer/device.h
	renderer/engine.h
	renderer/frame.h
	renderer/image.h
	renderer/instance.h
	renderer/pipeline.h
	renderer/pipeline_builder.h
	renderer/queue_family.h
	renderer/shader.h
	renderer/swapchain.h
	renderer/sync.h
	utility/allocator.h
	utility/window.h)

target_include_directories(run_application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
