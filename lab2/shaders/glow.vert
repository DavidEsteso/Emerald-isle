#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 model;
uniform vec3 viewPos;

out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragPos;

void main() {
    vec3 offset = vertexNormal * 0.05;
    vec4 worldPos = model * vec4(vertexPosition + offset, 1.0);
    gl_Position = MVP * worldPos;

    fragNormal = mat3(model) * vertexNormal;
    fragUV = vertexUV;
    fragPos = worldPos.xyz;
}