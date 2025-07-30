# project-a/CMakeLists.txt
project("2DFluidSimulator")

# Create the executable
add_executable(2DFluidSimulator)

# Add project source files
file(GLOB_RECURSE PROJECT_A_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

target_sources(2DFluidSimulator PRIVATE ${PROJECT_A_SOURCES})

# Link against the engine
target_link_libraries(2DFluidSimulator PRIVATE VulkanEngine)

# Project-specific include directories
target_include_directories(2DFluidSimulator PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compile project-specific shaders
compile_project_shaders(2DFluidSimulator ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Set different output name
set_target_properties(2DFluidSimulator PROPERTIES OUTPUT_NAME "2d-fluid-sim")

