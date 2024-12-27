#version 330 core


layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in vec4 jointIndices;
layout(location = 4) in vec4 jointWeights;

uniform mat4 MVP;
uniform mat4 model;
uniform vec3 viewPos;
uniform mat4 jointMatrices[50];
uniform int isShoulderUniform;

out vec3 worldPosition;
out vec3 worldNormal;
flat out int isShoulderOut;

void main() {
    vec4 skinnedPosition = vec4(0.0);
    vec4 skinnedNormal = vec4(0.0);

    for(int i = 0; i < 4; i++) {
        int jointIndex = int(jointIndices[i]);
        if(jointIndex >= 0) {
            vec4 localPosition = jointMatrices[jointIndex] * vec4(vertexPosition, 1.0);
            skinnedPosition += jointWeights[i] * localPosition;

            vec4 localNormal = jointMatrices[jointIndex] * vec4(vertexNormal, 0.0);
            skinnedNormal += jointWeights[i] * localNormal;
        }
    }

    vec4 worldPos = model * skinnedPosition;
    worldPosition = worldPos.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    worldNormal = normalize(normalMatrix * skinnedNormal.xyz);


    gl_Position = MVP * skinnedPosition;

    isShoulderOut = isShoulderUniform;
}