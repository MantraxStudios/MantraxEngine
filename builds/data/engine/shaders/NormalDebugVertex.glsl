#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec4 aInstanceMatrix_0;
layout (location = 3) in vec4 aInstanceMatrix_1;
layout (location = 4) in vec4 aInstanceMatrix_2;
layout (location = 5) in vec4 aInstanceMatrix_3;
layout (location = 6) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform bool uUseModelNormals;
uniform bool uFlipNormals;

out vec3 Normal;
out vec3 WorldPos;

void main() {
    // Reconstruir la matriz de instancia
    mat4 aInstanceMatrix = mat4(aInstanceMatrix_0, aInstanceMatrix_1, aInstanceMatrix_2, aInstanceMatrix_3);
    
    // Transformar posición
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    WorldPos = worldPos.xyz;
    
    // Calcular matriz normal
    mat3 normalMatrix = mat3(transpose(inverse(aInstanceMatrix)));
    
    vec3 normal;
    if (uUseModelNormals) {
        normal = normalize(normalMatrix * aNormal);
    } else {
        normal = normalize(normalMatrix * normalize(aPos));
    }
    
    // Aplicar inversión si se solicita
    if (uFlipNormals) {
        normal = -normal;
    }
    
    Normal = normal;
    
    gl_Position = projection * view * worldPos;
}
