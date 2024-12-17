#version 330 core

in vec2 fragmentUV;
in vec3 fragmentNormal;
in vec3 fragmentPosition;
in vec4 fragPosLightSpace;

uniform sampler2D textureSampler;
uniform sampler2D shadowMap;
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightColor;
uniform mat4 lightSpaceMatrix;

out vec4 color;

float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projectedCoords = projectedCoords * 0.5 + 0.5;

    float closestDepthFromLight = texture(shadowMap, projectedCoords.xy).r;

    float currentFragmentDepth = projectedCoords.z;

    float shadowFactor = (projectedCoords.z >= 0 && currentFragmentDepth >= closestDepthFromLight + 1e-3) ? 0.2 : 1.0;

    return shadowFactor;
}

void main() {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float depth = texture(shadowMap, projCoords.xy).r;

    if(depth < 0.2) color = vec4(1.0, 0.0, 0.0, 1.0);
    else if(depth < 0.5) color = vec4(0.0, 1.0, 0.0, 1.0);
    else color = vec4(0.0, 0.0, 1.0, 1.0);
}