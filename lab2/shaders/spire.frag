#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos; // Entrada de la posición del fragmento
in vec3 Normal;  // Entrada de las normales

uniform sampler2D diffuseMap;

void main() {
    // Color base del mapa de texturas
    vec4 baseColor = texture(diffuseMap, TexCoords);

    // Normalizada de las normales
    vec3 norm = normalize(Normal);

    // Simulación de una fuente de luz direccional
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3)); // Dirección de la luz
    float diff = max(dot(norm, lightDir), 0.0); // Componente difusa

    // Simulación de un brillo especular metálico
    vec3 viewDir = normalize(-FragPos); // Asume la cámara en el origen
    vec3 reflectDir = reflect(-lightDir, norm); // Dirección de reflexión
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0); // Componente especular

    // Color gris metálico base
    vec3 metallicGray = vec3(0.6, 0.6, 0.6);

    // Mezcla del efecto de iluminación con el gris metálico
    vec3 metallicColor = mix(metallicGray, diff * baseColor.rgb + spec * vec3(1.0), diff + spec);

    FragColor = vec4(metallicColor, baseColor.a);
}
