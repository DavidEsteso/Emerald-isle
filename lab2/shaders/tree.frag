#version 330 core

in vec2 TexCoords;

out vec4 FragColor;
uniform sampler2D diffuseSampler;
uniform sampler2D specularSampler;

void main() {
    vec3 diffuseMap = texture(diffuseSampler, TexCoords).rgb;
    vec3 specularMap = texture(specularSampler, TexCoords).rgb;


    FragColor = vec4(diffuseMap + specularMap, 1.0);
}
