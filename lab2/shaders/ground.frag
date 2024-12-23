#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;
in vec3 Color;

out vec4 fragColor;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D reflectionMap;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

uniform float time;
uniform bool ground;
uniform bool isSolarPanel;

vec2 getTiledUV(vec2 uv, float tiles) {
    return fract(uv * tiles);
}

float getSunrayEffect(vec2 uv, float time) {
    float speed = 0.5;
    float rayPosition = fract(time * speed);
    float rayWidth = 0.1;
    float angle = 3.14159 / 4.0;
    vec2 rotatedUV = vec2(
    uv.x * cos(angle) - uv.y * sin(angle),
    uv.x * sin(angle) + uv.y * cos(angle)
    );
    float distToRay = abs(rotatedUV.x - rayPosition);
    float ray = smoothstep(rayWidth, 0.0, distToRay);
    ray *= sin(time * 2.0) * 0.5 + 0.5;
    return ray;
}

void main() {
    float tiles = isSolarPanel ? 1.0 : 4.0;
    vec2 tiledUV = getTiledUV(UV, tiles);

    float ambientStrength = 0.4;

    vec4 diffuseColor = texture(diffuseTexture, tiledUV);
    vec4 specularValue = texture(specularTexture, tiledUV);
    vec4 reflectionValue = texture(reflectionMap, tiledUV);

    float reflectionIntensity = isSolarPanel ? 0.5 : 0.5;

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * diffuseColor.rgb;

    vec3 finalColor = ( diffuse) * lightIntensity;



    if (isSolarPanel) {
        float sunray = getSunrayEffect(UV, time);
        vec3 rayColor = vec3(1.0, 0.9, 0.7) * lightIntensity;
        finalColor += rayColor * sunray * 0.2;

        vec2 gridUV = fract(UV * tiles);
        float gridLine = step(0.99, gridUV.x) + step(0.99, gridUV.y);
        finalColor *= (1.0 - gridLine * 0.05);
        finalColor *= (1.0 + sunray * 0.3);

        finalColor = mix(finalColor, reflectionValue.rgb, reflectionIntensity);
    }

    fragColor = vec4(finalColor, 1.0);
}