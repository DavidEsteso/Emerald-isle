#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;

uniform mat4 MVP;
uniform vec3 cameraPos;

out vec2 worldPos;
out vec2 UV;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1);
    worldPos = vertexPosition.xz;
    UV = vertexUV;
}
