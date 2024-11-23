#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;

out vec3 reflectionVector;
out vec3 worldPos;
out vec3 normal;

void main() {
    worldPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;
    vec3 viewDir = normalize(cameraPosition - worldPos);
    reflectionVector = reflect(-viewDir, normal);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}