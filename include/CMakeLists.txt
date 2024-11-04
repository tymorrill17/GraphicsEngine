file(GLOB_RECURSE PROJECT_HEADERS *.h)

target_include_directories(run_application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
