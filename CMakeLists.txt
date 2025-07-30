# CMakeList.txt : CMake project for VulkanEngineV2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

project ("VulkanProjects")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_compile_definitions(BASE_DIR="${CMAKE_SOURCE_DIR}") # Allows us to reference the project directory in-code

# Sets C++ Standard to 23
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set(CMAKE_CXX_STANDARD 23)
  #set_property(TARGET VulkanEngineV2 PROPERTY CXX_STANDARD 23)
endif()

find_package(Vulkan REQUIRED) # Obtains all Vulkan API compiling info

# Build the render engine as a library first
add_subdirectory(render_engine) # Contains all third-party libraries

# Build all the projects (each project will link against the engine)
add_subdirectory(fluid_sim) # Contains all source code

