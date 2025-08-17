#pragma once
#include "AssimpGeometry.h"
#include <memory>
#include <unordered_map>
#include <string>

class ModelLoader {
public:
    static ModelLoader& getInstance();
    
    // Cargar modelo (con cache)
    std::shared_ptr<AssimpGeometry> loadModel(const std::string& path);
    
    // Obtener modelo cargado
    std::shared_ptr<AssimpGeometry> getModel(const std::string& path);
    
    // Limpiar cache
    void clearCache();
    
    // Info del cache
    size_t getCacheSize() const { return modelCache.size(); }
    void listLoadedModels() const;

private:
    ModelLoader() = default;
    ~ModelLoader() = default;
    
    // Cache de modelos cargados
    std::unordered_map<std::string, std::shared_ptr<AssimpGeometry>> modelCache;
    
    // Prohibir copia
    ModelLoader(const ModelLoader&) = delete;
    ModelLoader& operator=(const ModelLoader&) = delete;
}; 