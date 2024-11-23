#version 330 core

in vec3 reflectionVector;
in vec3 worldPos;
in vec3 normal;

uniform samplerCube cubemap;
uniform vec3 cameraPosition;

out vec4 FragColor;

void main() {
    vec3 reflection = normalize(reflectionVector);

    vec4 reflectionColor = textureLod(cubemap, reflection, 0.0);

    vec3 viewDir = normalize(cameraPosition - worldPos);
    float fresnelFactor = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);

    vec3 silverBaseColor = vec3(0.75, 0.75, 0.78);

    vec3 finalColor = mix(
    silverBaseColor,
    reflectionColor.rgb,
    fresnelFactor * 0.9
    );

    float specularIntensity = pow(max(dot(reflect(-viewDir, normal), reflection), 0.0), 50.0);
    finalColor += vec3(specularIntensity) * 0.2;

    float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(finalColor, vec3(luminance), 0.2);

    FragColor = vec4(finalColor, 1.0);
}