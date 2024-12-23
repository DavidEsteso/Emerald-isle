#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D diffuseMap;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

void main() {
    float ambientStrength = 0.4;

    vec3 baseColor =  vec3(0.6, 0.6, 0.6);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 metallicGray = vec3(0.6, 0.6, 0.6);

    vec3 ambient = ambientStrength * lightColor * baseColor.rgb;

    vec3 lighting = (ambient + diff * baseColor.rgb + spec * vec3(1.0)) * lightColor * lightIntensity;


    FragColor = vec4(lighting, 1.0);
}