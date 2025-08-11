# CMakeList.txt : CMake project for VulkanEngineV2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 4.1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# target_link_libraries() allows use with targets in other directories
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project ("VulkanProjects")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_compile_definitions(BASE_DIR="${CMAKE_SOURCE_DIR}") # Allows us to reference the project directory in-code

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

# Build the render engine as a library first
add_subdirectory(graphics_engine) # Contains all third-party libraries

# Build all the projects (each project will link against the engine)
add_subdirectory(fluid_sim) # Contains all source code
