#ifndef MODEL_OBJ_H
#define MODEL_OBJ_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tinyobjloader/tiny_obj_loader.h>
#include <vector>
#include <string>
#include <iostream>
#include <render/shader.h>
#include <fstream>

GLuint LoadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    if (textureID == 0) {
        std::cerr << "Error generando textura" << std::endl;
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Formato de imagen no soportado: " << nrChannels << " canales" << std::endl;
            stbi_image_free(data);
            return 0;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "Textura cargada: " << path << std::endl;
        std::cout << "Dimensiones: " << width << "x" << height << std::endl;
        std::cout << "Canales: " << nrChannels << std::endl;
    } else {
        std::cerr << "Error cargando la textura: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    stbi_image_free(data);
    return textureID;
}


struct ModelOBJ {
    glm::vec3 position;
    glm::vec3 scale;

    // Mesh data
    std::vector<float> vertex_buffer_data;
    std::vector<float> normal_buffer_data;
    std::vector<float> uv_buffer_data;
    std::vector<unsigned int> index_buffer_data;

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint indexBufferID;
    GLuint textureID;
    // Load texture
    GLuint diffuseTexture ;
    GLuint normalTexture ;
    GLuint specularTexture;
    GLuint refTexture;

    // Shader variables
    GLuint programID;
    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint viewPosID;
    GLuint textureSamplerID;

    void initialize(const char* modelPath, glm::vec3 pos, glm::vec3 scl) {
        position = pos;
        scale = scl;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath)) {
            std::cerr << "Error loading model: " << err << std::endl;
            return;
        }

        int total_vertices = 0;
        int invalid_uvs = 0;
        int out_of_range_uvs = 0;

        // Process the model data
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                total_vertices++;

                // Vertex positions
                vertex_buffer_data.push_back(attrib.vertices[3 * index.vertex_index + 0]);
                vertex_buffer_data.push_back(attrib.vertices[3 * index.vertex_index + 1]);
                vertex_buffer_data.push_back(attrib.vertices[3 * index.vertex_index + 2]);

                // Normals (si existen)
                if (index.normal_index >= 0) {
                    normal_buffer_data.push_back(attrib.normals[3 * index.normal_index + 0]);
                    normal_buffer_data.push_back(attrib.normals[3 * index.normal_index + 1]);
                    normal_buffer_data.push_back(attrib.normals[3 * index.normal_index + 2]);
                }

                // Texture coordinates
                if (index.texcoord_index >= 0) {
                    float u = attrib.texcoords[2 * index.texcoord_index + 0];
                    float v = attrib.texcoords[2 * index.texcoord_index + 1];

                    if (u < 0.0f) u = 0.0f;
                    if (u > 1.0f) u = 1.0f;
                    if (v < 0.0f) v = 0.0f;
                    if (v > 1.0f) v = 1.0f;


                    uv_buffer_data.push_back(u);
                    uv_buffer_data.push_back(v);
                } else {
                    // Si no hay coordenadas UV, usar valores por defecto
                    uv_buffer_data.push_back(0.0f);
                    uv_buffer_data.push_back(0.0f);
                    invalid_uvs++;
                }

                index_buffer_data.push_back(index_buffer_data.size());
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
        programID = LoadShadersFromFile("../lab2/shaders/model.vert", "../lab2/shaders/model.frag");
        if (programID == 0) {
            std::cerr << "Failed to load shaders for model." << std::endl;
            return;
        }

        // Get shader uniform locations
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        modelMatrixID = glGetUniformLocation(programID, "model");
        viewPosID = glGetUniformLocation(programID, "viewPos");
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");

        // Load texture
        diffuseTexture = LoadTexture("../lab2/textures/Futuristic_Car_C.jpg");
        std::cout << "Hello" << std::endl;

        normalTexture = LoadTexture("../lab2/textures/Futuristic_Car_N.jpg");
        std::cout << "Hello" << std::endl;

        specularTexture = LoadTexture("../lab2/textures/Futuristic_Car_S.jpg");
        std::cout << "Hello" << std::endl;

        refTexture = LoadTexture("../lab2/textures/REF1.jpg");
        std::cout << "Hello" << std::endl;

    }

    void render(glm::mat4 viewProjectionMatrix, glm::vec3 cameraPos) {
        glUseProgram(programID);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Normals
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // UVs
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Bind index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // Calculate model matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, scale);

        // Calculate and set MVP matrix
        glm::mat4 mvp = viewProjectionMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

        // Set camera position for lighting calculations
        glUniform3fv(viewPosID, 1, &cameraPos[0]);



        glActiveTexture(GL_TEXTURE0); // Primera textura: difusa
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        glUniform1i(glGetUniformLocation(programID, "diffuseSampler"), 0);

        glActiveTexture(GL_TEXTURE1); // Segunda textura: normal
        glBindTexture(GL_TEXTURE_2D, normalTexture);
        glUniform1i(glGetUniformLocation(programID, "normalSampler"), 1);

        glActiveTexture(GL_TEXTURE2); // Tercera textura: especular
        glBindTexture(GL_TEXTURE_2D, specularTexture);
        glUniform1i(glGetUniformLocation(programID, "specularSampler"), 2);

        glActiveTexture(GL_TEXTURE3); // Cuarta textura: reflexión
        glBindTexture(GL_TEXTURE_2D, refTexture);
        glUniform1i(glGetUniformLocation(programID, "reflectionSampler"), 3);

        // Draw the model
        glDrawElements(
            GL_TRIANGLES,
            index_buffer_data.size(),
            GL_UNSIGNED_INT,
            (void*)0
        );

        // Cleanup state
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &normalBufferID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);

        glDeleteProgram(programID);
    }

    glm::vec3 getPosition() const {
        return position;
    }
};

#endif