in vec3 worldPosition;
in vec3 worldNormal;
in vec3 viewPosition;
flat in int isShoulderOut;

uniform samplerCube cubemapSampler;
uniform float refractionIndex = 1.52;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;

out vec4 color;

void main() {
    vec3 normal = normalize(worldNormal);
    vec3 viewDir = normalize(viewPosition);
    vec3 lightDir = normalize(lightPos - worldPosition);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * lightIntensity;


    vec3 reflectDir = reflect(-viewDir, normal);
    vec3 refractDir = refract(-viewDir, normal, 1.0/refractionIndex);

    vec4 reflectColor = texture(cubemapSampler, reflectDir);
    vec4 refractColor = texture(cubemapSampler, refractDir);

    float fresnelBias = 0.0;
    float fresnelScale = 1.0;
    float fresnelPower = 2.0;
    float fresnel = fresnelBias + fresnelScale * pow(1.0 + dot(viewDir, normal), fresnelPower);

    vec4 refractionReflection = mix(refractColor, reflectColor, fresnel);

    vec3 ambient = 0.3 * refractionReflection.rgb;

    color = vec4(refractionReflection.rgb * (diffuse + ambient), refractionReflection.a);
}