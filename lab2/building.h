#ifndef BUILDING_H
#define BUILDING_H


#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <render/shader.h>

#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#include <math.h>
#include <unordered_map>
#include <cmath>
#include <fastNoiseLite.h>
#include <random>

#include "entity.h"


GLuint LoadCubeMap(const char* cubemapDir) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	const char* faces[6] = {
		"right.png", "left.png",
		"up.png", "down.png",
		"front.png", "back.png"
	};

	for (unsigned int i = 0; i < 6; i++) {
		std::string filename = std::string(cubemapDir) + "_" + faces[i];
		// Load image, create OpenGL texture
		int width, height, channels;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
		if (data) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		} else {
			std::cerr << "Cubemap texture failed to load: " << filename << std::endl;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

struct Building : public Entity{
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

    // TODO: Define UV buffer data--DONE
    // ---------------------------
	GLfloat uv_buffer_data[48] = {
		// Front
		0.0f, 1.0f, // Top-left
		1.0f, 1.0f, // Top-right
		1.0f, 0.0f, // Bottom-right
		0.0f, 0.0f, // Bottom-left

		// Back
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Left
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Right
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Top - we do not want texture the top
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// Bottom - we do not want texture the bottom
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};

	GLfloat normal_buffer_data[72] = {
		0.0f, 0.0f, 1.0f,  // Front
		0.0f, 0.0f, -1.0f, // Back
		-1.0f, 0.0f, 0.0f, // Left
		1.0f, 0.0f, 0.0f,  // Right
		0.0f, 1.0f, 0.0f,  // Top
		0.0f, -1.0f, 0.0f  // Bottom
	};

    // ---------------------------

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;
	GLuint cubemapTextureID;
	GLuint cubemapSamplerID;
	GLuint modelMatrixID;
	GLuint viewMatrixID;
	GLuint projectionMatrixID;
	GLuint normalBufferID;


	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	void initialize(glm::vec3 position, glm::vec3 scale, const char* cubemapPath) {

		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
        // TODO:
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);


		for (int i = 0; i < 72; ++i) color_buffer_data[i] = 1.0f;

		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		/**

		float height = scale.y;
		float width = scale.x;
		float depth = scale.z;

		for (int i = 0; i < 24; ++i) {
			if (i < 8) {
				// Faces along the x-y plane (front and back)
				uv_buffer_data[2 * i] *= width / 16;
				uv_buffer_data[2 * i + 1] *= height / 16;
			} else if (i < 16) {
				// Faces along the y-z plane (left and right sides)
				uv_buffer_data[2 * i] *= depth / 16;
				uv_buffer_data[2 * i + 1] *= height / 16;
			} else {
				// Faces along the x-z plane (top and bottom)
				uv_buffer_data[2 * i] *= width / 16;
				uv_buffer_data[2 * i + 1] *= depth / 16;
			}
		}
		**/
		// --------------------

		// TODO: Create a vertex buffer object to store the UV data--DONE
		// --------------------------------------------------------
		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);
        // --------------------------------------------------------

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		cubemapTextureID = LoadCubeMap(cubemapPath);


		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../lab2/shaders/box.vert", "../lab2/shaders/box.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		//mvpMatrixID = glGetUniformLocation(programID, "MVP");

        // TODO: Load a texture--DONE
        // --------------------
		//textureID = LoadTextureTileBox(texturePath);
        // --------------------

        // TODO: Get a handle to texture sampler--DONE
        // -------------------------------------
		cubemapSamplerID = glGetUniformLocation(programID, "cubemap");
		viewMatrixID = glGetUniformLocation(programID, "view");
		projectionMatrixID = glGetUniformLocation(programID, "projection");
		modelMatrixID = glGetUniformLocation(programID, "model");
		GLuint cameraPosLocation = glGetUniformLocation(programID, "cameraPosition");


	}

    void render(glm::mat4 cameraMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, glm::vec3 eye_center) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// TODO: Model transform
		// -----------------------
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::scale(modelMatrix, scale);

        // -----------------------

		// Set model-view-projection matrix
		glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniform3fv(glGetUniformLocation(programID, "cameraPosition"), 1, &eye_center[0]);		// Bind cubemap texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
		glUniform1i(glGetUniformLocation(programID, "cubemap"), 0);

        // ------------------------------------------
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        //glDisableVertexAttribArray(2);
	}

	void cleanup() override{
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		//glDeleteBuffers(1, &uvBufferID);
		//glDeleteTextures(1, &textureID);
		glDeleteProgram(programID);
		glDeleteTextures(1, &cubemapTextureID);
	}
	glm::vec3 getPosition() const {
		return position;
	}
};

#endif