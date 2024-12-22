#version 330 core
in vec2 UV;
in vec3 Normal;
in vec3 Position;
in vec3 FragPos;

out vec4 color;

// Uniforms de texturas
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalMap;
uniform sampler2D reflectionMap;
uniform float opacity;
uniform bool enableContour;
uniform vec3 viewPos;

// Uniforms de luz
uniform vec3 lightPos;      // Posición de la luz
uniform vec3 lightColor;    // Color de la luz
uniform float lightIntensity; // Intensidad de la luz

void main() {
    vec4 diffuseColor = texture(diffuseTexture, UV);

    vec4 specularValue = texture(specularTexture, UV);

    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor.rgb * lightColor * lightIntensity;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * specularValue.rgb * lightColor * lightIntensity;

    vec4 reflectionValue = texture(reflectionMap, UV);
    vec3 reflection = reflectionValue.rgb * 0.3;



    vec3 finalColor = diffuse + specular + reflection;

    if (enableContour) {
        float outline = 1.0 - abs(dot(normal, viewDir));
        outline = smoothstep(0.0, 0.5, outline);
        if (outline < 0.3) {
            color = vec4(0.0, 0.0, 0.0, opacity);
        } else {
            color = vec4(finalColor, opacity);
        }
    } else {
        color = vec4(finalColor, opacity);
    }
}