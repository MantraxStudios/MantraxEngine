#include "MaterialManager.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

MaterialManager& MaterialManager::getInstance() {
    static MaterialManager instance;
    return instance;
}

bool MaterialManager::loadMaterialsFromConfig(const std::string& configPath) {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open materials config file: " << configPath << std::endl;
            return false;
        }

        json config;
        file >> config;

        if (!config.contains("materials")) {
            std::cerr << "Error: Invalid materials config file - missing 'materials' section" << std::endl;
            return false;
        }

        json& materialsJson = config["materials"];
        
        for (auto it = materialsJson.begin(); it != materialsJson.end(); ++it) {
            const std::string& materialKey = it.key();
            const json& materialData = it.value();

            // Create new material
            auto material = std::make_shared<Material>();
            
            // Set basic properties
            if (materialData.contains("name")) {
                material->setName(materialData["name"]);
            }
            
            if (materialData.contains("albedo")) {
                auto albedo = materialData["albedo"];
                glm::vec3 albedoColor(albedo[0], albedo[1], albedo[2]);
                material->setAlbedo(albedoColor);
            }
            
            if (materialData.contains("metallic")) {
                material->setMetallic(materialData["metallic"]);
            }
            
            if (materialData.contains("roughness")) {
                material->setRoughness(materialData["roughness"]);
            }
            
            if (materialData.contains("emissive")) {
                auto emissive = materialData["emissive"];
                glm::vec3 emissiveColor(emissive[0], emissive[1], emissive[2]);
                material->setEmissive(emissiveColor);
            }
            
            if (materialData.contains("tiling")) {
                auto tiling = materialData["tiling"];
                glm::vec2 tilingValue(tiling[0], tiling[1]);
                material->setTiling(tilingValue);
            }
            
            if (materialData.contains("normal_strength")) {
                material->setNormalStrength(materialData["normal_strength"]);
            }

            // Load textures if present
            if (materialData.contains("textures")) {
                const json& textures = materialData["textures"];
                
                if (textures.contains("albedo")) {
                    material->setAlbedoTexture(textures["albedo"]);
                }
                
                if (textures.contains("normal")) {
                    material->setNormalTexture(textures["normal"]);
                }
                
                if (textures.contains("metallic")) {
                    material->setMetallicTexture(textures["metallic"]);
                }
                
                if (textures.contains("roughness")) {
                    material->setRoughnessTexture(textures["roughness"]);
                }
                
                if (textures.contains("emissive")) {
                    material->setEmissiveTexture(textures["emissive"]);
                }
                
                if (textures.contains("ao")) {
                    material->setAOTexture(textures["ao"]);
                }
            }

            // Store material with its key
            materials[materialKey] = material;
            std::cout << "Loaded material: " << materialKey << " -> " << material->getName() << std::endl;
        }

        std::cout << "Successfully loaded " << materials.size() << " materials from config" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading materials config: " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<Material> MaterialManager::getMaterial(const std::string& materialName) {
    auto it = materials.find(materialName);
    if (it != materials.end()) {
        return it->second;
    }
    
    std::cerr << "Warning: Material '" << materialName << "' not found. Returning nullptr." << std::endl;
    return nullptr;
}

bool MaterialManager::hasMaterial(const std::string& materialName) const {
    return materials.find(materialName) != materials.end();
}

void MaterialManager::listMaterials() const {
    std::cout << "\n=== Available Materials ===" << std::endl;
    for (const auto& pair : materials) {
        std::cout << "- " << pair.first << " -> " << pair.second->getName() << std::endl;
    }
    std::cout << "==========================\n" << std::endl;
}

void MaterialManager::clearMaterials() {
    materials.clear();
    std::cout << "All materials cleared" << std::endl;
}

size_t MaterialManager::getMaterialCount() const {
    return materials.size();
} 