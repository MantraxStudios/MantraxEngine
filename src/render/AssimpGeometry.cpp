#include "AssimpGeometry.h"
#include <iostream>
#include <limits>
#include <glm/gtc/type_ptr.hpp>

AssimpGeometry::AssimpGeometry(const std::string& path) 
    : modelPath(path), loaded(false), EBO(0),
      boundingBoxMin(std::numeric_limits<float>::max()),
      boundingBoxMax(std::numeric_limits<float>::lowest()) {
    
    // Inicializar buffers
    vao = 0;
    vbo = 0;
    instanceVBO = 0;
    
    // Crear buffers para el modelo
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &instanceVBO);
    
    // Verificar que los buffers se crearon correctamente
    if (vao == 0 || vbo == 0 || EBO == 0 || instanceVBO == 0) {
        std::cerr << "ERROR: Failed to create OpenGL buffers" << std::endl;
        loaded = false;
        return;
    }
    
    std::cout << "OpenGL buffers created successfully (VAO: " << vao << ", VBO: " << vbo 
              << ", EBO: " << EBO << ", InstanceVBO: " << instanceVBO << ")" << std::endl;
    
    loadModel(path);
}

AssimpGeometry::~AssimpGeometry() {
    // Limpiar buffer específico de AssimpGeometry
    if (EBO) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    
    // Limpiar buffers base que creamos manualmente
    if (instanceVBO) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void AssimpGeometry::loadModel(const std::string& path) {
    Assimp::Importer importer;
    
    // Configurar flags de procesamiento para maximizar calidad
    unsigned int flags = aiProcess_Triangulate | 
                        aiProcess_FlipUVs | 
                        aiProcess_CalcTangentSpace |
                        aiProcess_GenSmoothNormals |
                        aiProcess_JoinIdenticalVertices |
                        aiProcess_ImproveCacheLocality |
                        aiProcess_OptimizeMeshes;
    
    std::cout << "Loading model from: " << path << std::endl;
    
    const aiScene* scene = importer.ReadFile(path, flags);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        std::cerr << "Failed to load file: " << path << std::endl;
        loaded = false;
        return;
    }
    
    std::cout << "Assimp scene loaded, processing meshes..." << std::endl;
    
    // Procesar todos los nodos del modelo
    processNode(scene->mRootNode, scene);
    
    if (!vertices.empty() && !indices.empty()) {
        calculateBoundingBox();
        setupMesh();
        loaded = true;
        std::cout << "Model loaded successfully: " << path << std::endl;
        std::cout << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;
        std::cout << "Bounding box: Min(" << boundingBoxMin.x << "," << boundingBoxMin.y << "," << boundingBoxMin.z 
                  << ") Max(" << boundingBoxMax.x << "," << boundingBoxMax.y << "," << boundingBoxMax.z << ")" << std::endl;
    } else {
        std::cerr << "ERROR: No valid geometry loaded from model: " << path << std::endl;
        std::cerr << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;
        loaded = false;
    }
}

void AssimpGeometry::processNode(aiNode* node, const aiScene* scene) {
    // Procesar todos los meshes del nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }
    
    // Procesar recursivamente todos los nodos hijos
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

void AssimpGeometry::processMesh(aiMesh* mesh, const aiScene* scene) {
    unsigned int vertexOffset = vertices.size();
    
    // Procesar vértices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        
        // Posición
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;
        
        // Normales
        if (mesh->mNormals) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        
        // Coordenadas de textura (solo el primer conjunto)
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoords = glm::vec2(0.0f);
        }
        
        // Tangentes
        if (mesh->mTangents) {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        } else {
            vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        
        // Bitangentes
        if (mesh->mBitangents) {
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        } else {
            vertex.bitangent = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        
        vertices.push_back(vertex);
    }
    
    // Procesar índices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(vertexOffset + face.mIndices[j]);
        }
    }
}

void AssimpGeometry::calculateBoundingBox() {
    if (vertices.empty()) return;
    
    boundingBoxMin = vertices[0].position;
    boundingBoxMax = vertices[0].position;
    
    for (const auto& vertex : vertices) {
        boundingBoxMin = glm::min(boundingBoxMin, vertex.position);
        boundingBoxMax = glm::max(boundingBoxMax, vertex.position);
    }
}

void AssimpGeometry::setupMesh() {
    glBindVertexArray(vao);
    
    // Buffer de vértices
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Buffer de índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Configurar atributos de vértices según el shader existente
    
    // location 0: position (vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    
    // location 1: texCoords (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(1);
    
    // location 6: normal (vec3)
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(6);
    
    // location 7: tangent (vec3)
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(7);
    
    // location 8: bitangent (vec3)
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(8);
    
    // Buffer de instancias (matrices de modelo) - locations 2-5
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

void AssimpGeometry::draw() const {
    if (!loaded || indices.empty() || vao == 0) {
        std::cerr << "WARNING: Attempting to draw invalid AssimpGeometry (loaded: " << loaded 
                  << ", indices: " << indices.size() << ", vao: " << vao << ")" << std::endl;
        return;
    }
    
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
}

void AssimpGeometry::drawInstanced(const std::vector<glm::mat4>& modelMatrices) const {
    if (!loaded || indices.empty() || modelMatrices.empty() || vao == 0) {
        if (!loaded || vao == 0) {
            std::cerr << "WARNING: Attempting to draw invalid AssimpGeometry in instanced mode (loaded: " << loaded 
                      << ", vao: " << vao << ")" << std::endl;
        }
        return;
    }
    
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), 
                           GL_UNSIGNED_INT, 0, static_cast<GLsizei>(modelMatrices.size()));
} 