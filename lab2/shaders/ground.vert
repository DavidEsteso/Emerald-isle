#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 3) in vec2 vertexUV;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 MVP;
uniform mat4 model;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1);
    FragPos = vec3(model * vec4(vertexPosition, 1.0));
    Normal = vec3(0.0, 1.0, 0.0);
    UV = vertexUV;
}
