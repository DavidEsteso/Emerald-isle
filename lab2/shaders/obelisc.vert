#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

// Outputs to fragment shader
out vec3 fragPos;
out vec3 fragNormal;


// Uniforms
uniform mat4 MVP;
uniform mat4 model;

void main() {
    // Pass the position and normal in world space to fragment shader
    fragPos = (model * vec4(vertexPosition, 1.0)).xyz;
    // Transform normal to world space with support for non-uniform scaling
    fragNormal = normalize(transpose(inverse(mat3(model))) * vertexNormal);

    gl_Position = MVP * vec4(vertexPosition, 1.0);
}
