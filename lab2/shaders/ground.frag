#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;
in vec3 Color;

out vec4 fragColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 lightColor;

void main() {
    // Normalizar la normal y calcular dirección de luz
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);

    // Cálculo básico de luz difusa
    float diff = max(dot(normal, lightDir), 0.0);

    // Luz ambiental fija para ver algo siempre
    vec3 ambient = vec3(0.2);

    // Color final: ambient + diffuse
    vec3 finalColor = ambient + diff * lightColor * lightIntensity;

    // Para diagnóstico, mostremos diferentes componentes
    // Descomentar la línea que quieras verificar:

    // fragColor = vec4(1.0, 0.0, 0.0, 1.0);  // Rojo puro para ver si el shader funciona
     fragColor = vec4(normal * 0.5 + 0.5, 1.0);  // Visualizar normales
     fragColor = vec4(vec3(diff), 1.0);  // Ver solo la luz difusa
    // fragColor = vec4(lightColor, 1.0);  // Ver el color de la luz
    // fragColor = vec4(lightIntensity, 1.0);  // Ver la intensidad de la luz
    //fragColor = vec4(finalColor, 1.0);  // Ver el resultado final
}