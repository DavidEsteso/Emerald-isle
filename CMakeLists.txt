cmake_minimum_required(VERSION 3.5)
project(lab2)

find_package(OpenGL REQUIRED)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Add the external directory
add_subdirectory(external)

# Include the necessary directories
include_directories(
		external/glfw-3.1.2/include/
		external/glm-0.9.7.1/
		external/glad-opengl-3.3/include/
		external/
		lab2/
)

# Create the first executable, lab2_building
add_executable(lab2_building
		lab2/lab2_building.cpp
		lab2/render/shader.cpp
		lab2/sky.h
		lab2/building.h
		lab2/city.h
		lab2/ground.h
)

# Link the necessary libraries to the first executable
target_link_libraries(lab2_building
		${OPENGL_LIBRARY}
		glfw
		glad
)

# Create the second executable, lab2_skybox
add_executable(lab2_skybox
		lab2/lab2_skybox.cpp
		lab2/render/shader.cpp
)

# Link the necessary libraries to the second executable
target_link_libraries(lab2_skybox
		${OPENGL_LIBRARY}
		glfw
		glad
)