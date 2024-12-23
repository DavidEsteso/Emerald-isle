#version 330 core


in vec3 worldNormal;
in vec3 viewDirection;
in vec3 worldPosition;

uniform samplerCube cubemapSampler;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

out vec4 fragColor;

void main() {
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(viewPos - worldPosition);

    vec3 L = normalize(lightPos - worldPosition);

    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 diffuse = lightColor * lightIntensity * NdotL;
    vec3 specular = lightColor * lightIntensity * pow(NdotH, 16.0);

    vec3 reflectedDir = reflect(-V, N);
    vec3 envColor = texture(cubemapSampler, reflectedDir).rgb;

    vec3 lighting = diffuse + specular;
    vec3 finalColor = mix(envColor, lighting, 0.5);

    fragColor = vec4(finalColor, 1.0);
}