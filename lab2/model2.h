#ifndef MODEL_OBJ_H2
#define MODEL_OBJ_H2

#include <algorithm>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tinyobjloader/tiny_obj_loader.h>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include <chrono>

struct KeyFrame {
    float time; 
    glm::vec3 position;
};

float smoothEaseInOut(float x) {
    float t = (1 - cos(x * M_PI)) / 2;
    return t * t * (3 - 2 * t);
}

float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}


struct MaterialAir {
    GLuint diffuseMap;
    GLuint specularMap;
    GLuint normalMap;
    GLuint reflectionMap;
    float shininess;
    float opacity;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    glm::vec3 ambientColor;
};

GLuint LoadTextureAir(const std::string& path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::string normalizedPath = path;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

    std::cout << "Attempting to load texture: " << normalizedPath << std::endl;

    FILE* file = fopen(normalizedPath.c_str(), "rb");

    if (!file) {
        std::cerr << "File does not exist: " << normalizedPath << std::endl;
        unsigned char defaultColor[] = {128, 128, 128, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultColor);
        return textureID;
    }
    fclose(file);

    int width = 0, height = 0, nrChannels = 0;
    try {
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(normalizedPath.c_str(), &width, &height, &nrChannels, 0);

        if (!data) {
            std::cerr << "Failed to load texture data: " << normalizedPath << std::endl;
            std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
            // Crear una textura gris por defecto
            unsigned char defaultColor[] = {128, 128, 128, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultColor);
            return textureID;
        }

        GLenum format;
        GLenum internalFormat;
        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB;
        }
        else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA;
        }
        else {
            std::cerr << "Unexpected number of channels: " << nrChannels << std::endl;
            format = GL_RGB;
            internalFormat = GL_RGB;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);


    } catch (const std::exception& e) {
        std::cerr << "Exception while loading texture: " << e.what() << std::endl;
        unsigned char defaultColor[] = {128, 128, 128, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultColor);
    }

    std::cout << "Texture loaded successfully:" << std::endl;

    return textureID;


}


struct Aircraft {
    glm::vec3 position;
    glm::vec3 scale;

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

    GLuint programID;
    GLuint mvpMatrixID;
    GLuint modelMatrixID;
    GLuint viewPosID;

    std::vector<MaterialAir> materials;

    std::vector<KeyFrame> keyframes;
    float animationDuration;
    float currentTime;
    float floatAmplitude;
    float floatScale;
    float floatSpeed;
    glm::vec3 basePosition;

    struct MovementRange {
        float minHeight;
        float maxHeight;
        float primaryScale;
        float secondaryScale;
    } moveRange;

    void initialize(const char* modelPath, const char* materialBaseDir, glm::vec3 pos, glm::vec3 scl) {
        basePosition = pos;
        position = pos;
        scale = scl;

        floatAmplitude = 2.0f;
        floatScale = 1.5f;
        floatSpeed = 0.5f;
        currentTime = 0.0f;
        animationDuration = 4.0f;

        moveRange.minHeight = -3.0f;
        moveRange.maxHeight = 3.0f;
        moveRange.primaryScale = 1.5f;
        moveRange.secondaryScale = 0.3f;

        setupFloatingAnimation();


        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials_obj;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials_obj, &warn, &err, modelPath, materialBaseDir)) {
            std::cerr << "Error loading model: " << err << std::endl;
            return;
        }

        std::cout << "Model path used: " << modelPath << std::endl;
        std::cout << "Material base dir used: " << materialBaseDir << std::endl;
        std::cout << "Warning messages: " << warn << std::endl;
        std::cout << "Error messages: " << err << std::endl;
        std::cout << "Number of shapes loaded: " << shapes.size() << std::endl;
        std::cout << "Number of materials loaded: " << materials_obj.size() << std::endl;

        // Load materials first
        loadMaterials(materials_obj, materialBaseDir);

        std::cout << "Number of materials loaded: " << materials_obj.size() << std::endl;
        for (size_t i = 0; i < materials_obj.size(); i++) {
            const auto& mat = materials_obj[i];
            std::cout << "Material " << i << ":" << std::endl;
            std::cout << "  Diffuse texture: " << mat.diffuse_texname << std::endl;
            std::cout << "  Diffuse color: " << mat.diffuse[0] << " " << mat.diffuse[1] << " " << mat.diffuse[2] << std::endl;
            std::cout << "  Specular texture: " << mat.specular_texname << std::endl;
            std::cout << "  Normal texture: " << mat.bump_texname << std::endl;
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

        programID = LoadShadersFromFile("../lab2/shaders/aircraft.vert", "../lab2/shaders/aircraft.frag");
        if (programID == 0) {
            std::cerr << "Failed to load shaders." << std::endl;
            return;
        }

        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        modelMatrixID = glGetUniformLocation(programID, "model");
        viewPosID = glGetUniformLocation(programID, "viewPos");
    }

    void render(glm::mat4 viewProjectionMatrix, glm::vec3 cameraPos) {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        update(deltaTime);

        glUseProgram(programID);

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
                // Draw the previous block (if any)
                if (lastMaterialID >= 0) {
                    size_t count = i - startIndex;
                    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));
                }

                // Change material
                if (currentMaterialID >= 0 && currentMaterialID < materials.size())
                {
                    const MaterialAir& material = materials[currentMaterialID];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, material.diffuseMap);
                    glUniform1i(glGetUniformLocation(programID, "diffuseTexture"), 0);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, material.specularMap);
                    glUniform1i(glGetUniformLocation(programID, "specularTexture"), 1);

                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, material.normalMap);
                    glUniform1i(glGetUniformLocation(programID, "normalMap"), 2);

                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, material.reflectionMap);
                    glUniform1i(glGetUniformLocation(programID, "reflectionMap"), 3);

                    // Set material properties
                    glUniform1f(glGetUniformLocation(programID, "shininess"), material.shininess);
                    glUniform1f(glGetUniformLocation(programID, "opacity"), material.opacity);
                    glUniform3fv(glGetUniformLocation(programID, "diffuseColor"), 1, &material.diffuseColor[0]);
                    glUniform3fv(glGetUniformLocation(programID, "specularColor"), 1, &material.specularColor[0]);
                    glUniform3fv(glGetUniformLocation(programID, "ambientColor"), 1, &material.ambientColor[0]);

                    startIndex = i;
                }
            }

            lastMaterialID = currentMaterialID;
        }

        size_t count = material_indices.size() - startIndex;
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(startIndex * sizeof(unsigned int)));

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

        void setupFloatingAnimation() {
        keyframes.clear();

        const int numKeyframes = 12;
        for (int i = 0; i < numKeyframes; i++) {
            float t = (float)i / (numKeyframes - 1) * animationDuration;

            float phase = (t / animationDuration) * 2.0f * M_PI;

            float primaryMotion = sin(phase) * moveRange.primaryScale;
            float secondaryMotion = sin(phase * 2.0f) * moveRange.secondaryScale;

            float heightOffset = (primaryMotion + secondaryMotion) * floatAmplitude * floatScale;

            heightOffset = std::clamp(heightOffset, moveRange.minHeight, moveRange.maxHeight);

            float y = basePosition.y + heightOffset;

            keyframes.push_back({t, glm::vec3(basePosition.x, y, basePosition.z)});
        }
    }

    glm::vec3 interpolatePosition(float time) {
        time = fmod(time, animationDuration);

        KeyFrame* k1 = nullptr;
        KeyFrame* k2 = nullptr;

        for (size_t i = 0; i < keyframes.size() - 1; i++) {
            if (time >= keyframes[i].time && time <= keyframes[i + 1].time) {
                k1 = &keyframes[i];
                k2 = &keyframes[i + 1];
                break;
            }
        }

        if (!k1 || !k2) {
            return position;
        }

        float t = (time - k1->time) / (k2->time - k1->time);
        t = easeInOutCubic(t);

        // Enhanced smoothing for larger range
        float extremeThreshold = 0.25f; // Aumentado para suavizar más en los extremos
        if (t < extremeThreshold) {
            t = smoothstep(0.0f, extremeThreshold, t) * extremeThreshold;
        } else if (t > (1.0f - extremeThreshold)) {
            t = (1.0f - extremeThreshold) + smoothstep(0.0f, extremeThreshold, t - (1.0f - extremeThreshold)) * extremeThreshold;
        }

        glm::vec3 interpolatedPos = k1->position * (1.0f - t) + k2->position * t;

        // Add enhanced secondary motion
        float secondaryMotion = sin(time * 2.5f) * 0.15f * floatAmplitude * moveRange.secondaryScale;
        interpolatedPos.y += secondaryMotion;

        return interpolatedPos;
    }

    void update(float deltaTime) {
        currentTime += deltaTime * floatSpeed;

        float cyclePosition = fmod(currentTime, animationDuration) / animationDuration;

        float angle = cyclePosition * 2.0f * M_PI;

        float baseOffset = sin(angle);

        float transitionZone = 0.2f;

        float distToTop = abs(angle - M_PI * 0.5f);
        float distToBottom = abs(angle - M_PI * 1.5f);
        float minDist = std::min(distToTop, distToBottom);

        float verticalOffset;
        if (minDist < transitionZone) {
            float t = 1.0f - (minDist / transitionZone);
            float smoothFactor = smoothEaseInOut(t);

            if (distToTop < distToBottom) {
                verticalOffset = floatAmplitude * (1.0f - (1.0f - smoothFactor) * (1.0f - sin(angle)));
            } else {
                verticalOffset = floatAmplitude * (-1.0f + (1.0f - smoothFactor) * (1.0f + sin(angle)));
            }
        } else {
            verticalOffset = floatAmplitude * baseOffset;
        }

        position = basePosition + glm::vec3(0.0f, verticalOffset, 0.0f);
    }


    void loadMaterials(const std::vector<tinyobj::material_t>& materials_obj, const char* materialBaseDir) {
        materials.resize(materials_obj.size());
        for (size_t i = 0; i < materials_obj.size(); i++) {
            const auto& mat = materials_obj[i];

            materials[i].diffuseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            materials[i].specularColor = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
            materials[i].ambientColor = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
            materials[i].shininess = mat.shininess;
            materials[i].opacity = mat.dissolve;

            if (!mat.diffuse_texname.empty()) {
                std::string texturePath = std::string(materialBaseDir) + "/" + mat.diffuse_texname;
                materials[i].diffuseMap = LoadTextureAir(texturePath);
            }

            if (!mat.specular_texname.empty()) {
                std::string specPath = std::string(materialBaseDir) + "/" + mat.specular_texname;
                materials[i].specularMap = LoadTextureAir(specPath);
            }

            if (!mat.bump_texname.empty()) {
                std::string bumpPath = std::string(materialBaseDir) + "/" + mat.bump_texname;
                materials[i].normalMap = LoadTextureAir(bumpPath);
            }

            GLuint defaultReflectionMap;
            glGenTextures(1, &defaultReflectionMap);
            glBindTexture(GL_TEXTURE_2D, defaultReflectionMap);

            unsigned char defaultReflColor[] = {128, 128, 128, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultReflColor);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            materials[i].reflectionMap = defaultReflectionMap;
        }
    }
};

#endif
