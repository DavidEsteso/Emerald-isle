#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

out vec2 UV;
out vec3 Normal;
out vec3 Position;

uniform mat4 MVP;
uniform mat4 model;
uniform bool enableContour;

void main() {
    vec3 adjustedPosition = vertexPosition;
    if (enableContour) {
        adjustedPosition += vertexNormal * 0.03;
    }

    gl_Position = MVP * vec4(adjustedPosition, 1);
    UV = vertexUV;
    Normal = mat3(model) * vertexNormal;
    Position = vec3(model * vec4(vertexPosition, 1.0));
}