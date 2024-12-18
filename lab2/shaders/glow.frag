#version 330 core
out vec4 FragColor;

in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragPos;

uniform sampler2D diffuseTexture;
uniform vec3 viewPos;

uniform vec3 diffuseColor;
uniform float opacity;

void main() {
    vec3 normal = normalize(fragNormal);

    vec3 viewDir = normalize(viewPos - fragPos);

    float outline = abs(dot(normal, viewDir));
    outline = smoothstep(0.0, 0.3, 1.0 - outline);

    vec4 texColor = texture(diffuseTexture, fragUV);

    vec3 finalColor;
    if (outline < 0.5) {
        finalColor = vec3(0.0, 0.0, 0.0);
    } else {
        finalColor = texColor.rgb * diffuseColor;
    }

    FragColor = vec4(finalColor, opacity);
}