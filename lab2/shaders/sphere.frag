#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float time;

void main()
{
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 norm = normalize(Normal);

    vec3 baseColor = vec3(1.0, 0.95, 0.8);
    float baseIntensity = 1.0 + 0.2 * sin(time * 3.0);
    vec3 baseGlow = baseColor * baseIntensity;

    float rim = 1.0 - max(dot(viewDir, norm), 0.0);
    rim = pow(rim, 2.0 + sin(time * 2.0));

    float corona = 0.3 * sin(time * 2.0 + length(FragPos) * 0.8)
    + 0.2 * cos(time * 1.5 + FragPos.y)
    + 0.7;

    vec3 rimColor = vec3(1.0, 0.9, 0.7);

    vec3 finalColor = baseGlow + (rim * rimColor * corona);

    FragColor = vec4(finalColor, 1.0);
}