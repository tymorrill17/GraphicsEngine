cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# target_link_libraries() allows use with targets in other directories
# (to use compile_project_shaders in any CMakeLists file)
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# STEP 1) ENTER THE NAME OF THE PROJECT
project("ProjectExecutable")

# set executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# output compile_commands.json to properly set up clangd for use in neovim
add_compile_definitions(BASE_DIR="${CMAKE_SOURCE_DIR}")

# Sets C++ Standard to 23
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 23)
  #set_property(TARGET VulkanEngineV2 PROPERTY CXX_STANDARD 23)
endif()

find_package(Vulkan REQUIRED) # Obtains all Vulkan API compiling info

# Function to help projects compile their own shaders
function(compile_project_shaders PROJECT_NAME PROJECT_DIR)
    file(GLOB_RECURSE PROJECT_GLSL_FILES
        "${PROJECT_DIR}/shaders/*.frag"
        "${PROJECT_DIR}/shaders/*.vert"
        "${PROJECT_DIR}/shaders/*.comp"
    )

    set(PROJECT_SPIRV_FILES "")
    foreach(GLSL ${PROJECT_GLSL_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${PROJECT_DIR}/shaders/${FILE_NAME}.spv")

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            COMMENT "Compiling ${PROJECT_NAME} shader ${FILE_NAME}"
        )
        list(APPEND PROJECT_SPIRV_FILES ${SPIRV})
    endforeach()

    if(PROJECT_SPIRV_FILES)
        add_custom_target(${PROJECT_NAME}Shaders DEPENDS ${PROJECT_SPIRV_FILES})
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}Shaders)
    endif()
endfunction()


# STEP 2) Add any subtree directories

# Build the render engine as a library first
add_subdirectory(graphics_engine) # Contains all third-party libraries


# STEP 3) Create and define executable

# Create the executable
set(ExecutableName "2d-fluid-sim")
add_executable(ProjectExecutable)

# Add project source files
file(GLOB_RECURSE PROJECT_A_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

target_sources(ProjectExecutable PRIVATE ${PROJECT_A_SOURCES})

# Link against the engine
target_link_libraries(ProjectExecutable PRIVATE VulkanEngine)

# Project-specific include directories
target_include_directories(ProjectExecutable PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compile project-specific shaders
compile_project_shaders(ProjectExecutable ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Set different output name
set_target_properties(ProjectExecutable PROPERTIES OUTPUT_NAME ${ExecutableName})

