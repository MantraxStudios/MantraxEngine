#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 WorldPos;

void main() {
    // Visualizar normales como colores (RGB = XYZ)
    vec3 normalColor = normalize(Normal) * 0.5 + 0.5;
    
    // Agregar un poco de iluminación básica para mejor visibilidad
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.3, 0.3, 0.3);
    
    vec3 finalColor = normalColor + diffuse;
    
    FragColor = vec4(finalColor, 1.0);
}
