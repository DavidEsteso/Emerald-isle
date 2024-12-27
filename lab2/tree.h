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
#include <unordered_map>

struct Material {
    GLuint diffuseMap;
    GLuint specularMap;
    float shininess;
};

GLuint LoadTexture(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

struct Tree : public Entity {

    std::vector<float> vertex_buffer_data;
    std::vector<float> normal_buffer_data;
    std::vector<float> uv_buffer_data;
    std::vector<unsigned int> index_buffer_data;
    std::vector<int> material_indices;

    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint indexBufferID;

    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint viewPosID;


    std::vector<Material> materials;

    void initialize(glm::vec3 pos, glm::vec3 scl) {
        position = pos;
        scale = scl;
        const char* modelPath = "../lab2/models/Tree/Tree.obj";
        const char* materialBaseDir = "../lab2/models/Tree";

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials_obj;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials_obj, &warn, &err, modelPath, materialBaseDir)) {
            std::cerr << "Error loading model: " << err << std::endl;
            return;
        }

        // Load materials first
        materials.resize(materials_obj.size());
        for (size_t i = 0; i < materials_obj.size(); i++) {
            const auto& mat = materials_obj[i];

            // Load diffuse texture
            if (!mat.diffuse_texname.empty()) {
                std::string texturePath = std::string(materialBaseDir) + "/" + mat.diffuse_texname;
                std::cout << "Loading diffuse texture: " << texturePath << std::endl;
                materials[i].diffuseMap = LoadTexture(texturePath);
            }

            // Load specular texture
            if (!mat.specular_texname.empty()) {
                std::string specPath = std::string(materialBaseDir) + "/" + mat.specular_texname;
                std::cout << "Loading specular texture: " << specPath << std::endl;
                materials[i].specularMap = LoadTexture(specPath);
            }

            materials[i].shininess = mat.shininess;
        }

        // Load geometry
        for (const auto& shape : shapes) {
            for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
                int materialId = shape.mesh.material_ids[f];
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
                    }

                    // UVs
                    if (idx.texcoord_index >= 0) {
                        uv_buffer_data.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                        uv_buffer_data.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    } else {
                        uv_buffer_data.push_back(0.0f);
                        uv_buffer_data.push_back(0.0f);
                    }

                    // Material index
                    material_indices.push_back(materialId);

                    // Index
                    index_buffer_data.push_back(index_buffer_data.size());
                }
            }
        }

        // Create and bind buffers
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(float), vertex_buffer_data.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &normalBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
        glBufferData(GL_ARRAY_BUFFER, normal_buffer_data.size() * sizeof(float), normal_buffer_data.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, uv_buffer_data.size() * sizeof(float), uv_buffer_data.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(unsigned int), index_buffer_data.data(), GL_STATIC_DRAW);

        programID = LoadShadersFromFile("../lab2/shaders/tree.vert", "../lab2/shaders/tree.frag");
        if (programID == 0) {
            std::cerr << "Failed to load shaders." << std::endl;
            return;
        }

        initLightUniforms();


        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        modelMatrixID = glGetUniformLocation(programID, "model");
        viewPosID = glGetUniformLocation(programID, "viewPos");



    }


    void render(glm::mat4 viewProjectionMatrix, glm::vec3 cameraPos) {
        glUseProgram(programID);

        glBindVertexArray(vertexArrayID);


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

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(-1.0f, 0.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, scale);

        glm::mat4 mvp = viewProjectionMatrix * modelMatrix;

        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniform3fv(viewPosID, 1, &cameraPos[0]);

        int lastMaterialID = -1;
        size_t startIndex = 0;

        for (size_t i = 0; i < material_indices.size(); i++) {
            int currentMaterialID = material_indices[i];

            if (currentMaterialID != lastMaterialID) {
                if (lastMaterialID >= 0) {
                    size_t count = i - startIndex;
                    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }

                if (currentMaterialID >= 0 && currentMaterialID < materials.size()) {
                    const Material& material = materials[currentMaterialID];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, material.diffuseMap);
                    glUniform1i(glGetUniformLocation(programID, "diffuseSampler"), 0);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, material.specularMap);
                    glUniform1i(glGetUniformLocation(programID, "specularSampler"), 1);

                    glUniform1f(glGetUniformLocation(programID, "shininess"), material.shininess);
                }

                lastMaterialID = currentMaterialID;
                startIndex = i;
            }
        }

        // Draw the last batch of triangles
        if (lastMaterialID >= 0) {
            size_t count = material_indices.size() - startIndex;
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    void cleanup() override{
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &normalBufferID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteProgram(programID);

        for (const auto& material : materials) {
            glDeleteTextures(1, &material.diffuseMap);
            if (material.specularMap) {
                glDeleteTextures(1, &material.specularMap);
            }
        }

    }

};

#endif