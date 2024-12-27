//
// Created by david on 26/11/2024.
//

#ifndef ENTITY_H
#define ENTITY_H



#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Entity {
public:
    GLuint vertexBufferID;
    GLuint indexBufferID;

    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    glm::mat4 modelMatrix;

    GLuint programID;

    GLint uLightPos;
    GLint uLightColor;
    GLint uViewPos;
    GLint uLightIntensity;

    glm::vec3 lightPos;
    glm::vec3 lightColor;
    glm::vec3 viewPos;
    float lightIntensity;


    virtual void cleanup() = 0;

    virtual ~Entity() = default;

    // Setters
    virtual void setPosition(const glm::vec3& newPosition) {
        position = newPosition;
        updateModelMatrix();
    }

    virtual void setScale(const glm::vec3& newScale) {
        scale = newScale;
        updateModelMatrix();
    }

    virtual void setRotation(const glm::vec3& newRotation) {
        rotation = newRotation;
        updateModelMatrix();
    }

    virtual void updateModelMatrix() {
        modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::translate(modelMatrix, position);

        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        modelMatrix = glm::scale(modelMatrix, scale);
    }

    virtual void setLightUniforms(const glm::vec3& lightPos, const glm::vec3& lightColor,
                             const glm::vec3& viewPos, float intensity) {
        glUseProgram(programID);
        glUniform3fv(uLightPos, 1, &lightPos[0]);
        glUniform3fv(uLightColor, 1, &lightColor[0]);
        glUniform3fv(uViewPos, 1, &viewPos[0]);
        glUniform1f(uLightIntensity, intensity);

    }

    virtual void initLightUniforms() {
        uLightPos = glGetUniformLocation(programID, "lightPos");
        uLightColor = glGetUniformLocation(programID, "lightColor");
        uViewPos = glGetUniformLocation(programID, "viewPos");
        uLightIntensity = glGetUniformLocation(programID, "lightIntensity");
    }

    const glm::vec3& getPosition() const { return position; }
    const glm::mat4& getModelMatrix() const { return modelMatrix; }
};

#endif

