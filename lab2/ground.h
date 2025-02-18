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
	GLuint isPanelID;


	GLuint diffuseTextureID;
	GLuint specularTextureID;
	GLuint reflectionTextureID;

	bool isPanel = false;


	bool ground = false;

	void initialize(glm::vec3 position, glm::vec3 scale, const char* texturePath)
	{
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);


		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);



		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../lab2/shaders/ground.vert", "../lab2/shaders/ground.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}


	     initLightUniforms();


		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		modelMatrixID = glGetUniformLocation(programID, "model");
		isPanelID = glGetUniformLocation(programID, "isSolarPanel");
		isPanel = std::string(texturePath).find("panel") != std::string::npos;

		// Load textures depending on the type of ground
		if (isPanel) {
			std::string diffusePath = std::string(texturePath) + "/SolarPanel002_1K-JPG_Color.jpg";
			std::string specularPath = std::string(texturePath) + "/SolarPanel002_1K-JPG_Roughness.jpg";
			std::string reflectionPath = std::string(texturePath) + "/SolarPanel002_1K-JPG_Metalness.jpg";

			diffuseTextureID = LoadTextureTileBox(diffusePath.c_str());
			specularTextureID = LoadTextureTileBox(specularPath.c_str());
			reflectionTextureID = LoadTextureTileBox(reflectionPath.c_str());

		} else {
			std::string diffusePath = std::string(texturePath) + "/floor_bricks_02_diff_1k.jpg";
			std::string specularPath = std::string(texturePath) + "/floor_bricks_02_rough_1k.png";

			diffuseTextureID = LoadTextureTileBox(diffusePath.c_str());
			specularTextureID = LoadTextureTileBox(specularPath.c_str());

		}

	}



	void render(glm::mat4 cameraMatrix, GLuint shadowMapTexture, glm::mat4 lightSpaceMatrix) {
		glUseProgram(programID);

		glBindVertexArray(vertexArrayID);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);



		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		glm::mat4 mvp = cameraMatrix * getModelMatrix();

		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
		glUniform1i(glGetUniformLocation(programID, "diffuseTexture"), 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, specularTextureID);
		glUniform1i(glGetUniformLocation(programID, "specularTexture"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, reflectionTextureID);
		glUniform1i(glGetUniformLocation(programID, "reflectionMap"), 3);

		float time = glfwGetTime();
		glUniform1f(glGetUniformLocation(programID, "time"), time);
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);


		glUniform1i(isPanelID, isPanel);


		// Draw the ground
		glDrawElements(
			GL_TRIANGLES,
			6,
			GL_UNSIGNED_INT,
			(void*)0
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
	}




	void cleanup() override{
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteBuffers(1, &uvBufferID);
		glDeleteTextures(1, &textureID);
		glDeleteProgram(programID);

	}
	glm::vec3 getPosition() const {
		return position;
	}
};

#endif