#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 model;
uniform vec3 lightPosition;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec3 fragmentLightDir;
out vec2 fragmentUV;

uniform mat4 lightSpaceMatrix;
out vec4 fragPosLightSpace;

void main() {
    vec4 worldPosition = model * vec4(vertexPosition, 1.0);
    fragmentPosition = worldPosition.xyz;

    fragmentNormal = vec3(0.0, 1.0, 0.0); 

    fragmentLightDir = normalize(lightPosition - fragmentPosition);

    fragmentUV = vertexUV;
    gl_Position = MVP * vec4(vertexPosition, 1.0);

    fragPosLightSpace = lightSpaceMatrix * model * vec4(vertexPosition, 1.0);

}