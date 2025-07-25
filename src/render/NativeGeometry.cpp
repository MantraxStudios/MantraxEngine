#include "NativeGeometry.h"
#include <glm/gtc/type_ptr.hpp>

NativeGeometry::NativeGeometry() {
    // Vértices con posiciones y coordenadas UV
    float vertices[] = {
        // Posiciones (x, y, z)     // UVs (u, v)
        // Cara frontal
        -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,       1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,       0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,       0.0f, 0.0f,
        
        // Cara trasera
        -0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,       1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,       1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,       1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,       0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
        
        // Cara izquierda
        -0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
        
        // Cara derecha
         0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
        
        // Cara inferior
        -0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,       1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,       1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,       1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,       0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,       0.0f, 1.0f,
        
        // Cara superior
        -0.5f,  0.5f, -0.5f,       0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,       1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,       1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,       0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,       0.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &instanceVBO);
    
    glBindVertexArray(vao);
    
    // Buffer de vértices (posición + UV)
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Atributo de posición (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Atributo de UV (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Para cubos no usamos normales/tangentes reales (se calculan en shader)
    // pero necesitamos deshabilitar los atributos para evitar errores
    glDisableVertexAttribArray(6); // Normal
    glDisableVertexAttribArray(7); // Tangent  
    glDisableVertexAttribArray(8); // Bitangent
    
    // Buffer de instancias (matrices de modelo)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    
    // Configurar atributos para la matriz de modelo (4 vec4) - locations 2-5
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    
    // Configurar divisor de instancias
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    
    glBindVertexArray(0);
}

NativeGeometry::~NativeGeometry() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &instanceVBO);
}

void NativeGeometry::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void NativeGeometry::drawInstanced(const std::vector<glm::mat4>& modelMatrices) const {
    if (modelMatrices.empty()) return;
    
    glBindVertexArray(vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, static_cast<GLsizei>(modelMatrices.size()));
}

void NativeGeometry::updateInstanceBuffer(const std::vector<glm::mat4>& modelMatrices) {
    if (modelMatrices.empty()) return;
    
    currentInstances = modelMatrices;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), 
                 glm::value_ptr(modelMatrices[0]), GL_DYNAMIC_DRAW);
} 