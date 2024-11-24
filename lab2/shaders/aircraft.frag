#version 330 core

in vec2 UV;
out vec4 color;

uniform sampler2D diffuseTexture;
uniform float opacity;

void main() {
    color = texture(diffuseTexture, UV) * vec4(1.0, 1.0, 1.0, opacity);
}