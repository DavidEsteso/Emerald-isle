#version 330 core
in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

out vec4 color;

uniform sampler2D diffuseTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float lightIntensity;

const float ambientStrength = 0.4;
const float specularStrength = 0.5;
const float shininess = 32.0;

void main() {
    vec3 baseColor = texture(diffuseTexture, UV).rgb;

    vec3 ambient = ambientStrength * lightColor;

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 finalColor = (ambient + diffuse + specular) * baseColor * lightIntensity;

    color = vec4(finalColor, 1.0);
}