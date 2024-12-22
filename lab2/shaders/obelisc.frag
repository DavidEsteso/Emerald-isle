#version 330 core
in vec3 fragPos;
in vec3 fragNormal;
out vec4 fragmentColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

float random(vec3 st) {
    return fract(sin(dot(st.xyz, vec3(12.9898,78.233,37.719))) * 43758.5453123);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);
    float a = random(i);
    float b = random(i + vec3(1.0, 0.0, 0.0));
    float c = random(i + vec3(0.0, 1.0, 0.0));
    float d = random(i + vec3(1.0, 1.0, 0.0));
    float e = random(i + vec3(0.0, 0.0, 1.0));
    float f1 = random(i + vec3(1.0, 0.0, 1.0));
    float g = random(i + vec3(0.0, 1.0, 1.0));
    float h = random(i + vec3(1.0, 1.0, 1.0));
    return mix(mix(mix(a, b, u.x),
    mix(c, d, u.x), u.y),
    mix(mix(e, f1, u.x),
    mix(g, h, u.x), u.y), u.z);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 3.0;
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 pos = fragPos * 0.5;
    float n = fbm(pos);

    vec3 rockColor1 = vec3(0.5, 0.4, 0.3);
    vec3 rockColor2 = vec3(0.6, 0.6, 0.5);
    vec3 rockColor3 = vec3(0.3, 0.3, 0.3);

    vec3 finalColor = mix(rockColor1, rockColor2, n);
    finalColor = mix(finalColor, rockColor3, noise(pos * 2.0) * 0.5);

    float roughness = noise(pos * 10.0) * 0.1;
    finalColor += roughness;
    float variation = noise(pos * 4.0);
    finalColor *= (0.8 + variation * 0.4);
    float cracks = smoothstep(0.4, 0.5, noise(pos * 3.0));
    finalColor *= (1.0 - cracks * 0.3);

    vec3 ambient = finalColor * 0.3;
    vec3 diffuse = finalColor * lightColor * diff * lightIntensity;

    vec3 result =  diffuse;

    fragmentColor = vec4(result, 1.0);
}