#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 MVP;
out vec2 TexCoord;
flat out int faceIndex;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    faceIndex = gl_VertexID / 4; // Determine which face we're on
}