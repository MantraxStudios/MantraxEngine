#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Material.h"
#include "../core/CoreExporter.h"

class MANTRAXCORE_API MaterialManager {
public:
    static MaterialManager& getInstance();
    
    // Load materials from JSON configuration file
    bool loadMaterialsFromConfig(const std::string& configPath);
    
    // Get material by name
    std::shared_ptr<Material> getMaterial(const std::string& materialName);
    
    // Check if material exists
    bool hasMaterial(const std::string& materialName) const;
    
    // List all available materials
    void listMaterials() const;
    
    // Clear all materials
    void clearMaterials();
    
    // Get material count
    size_t getMaterialCount() const;
    
    // Get all materials
    const std::unordered_map<std::string, std::shared_ptr<Material>>& getAllMaterials() const;
    
    // Add material
    void addMaterial(const std::string& key, std::shared_ptr<Material> material);

private:
    MaterialManager() = default;
    ~MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;
    
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
}; 