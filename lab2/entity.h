//
// Created by david on 26/11/2024.
//

#ifndef ENTITY_H
#define ENTITY_H

#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Entity {
public:
    static inline glm::vec3 LightColor = glm::vec3(1.0f, 0.5f, 0.7f);
    static inline glm::vec3 wave500{0.0f, 255.0f, 146.0f};
    static inline glm::vec3 wave600{255.0f, 190.0f, 0.0f};
    static inline glm::vec3 wave700{205.0f, 0.0f, 0.0f};
    static inline glm::vec3 lightIntensity = 3.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
    static inline glm::vec3 LightPosition = glm::vec3(300.0f, 200.0f, 300.0f);

    GLuint vertexBufferID;
    GLuint indexBufferID;

    glm::vec3 position;		// Position of the box
    glm::vec3 scale;		// Size of the box in each axis
    glm::mat4 modelMatrix;	// Model matrix for the box

    virtual void cleanup() = 0;

    virtual ~Entity() = default;

    virtual void renderForShadows(const glm::mat4& lightSpaceMatrix, GLuint shadowProgramID) = 0;
};

#endif

#endif //ENTITY_H
