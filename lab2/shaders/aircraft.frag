#version 330 core
in vec2 UV;
in vec3 Normal;
in vec3 Position;

out vec4 color;

uniform sampler2D diffuseTexture;
uniform float opacity;
uniform bool enableContour;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(diffuseTexture, UV);

    if (enableContour) {
        vec3 viewDir = normalize(viewPos - Position);
        vec3 normal = normalize(Normal);

        float outline = 1.0 - abs(dot(normal, viewDir));
        outline = smoothstep(0.0, 0.5, outline);

        if (outline < 0.3) {
            color = vec4(0.0, 0.0, 0.0, opacity);
        } else {
            color = texColor * vec4(1.0, 1.0, 1.0, opacity);
        }
    } else {
        color = texColor * vec4(1.0, 1.0, 1.0, opacity);
    }
}