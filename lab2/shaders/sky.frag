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
    vec2 correctedTexCoord;

    switch(faceIndex) {
        case 0: // Front
        case 1: // Back
        case 2: // Left
        case 3: // Right
        correctedTexCoord = vec2(1.0 - TexCoord.x, 1.0 - TexCoord.y);
        break;
        case 4: // Top
        case 5: // Bottom

        correctedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
        break;
        default:
        correctedTexCoord = TexCoord;
    }
    vec4 texColor;
    switch(faceIndex) {
        case 0: texColor = texture(faceSampler0, correctedTexCoord); break;
        case 1: texColor = texture(faceSampler1, correctedTexCoord); break;
        case 2: texColor = texture(faceSampler2, correctedTexCoord); break;
        case 3: texColor = texture(faceSampler3, correctedTexCoord); break;
        case 4: texColor = texture(faceSampler4, correctedTexCoord); break;
        case 5: texColor = texture(faceSampler5, correctedTexCoord); break;
        default: texColor = vec4(1.0, 0.0, 0.0, 1.0); break;
    }
    FragColor = texColor;
}