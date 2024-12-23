#version 330 core
in vec3 reflectionVector;
in vec3 worldPos;
in vec3 normal;
in vec4 fragPosLightSpace;

uniform samplerCube cubemap;
uniform vec3 cameraPosition;

out vec4 FragColor;

void main() {
    vec3 reflection = normalize(reflectionVector);
    vec4 reflectionColor = textureLod(cubemap, reflection, 0.0);

    vec3 emeraldColor = vec3(0.0, 0.35, 0.15);

    vec3 viewDir = normalize(cameraPosition - worldPos);
    float fresnelFactor = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);

    vec3 finalColor = mix(
    emeraldColor,
    reflectionColor.rgb,
    fresnelFactor * 0.6
    );

    float crystalGlow = pow(max(dot(reflect(-viewDir, normal), vec3(0, 1, 0)), 0.0), 16.0);
    finalColor += vec3(0.0, 0.1, 0.05) * crystalGlow;

    FragColor = vec4(finalColor, 1.0);
}