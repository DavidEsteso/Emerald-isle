#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubemapSampler;

void main()
{
    FragColor = texture(cubemapSampler, TexCoords);
}