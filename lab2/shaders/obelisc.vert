#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

out vec3 fragPos;
out vec3 fragNormal;


//
uniform mat4 MVP;
uniform mat4 model;

void main() {
    fragPos = (model * vec4(vertexPosition, 1.0)).xyz;
    fragNormal = normalize(transpose(inverse(mat3(model))) * vertexNormal);

    gl_Position = MVP * vec4(vertexPosition, 1.0);
}
