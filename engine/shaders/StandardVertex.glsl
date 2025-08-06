#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
// CORREGIDO: Usar locations 2-5 para instanced rendering 
layout (location = 2) in vec4 aInstanceMatrix_0;
layout (location = 3) in vec4 aInstanceMatrix_1;
layout (location = 4) in vec4 aInstanceMatrix_2;
layout (location = 5) in vec4 aInstanceMatrix_3;
layout (location = 6) in vec3 aNormal;     // Normal del modelo (opcional)
layout (location = 7) in vec3 aTangent;    // Tangente del modelo (opcional)  
layout (location = 8) in vec3 aBitangent;  // Bitangente del modelo (opcional)

uniform mat4 view;
uniform mat4 projection;
uniform bool uUseModelNormals; // Flag para usar normales del modelo o calcular del cubo

// Shadow mapping uniforms
uniform mat4 uLightSpaceMatrix;      // Para luz direccional
// Spot light shadow matrices (opcional)
uniform mat4 uSpotLightMatrices[2];

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;
out vec4 FragPosLightSpace; // Light space position for simple shadows
// Spot light positions (opcional)
out vec4 FragPosSpotLightSpace[2];

void main() {
    TexCoord = aTexCoord;
    
    // CORREGIDO: Reconstruir la matriz de instancia desde los 4 vec4
    mat4 aInstanceMatrix = mat4(aInstanceMatrix_0, aInstanceMatrix_1, aInstanceMatrix_2, aInstanceMatrix_3);
    
    // Transformar posici�n
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Calcular posici�n en espacio de luz
    FragPosLightSpace = uLightSpaceMatrix * worldPos;
    
    // Calcular posiciones para spot lights (opcional)
    for (int i = 0; i < 2; i++) {
        FragPosSpotLightSpace[i] = uSpotLightMatrices[i] * worldPos;
    }
    
    // Matriz normal para transformar normales correctamente
    mat3 normalMatrix = mat3(transpose(inverse(aInstanceMatrix)));
    
    vec3 normal, tangent, bitangent;
    
    if (uUseModelNormals) {
        // Usar normales del modelo 3D
        normal = normalize(normalMatrix * aNormal);
        tangent = normalize(normalMatrix * aTangent);
        bitangent = normalize(normalMatrix * aBitangent);
    } else {
        // Calcular normales del cubo basadas en la posici�n local
        normal = normalize(normalMatrix * normalize(aPos));
        tangent = normalize(normalMatrix * vec3(1.0, 0.0, 0.0));
        bitangent = normalize(normalMatrix * vec3(0.0, 1.0, 0.0));
    }
    
    Normal = normal;
    
    // Crear TBN matrix para normal mapping (Gram-Schmidt orthogonalization)
    vec3 T = normalize(tangent - dot(tangent, normal) * normal);
    vec3 B = normalize(cross(normal, T));
    
    TBN = mat3(T, B, normal);
    
    gl_Position = projection * view * worldPos;
}