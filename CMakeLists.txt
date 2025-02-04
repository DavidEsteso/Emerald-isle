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
		external/tinyobjloader/
		external/tinygltf-2.9.3/
		external/
		lab2/
)

add_library(tinyobjloader STATIC
		external/tinyobjloader/tiny_obj_loader.cc
)

# Create the first executable, lab2_building
add_executable(lab2_building
        lab2/main.cpp
		lab2/render/shader.cpp
		lab2/sky.h
		lab2/building.h
		lab2/city.h
		lab2/ground.h
        lab2/tree.h
		lab2/aircraft.h
		lab2/entity.h
		lab2/bot.h
		external/tinygltf-2.9.3/tiny_gltf.h
		lab2/implementations.cpp
		external/tinygltf-2.9.3/stb_image.h
		lab2/tea.h
		lab2/spire.h
        lab2/obelisk.h
		lab2/sphere.h
)

# Link the necessary libraries to the first executable
target_link_libraries(lab2_building
		${OPENGL_LIBRARY}
		glfw
		glad
		tinyobjloader
)

