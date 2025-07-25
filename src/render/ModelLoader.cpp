#include "ModelLoader.h"
#include <iostream>

ModelLoader& ModelLoader::getInstance() {
    static ModelLoader instance;
    return instance;
}

std::shared_ptr<AssimpGeometry> ModelLoader::loadModel(const std::string& path) {
    // Verificar si ya está en cache
    auto it = modelCache.find(path);
    if (it != modelCache.end()) {
        std::cout << "Model loaded from cache: " << path << std::endl;
        return it->second;
    }
    
    // Cargar modelo nuevo
    std::cout << "Attempting to load model: " << path << std::endl;
    auto model = std::make_shared<AssimpGeometry>(path);
    
    if (model && model->isLoaded()) {
        // Guardar en cache solo si se cargó correctamente
        modelCache[path] = model;
        std::cout << "Model loaded and cached: " << path << std::endl;
        return model;
    } else {
        std::cerr << "Failed to load model: " << path << std::endl;
        std::cerr << "Make sure the file exists and is a valid 3D model format" << std::endl;
        return nullptr;
    }
}

std::shared_ptr<AssimpGeometry> ModelLoader::getModel(const std::string& path) {
    auto it = modelCache.find(path);
    if (it != modelCache.end()) {
        return it->second;
    }
    return nullptr;
}

void ModelLoader::clearCache() {
    modelCache.clear();
    std::cout << "Model cache cleared" << std::endl;
}

void ModelLoader::listLoadedModels() const {
    std::cout << "=== Loaded Models (" << modelCache.size() << ") ===" << std::endl;
    for (const auto& pair : modelCache) {
        const auto& model = pair.second;
        std::cout << "- " << pair.first 
                  << " (Vertices: " << model->getVertexCount() 
                  << ", Indices: " << model->getIndexCount() << ")" << std::endl;
    }
} 