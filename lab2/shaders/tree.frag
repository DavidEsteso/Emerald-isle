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
    vec3 reflection = normalize(reflectionVector);
    reflection.y = -reflection.y;
    vec4 reflectionColor = textureLod(cubemap, reflection, 0.0);

    vec3 viewDir = normalize(cameraPosition - worldPos);

    float fresnelFactor = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);

    vec3 silverBaseColor = vec3(0.75, 0.75, 0.78);

    vec3 lightDir = normalize(lightPosition - worldPos);

    float diffuseIntensity = max(dot(normal, lightDir), 0.0);

    float darkness = pow(diffuseIntensity, 0.2);
    darkness = darkness * darkness;

    vec3 finalColor = mix(
    silverBaseColor,
    reflectionColor.rgb,
    fresnelFactor * 0.9
    );

    finalColor *= darkness;

    // Reduce ambient lighting significantly
    finalColor *= 0.3;

    // Specular component with darkness scaling
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), 100.0);
    vec3 specularLight = lightColor * specularStrength * 0.3 * darkness;

    finalColor += specularLight * 0.5;

    float specularIntensity = pow(max(dot(reflect(-viewDir, normal), reflection), 0.0), 100.0);
    finalColor += vec3(specularIntensity) * 0.1 * darkness;

    float luminance = dot(finalColor, vec3(0.299, 0.587, 0.114));
    finalColor = mix(finalColor, vec3(luminance), 0.3);

    FragColor = vec4(finalColor, 1.0);
}