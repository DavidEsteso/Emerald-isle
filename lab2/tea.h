//
// Created by david on 20/12/2024.
//

#ifndef TEA_H
#define TEA_H



#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tinyobjloader/tiny_obj_loader.h>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

struct Tea : public Entity {


    std::vector<float> vertex_buffer_data;
    std::vector<float> normal_buffer_data;
    std::vector<float> uv_buffer_data;
    std::vector<unsigned int> index_buffer_data;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint indexBufferID;

    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint viewPosID;

    GLuint depthMapFBO;
    GLuint shadowMapTexture;
    GLuint shadowProgramID;
    GLuint lightSpaceMatrixID;
    GLuint lightSpaceMatrixID2;
    GLuint shadowMapID;
    glm::mat4 lightSpaceMatrix;

    void initialize(glm::vec3 pos, glm::vec3 scl) {
        position = pos;
        scale = scl;

        const char* modelPath = "../lab2/models/tea/20900_Brown_Betty_Teapot_v1.obj";


        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials; // Won't be used
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath)) {
            std::cerr << "Error loading model: " << err << std::endl;
            return;
        }

        // Process geometry
        for (const auto& shape : shapes) {
            for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
                for (size_t v = 0; v < 3; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[3 * f + v];

                    // Vertices
                    vertex_buffer_data.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    vertex_buffer_data.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertex_buffer_data.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

                    // Normals
                    if (idx.normal_index >= 0) {
                        normal_buffer_data.push_back(attrib.normals[3 * idx.normal_index + 0]);
                        normal_buffer_data.push_back(attrib.normals[3 * idx.normal_index + 1]);
                        normal_buffer_data.push_back(attrib.normals[3 * idx.normal_index + 2]);
                    } else {
                        // Add default normal if none exists
                        normal_buffer_data.push_back(0.0f);
                        normal_buffer_data.push_back(1.0f);
                        normal_buffer_data.push_back(0.0f);
                    }

                    // UVs
                    if (idx.texcoord_index >= 0) {
                        uv_buffer_data.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        uv_buffer_data.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        // Add default UV if none exists
                        uv_buffer_data.push_back(0.0f);
                        uv_buffer_data.push_back(0.0f);
                    }

                    // Index
                    index_buffer_data.push_back(index_buffer_data.size());
                }
            }
        }

        // Create and bind VAO
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Create and bind vertex buffer
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);

        // Create and bind normal buffer
        glGenBuffers(1, &normalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glBufferData(GL_ARRAY_BUFFER, normal_buffer_data.size() * sizeof(float), normal_buffer_data.data(), GL_STATIC_DRAW);

        // Create and bind UV buffer
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, uv_buffer_data.size() * sizeof(float), uv_buffer_data.data(), GL_STATIC_DRAW);

        // Create and bind index buffer
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(unsigned int), index_buffer_data.data(), GL_STATIC_DRAW);

        // Load shaders
        programID = LoadShadersFromFile("../lab2/shaders/tea.vert", "../lab2/shaders/tea.frag");
        if (programID == 0) {
            std::cerr << "Failed to load shaders." << std::endl;
            return;
        }
        initLightUniforms();

        shadowProgramID = LoadShadersFromFile("../lab2/shaders/shadow.vert", "../lab2/shaders/shadow.frag");
        if (shadowProgramID == 0) {
            std::cerr << "Failed to load shadow shaders." << std::endl;
            return;
        }


        // Get uniform locations
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        modelMatrixID = glGetUniformLocation(programID, "model");
        viewPosID = glGetUniformLocation(programID, "viewPos");
        lightSpaceMatrixID2 = glGetUniformLocation(programID, "lightSpaceMatrix");
        shadowMapID = glGetUniformLocation(programID, "shadowMap");
        lightSpaceMatrixID = glGetUniformLocation(shadowProgramID, "lightSpaceMatrix");

        // Correct rotation
        rotation.z = 90.0f;

    }

    void render(glm::mat4 viewProjectionMatrix, glm::vec3 cameraPos) {
        glUseProgram(programID);

        glBindVertexArray(vertexArrayID);

        // Enable vertex attributes
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // Calculate matrices
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, scale);

        glm::mat4 mvp = viewProjectionMatrix * modelMatrix;
        // Set uniforms
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform3fv(viewPosID, 1, &cameraPos[0]);

        setRotation(glm::vec3(90.0f, 90.0f, rotation.z));

        // Draw model
        glDrawElements(GL_TRIANGLES, index_buffer_data.size(), GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    void cleanup() override {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &normalBufferID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteProgram(programID);
    }

};

#endif

