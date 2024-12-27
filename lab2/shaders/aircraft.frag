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

uniform float shininess;
uniform float opacity;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform vec3 ambientColor;

void main() {
    vec3 ambient = ambientStrength * ambientColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * specularColor;

    vec3 result = (ambient + diffuse + specular) * lightColor * lightIntensity;
    color = texture(diffuseTexture, UV) * vec4(result, opacity);
}