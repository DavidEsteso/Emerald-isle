#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

void main() {
    float chessSize = 20.0;

    vec3 colorA = vec3(0.5, 0.5, 0.5);
    vec3 colorB = vec3(0.1, 0.1, 0.1);


    vec2 patternCoord = TexCoords * chessSize;

    vec2 cell = floor(patternCoord);

    float pattern = mod(cell.x + cell.y, 2.0);

    vec3 color = (pattern < 1.0) ? colorA : colorB;

    FragColor = vec4(color, 1.0);
}