#version 330 core

in vec2 UV;
out vec4 color;

uniform sampler2D diffuseSampler;

void main() {
    // Visualizar las coordenadas UV como color para debug
    // color = vec4(UV.x, UV.y, 0.0, 1.0);  // Descomenta esta línea para ver el mapeo UV

    vec4 texColor = texture(diffuseSampler, UV);
    if(texColor.a < 0.1) {  // Si el pixel es muy transparente
        discard;  // Descartarlo
    }
    color = texColor;
}