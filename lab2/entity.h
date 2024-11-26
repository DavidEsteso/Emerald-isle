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
    virtual void cleanup() = 0;

    virtual ~Entity() = default;
};

#endif

#endif //ENTITY_H
