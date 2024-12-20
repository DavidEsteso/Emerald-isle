in vec3 worldPosition;
in vec3 worldNormal;
in vec3 viewPosition;
flat in int isShoulderOut;

uniform samplerCube cubemapSampler;
uniform float refractionIndex = 1.52;

out vec4 color;

void main() {
    vec3 normal = normalize(worldNormal);
    vec3 viewDir = normalize(viewPosition);

    vec3 reflectDir = reflect(-viewDir, normal);

    vec3 refractDir = refract(-viewDir, normal, 1.0/refractionIndex);

    vec4 reflectColor = texture(cubemapSampler, reflectDir);
    vec4 refractColor = texture(cubemapSampler, refractDir);

    float fresnelBias = 0.0;
    float fresnelScale = 1.0;
    float fresnelPower = 2.0;

    float fresnel = fresnelBias + fresnelScale * pow(1.0 + dot(viewDir, normal), fresnelPower);

    color = mix(refractColor, reflectColor, fresnel);
}