#version 330 core
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;      // Added viewPos uniform for specular calculation
uniform float lightIntensity; // Added lightIntensity uniform

void main() {
    // Chess pattern calculation remains the same
    float chessSize = 20.0;
    vec3 colorA = vec3(0.5, 0.5, 0.5);
    vec3 colorB = vec3(0.1, 0.1, 0.1);
    vec2 patternCoord = TexCoords * chessSize;
    vec2 cell = floor(patternCoord);
    float pattern = mod(cell.x + cell.y, 2.0);
    vec3 baseColor = (pattern < 1.0) ? colorA : colorB;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = 0.4 * baseColor;
    vec3 diffuse = diff * baseColor * lightColor;
    vec3 specular = specularStrength * spec * lightColor;

    vec3 finalColor = ambient + (diffuse + specular) * lightIntensity;

    FragColor = vec4(finalColor, 1.0);
}