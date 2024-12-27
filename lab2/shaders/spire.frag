#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D diffuseMap;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

void main() {
    float ambientStrength = 0.4;
    float specularStrength = 0.5;
    vec3 baseColor = vec3(0.6, 0.6, 0.6);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = ambientStrength * baseColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor * lightColor;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 finalColor = ambient + (diffuse + specular) * lightIntensity;
    FragColor = vec4(finalColor, 1.0);
}