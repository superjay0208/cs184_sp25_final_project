cmake_minimum_required(VERSION 3.10)
project(ToonShadingExample CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Find Dependencies ---

# Find OpenGL (comes with most systems)
find_package(OpenGL REQUIRED)

# Find GLEW (OpenGL Extension Wrangler)
# You might need to provide hints if it's not in a standard location
# set(GLEW_DIR "/path/to/glew/install" CACHE PATH "GLEW install directory")
find_package(GLEW REQUIRED)

# Find GLFW (Windowing Library)
# You might need to provide hints if it's not in a standard location
# set(GLFW_DIR "/path/to/glfw/install" CACHE PATH "GLFW install directory")
find_package(glfw3 3.3 REQUIRED) # Use the config-file package if available

# Find GLM (OpenGL Mathematics)
# You might need to provide hints if it's not in a standard location
# set(GLM_DIR "/path/to/glm/install" CACHE PATH "GLM install directory")
# Or, if using find_package with GLM's CMake config:
find_package(glm 0.9.9 REQUIRED) # Adjust version as needed

# --- Executable ---

add_executable(toon_shader_app main.cpp shader.cpp)

# --- Include Directories ---

target_include_directories(toon_shader_app PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${glfw3_INCLUDE_DIRS}  # Use variable from find_package(glfw3 ...)
    ${glm_INCLUDE_DIRS}    # Use variable from find_package(glm ...)
)

# --- Linking ---

target_link_libraries(toon_shader_app PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw                # Use target name from find_package(glfw3 ...)
    glm::glm            # Use target name from find_package(glm ...)
)

# --- Copy Shaders (Optional, but helpful) ---
# Copies shader files to the build directory next to the executable
# Adjust the path "shaders/" if you place them elsewhere
set(SHADER_FILES
    shaders/toon.vert
    shaders/toon.frag
)

foreach(SHADER_FILE ${SHADER_FILES})
    add_custom_command(TARGET toon_shader_app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE}
        $<TARGET_FILE_DIR:toon_shader_app>/${SHADER_FILE}
        COMMENT "Copying ${SHADER_FILE}"
    )
endforeach()

# --- Platform Specifics (Example for macOS) ---
if(APPLE)
    target_link_libraries(toon_shader_app PRIVATE "-framework Cocoa -framework IOKit -framework CoreVideo")
endif()

