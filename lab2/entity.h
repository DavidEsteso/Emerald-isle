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
protected:
    static inline glm::vec3 LightDirection = glm::normalize(glm::vec3(0.5f, 1.0f, -0.3f));
    static inline glm::vec3 LightColor = glm::vec3(1.0f, 0.5f, 0.7f);
    static inline glm::vec3 wave500{0.0f, 255.0f, 146.0f};
    static inline glm::vec3 wave600{255.0f, 190.0f, 0.0f};
    static inline glm::vec3 wave700{205.0f, 0.0f, 0.0f};
    static inline glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
    static inline glm::vec3 LightPosition = glm::vec3(300.0f, 200.0f, 300.0f);
public:
    virtual void cleanup() = 0;



    virtual ~Entity() = default;
};

#endif

#endif //ENTITY_H
