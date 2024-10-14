# CMakeList.txt : CMake project for VulkanEngineV2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("VulkanEngineV2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_compile_definitions(BASE_DIR="${CMAKE_SOURCE_DIR}") # Allows us to reference the project directory in-code

# Sets C++ Standard to 23
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 23)
  #set_property(TARGET VulkanEngineV2 PROPERTY CXX_STANDARD 23)
endif()

find_package(Vulkan REQUIRED) # Obtains all Vulkan API compiling info

add_subdirectory(src) # Contains all source code
add_subdirectory(dependencies) # Contains all third-party libraries
add_subdirectory(include) # Contains all header files


### Shader Compilation - TODO: Should/Can this be done during runtime instead of at configure time?

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

if(NOT GLSL_VALIDATOR)
    message("Could not find glslangValidator!")
endif()

# Find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

# Iterate over each shader and compile them with glslangValidator
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  # Execute glslang command to compile the current shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
# A target is an executable, library, etc. that cmake builds. Create a custom shader target for our shader modules.
add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )
