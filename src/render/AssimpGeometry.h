#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class AssimpGeometry {
public:
    AssimpGeometry(const std::string& path);
    ~AssimpGeometry();

    void draw() const;
    void drawInstanced(const std::vector<glm::mat4>& modelMatrices) const;
    void updateInstanceBuffer(const std::vector<glm::mat4>& modelMatrices);
    
    // Para modelos 3D cargados
    bool usesModelNormals() const { return true; }
    glm::vec3 getBoundingBoxMin() const { return boundingBoxMin; }
    glm::vec3 getBoundingBoxMax() const { return boundingBoxMax; }
    
    // Info del modelo
    bool isLoaded() const { return loaded; }
    const std::string& getPath() const { return modelPath; }
    size_t getVertexCount() const { return vertices.size(); }
    size_t getIndexCount() const { return indices.size(); }

private:
    std::string modelPath;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // OpenGL buffers
    GLuint vao; // Vertex Array Object
    GLuint vbo; // Vertex Buffer Object
    GLuint EBO; // Element Buffer Object para índices
    GLuint instanceVBO; // Instance Buffer Object
    bool loaded;
    
    // Bounding box del modelo
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh, const aiScene* scene);
    void calculateBoundingBox();
    void setupMesh();
}; 