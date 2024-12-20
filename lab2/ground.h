#ifndef GROUND_H
#define GROUND_H


#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <render/shader.h>


#include <vector>
#include <iostream>
#include <math.h>
#include <unordered_map>
#include <cmath>
#include <fastNoiseLite.h>
#include <random>

#include "entity.h"

static GLuint LoadTextureTileBox(const char *texture_file_path) {
	int w, h, channels;
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);

	return texture;
}

struct Ground : public Entity {


	GLfloat vertex_buffer_data[12] = {	// Vertex definition for a canonical box
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
};

	GLfloat color_buffer_data[12] = {
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

	};

	GLuint index_buffer_data[6] = {
		0, 1, 2,
		0, 2, 3,

	};

    // TODO: Define UV buffer data--DONE
    // ---------------------------
	GLfloat uv_buffer_data[8] = {
		// Front
		0.0f, 1.0f, // Top-left
		1.0f, 1.0f, // Top-right
		1.0f, 0.0f, // Bottom-right
		0.0f, 0.0f, // Bottom-left

	};

	GLfloat normal_buffer_data[12] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		};

    // ---------------------------

	// OpenGL buffers
	GLuint vertexArrayID;

	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;
	GLuint normalBufferID;
	GLuint modelMatrixID;


	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint lightColorID;

	bool ground = false;

	void initialize(glm::vec3 position, glm::vec3 scale, const char* texturePath) {

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


		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);

		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

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

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../lab2/shaders/ground.vert", "../lab2/shaders/ground.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
		lightColorID = glGetUniformLocation(programID, "lightColor");

		modelMatrixID = glGetUniformLocation(programID, "model");


        // TODO: Load a texture--DONE
        // --------------------
		textureID = LoadTextureTileBox(texturePath);
        // --------------------

		// si el texture paz acaba en facade3.jpg le paso un 1 al shader sino un 0
		if (std::string(texturePath).find("AIRGROUND.png") != std::string::npos)
		{
			ground = true;
		}
		std::cout << "ground: " << ground << std::endl;


        // TODO: Get a handle to texture sampler--DONE
        // -------------------------------------
		textureSamplerID  = glGetUniformLocation(programID,"textureSampler");
        // -------------------------------------


	}

	void renderForShadows(const glm::mat4& lightSpaceMatrix, GLuint shadowProgramID) {
		glUseProgram(shadowProgramID);

		GLuint modelLoc = glGetUniformLocation(shadowProgramID, "model");

		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::scale(modelMatrix, scale);


		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMatrix[0][0]);

		GLuint lightSpaceMatrixID = glGetUniformLocation(shadowProgramID, "lightSpaceMatrix");

		glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		glDrawElements(
			GL_TRIANGLES,
			6,
			GL_UNSIGNED_INT,
			(void*)0
		);

		glDisableVertexAttribArray(0);
	}


	void render(glm::mat4 cameraMatrix, GLuint shadowMapTexture, glm::mat4 lightSpaceMatrix) {
		glUseProgram(programID);

		glBindVertexArray(vertexArrayID);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		GLuint shadowMapID = glGetUniformLocation(programID, "shadowMap");
		glUniform1i(shadowMapID, 1);

		// TODO: Model transform
		// -----------------------
        glm::mat4 modelMatrix = glm::mat4();
        // Scale the box along each axis to make it look like a building
		modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, scale);

        // -----------------------

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		// TODO: Enable UV buffer and texture sampler--DONE
		// ------------------------------------------
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);

		float time = glfwGetTime();
		glUniform1f(glGetUniformLocation(programID, "time"), time);
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);


		glUniform3fv(lightPositionID, 1, &LightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniform3fv(lightColorID, 1, &LightColor[0]);

		//pass lightSpaceMatrix to shader
		GLuint lightSpaceMatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		GLuint groundID = glGetUniformLocation(programID, "ground");
		glUniform1i(groundID, 0);


		// si ground es true paso un 1 al shader sino un 0
		if (ground)
		{
			GLuint groundID = glGetUniformLocation(programID, "ground");
			glUniform1i(groundID, 1);
		}





        // ------------------------------------------

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			6,    			   // number of indices
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
	}
	glm::vec3 getPosition() const {
		return position;
	}
};

#endif