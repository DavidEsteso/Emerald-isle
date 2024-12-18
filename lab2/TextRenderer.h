#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
class Frustum {
public:
    enum Planes {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5
    };

    glm::vec4 planes[6];

    void extractFrustumPlanes(const glm::mat4& viewProjectionMatrix) {
        planes[LEFT].x = viewProjectionMatrix[3][0] + viewProjectionMatrix[0][0];
        planes[LEFT].y = viewProjectionMatrix[3][1] + viewProjectionMatrix[0][1];
        planes[LEFT].z = viewProjectionMatrix[3][2] + viewProjectionMatrix[0][2];
        planes[LEFT].w = viewProjectionMatrix[3][3] + viewProjectionMatrix[0][3];

        planes[RIGHT].x = viewProjectionMatrix[3][0] - viewProjectionMatrix[0][0];
        planes[RIGHT].y = viewProjectionMatrix[3][1] - viewProjectionMatrix[0][1];
        planes[RIGHT].z = viewProjectionMatrix[3][2] - viewProjectionMatrix[0][2];
        planes[RIGHT].w = viewProjectionMatrix[3][3] - viewProjectionMatrix[0][3];

        // Plano inferior
        planes[BOTTOM].x = viewProjectionMatrix[3][0] + viewProjectionMatrix[1][0];
        planes[BOTTOM].y = viewProjectionMatrix[3][1] + viewProjectionMatrix[1][1];
        planes[BOTTOM].z = viewProjectionMatrix[3][2] + viewProjectionMatrix[1][2];
        planes[BOTTOM].w = viewProjectionMatrix[3][3] + viewProjectionMatrix[1][3];

        planes[TOP].x = viewProjectionMatrix[3][0] - viewProjectionMatrix[1][0];
        planes[TOP].y = viewProjectionMatrix[3][1] - viewProjectionMatrix[1][1];
        planes[TOP].z = viewProjectionMatrix[3][2] - viewProjectionMatrix[1][2];
        planes[TOP].w = viewProjectionMatrix[3][3] - viewProjectionMatrix[1][3];

        planes[NEAR].x = viewProjectionMatrix[3][0] + viewProjectionMatrix[2][0];
        planes[NEAR].y = viewProjectionMatrix[3][1] + viewProjectionMatrix[2][1];
        planes[NEAR].z = viewProjectionMatrix[3][2] + viewProjectionMatrix[2][2];
        planes[NEAR].w = viewProjectionMatrix[3][3] + viewProjectionMatrix[2][3];

        planes[FAR].x = viewProjectionMatrix[3][0] - viewProjectionMatrix[2][0];
        planes[FAR].y = viewProjectionMatrix[3][1] - viewProjectionMatrix[2][1];
        planes[FAR].z = viewProjectionMatrix[3][2] - viewProjectionMatrix[2][2];
        planes[FAR].w = viewProjectionMatrix[3][3] - viewProjectionMatrix[2][3];

        for (int i = 0; i < 6; i++) {
            float length = glm::length(glm::vec3(planes[i]));
            planes[i] /= length;
        }
    }

    bool sphereInFrustum(const glm::vec3& center, float radius) const {
        for (int i = 0; i < 6; i++) {
            float distance = planes[i].x * center.x +
                             planes[i].y * center.y +
                             planes[i].z * center.z +
                             planes[i].w;

            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }
};

#endif // TEXT_RENDERER_H