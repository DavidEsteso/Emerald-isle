#version 330 core

in vec2 UV;
in vec2 worldPos;

uniform sampler2D grassTexture;
uniform float time;

out vec4 color;

vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

float smoothNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(
    mix(dot(hash2(i), f - vec2(0.0, 0.0)),
    dot(hash2(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
    mix(dot(hash2(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
    dot(hash2(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x),
    u.y);
}

vec2 distortUV(vec2 uv, vec2 worldPos) {
    float noiseX = smoothNoise(worldPos * 0.2 + vec2(time * 0.1, 0.0));
    float noiseY = smoothNoise(worldPos * 0.2 + vec2(0.0, time * 0.1));

    return uv + vec2(noiseX, noiseY) * 0.02;
}

void main() {
    vec2 distortedUV = distortUV(UV, worldPos);

    vec4 grassColor = texture(grassTexture, distortedUV);

    color = grassColor;
}
