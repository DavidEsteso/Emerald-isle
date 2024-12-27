#version 330 core
in vec2 TexCoord;
flat in int faceIndex;
uniform sampler2D faceSampler0;
uniform sampler2D faceSampler1;
uniform sampler2D faceSampler2;
uniform sampler2D faceSampler3;
uniform sampler2D faceSampler4;
uniform sampler2D faceSampler5;
out vec4 FragColor;

void main() {
    vec2 texCoord = TexCoord;

    switch(faceIndex) {
        case 0: texCoord = vec2(1.0 - TexCoord.x, TexCoord.y); break;
        case 1: texCoord = vec2(1.0 - TexCoord.x, TexCoord.y); break;

        case 2: texCoord = vec2(1.0 - TexCoord.x, TexCoord.y); break;
        case 3: texCoord = vec2(1.0 - TexCoord.x, TexCoord.y); break;

        case 4: texCoord = vec2(TexCoord.x, TexCoord.y); break;
    }

    vec4 texColor;
    switch(faceIndex) {
        case 0: texColor = texture(faceSampler0, texCoord); break;
        case 1: texColor = texture(faceSampler1, texCoord); break;
        case 2: texColor = texture(faceSampler2, texCoord); break;
        case 3: texColor = texture(faceSampler3, texCoord); break;
        case 4: texColor = texture(faceSampler4, texCoord); break;
        default: texColor = vec4(1.0, 0.0, 0.0, 1.0); break;
    }
    FragColor = texColor;
}