#version 330 core


layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

out vec3 fragmentColor;
out vec3 fragmentNormal;
out vec3 fragmentPosition;

uniform mat4 MVP;
uniform mat4 model;

void main() {
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);

    fragmentNormal = normalize(mat3(transpose(inverse(model))) * vertexNormal);

    fragmentColor = abs(fragmentNormal);

    fragmentPosition = vec3(model * vec4(vertexPosition_modelspace, 1.0));
}