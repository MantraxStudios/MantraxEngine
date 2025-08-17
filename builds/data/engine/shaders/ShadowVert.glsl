#version 330 core
layout (location = 0) in vec3 aPos;
// CORREGIDO: Usar locations 2-5 para instanced rendering como en tu AssimpGeometry
layout (location = 2) in vec4 aInstanceMatrix_0;
layout (location = 3) in vec4 aInstanceMatrix_1;
layout (location = 4) in vec4 aInstanceMatrix_2;
layout (location = 5) in vec4 aInstanceMatrix_3;

uniform mat4 uLightSpaceMatrix;

void main() {
    // Reconstruir la matriz de instancia desde los 4 vec4
    mat4 aInstanceMatrix = mat4(aInstanceMatrix_0, aInstanceMatrix_1, aInstanceMatrix_2, aInstanceMatrix_3);
    gl_Position = uLightSpaceMatrix * aInstanceMatrix * vec4(aPos, 1.0);
}