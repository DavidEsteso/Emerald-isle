#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;
uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

void main() {
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * lightColor;

    vec3 diffuseMap = texture(diffuseSampler, TexCoords).rgb;
    vec3 specularMap = texture(specularSampler, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * (diffuseMap + specularMap) * lightIntensity;

    FragColor = vec4(result, 1.0);
}