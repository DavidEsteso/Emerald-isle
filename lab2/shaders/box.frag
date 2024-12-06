#version 330 core
in vec3 reflectionVector;
in vec3 worldPos;
in vec3 normal;

uniform samplerCube cubemap;
uniform vec3 cameraPosition;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightColor;

out vec4 FragColor;

void main() {
    // Reflection calculations
    vec3 reflection = normalize(reflectionVector);
    reflection.y = -reflection.y;
    vec4 reflectionColor = textureLod(cubemap, reflection, 0.0);

    vec3 viewDir = normalize(cameraPosition - worldPos);

    float fresnelFactor = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);

    // Base silver color
    vec3 silverBaseColor = vec3(0.75, 0.75, 0.78);

    vec3 lightDir = normalize(lightPosition - worldPos);
    float lightDistance = length(lightPosition - worldPos);

    vec3 ambientLight = lightColor * 0.1;

    float diffuseIntensity = max(dot(normal, lightDir), 0.0);
    float attenuation = 1.0 / (1.0 + 0.1 * lightDistance + 0.01 * lightDistance * lightDistance);
    vec3 diffuseLight = lightColor * diffuseIntensity * lightIntensity * attenuation;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularStrength = pow(max(dot(normal, halfwayDir), 0.0), 50.0);
    vec3 specularLight = lightColor * specularStrength * 0.5 * attenuation;

    // Combine reflection and lighting
    vec3 finalColor = mix(
    silverBaseColor,
    reflectionColor.rgb,
    fresnelFactor * 0.9
    );

    // Add lighting components
    finalColor += ambientLight + diffuseLight + specularLight;

    // Original shader's specular reflection
    float originalSpecularIntensity = pow(max(dot(reflect(-viewDir, normal), reflection), 0.0), 50.0);
    finalColor += vec3(originalSpecularIntensity) * 0.2;

    // Luminance adjustment
    float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(finalColor, vec3(luminance), 0.2);

    FragColor = vec4(finalColor, 1.0);
}