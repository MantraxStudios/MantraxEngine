#include "Material.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <locale>

Material::Material()
    : name("Default Material"), albedo(1.0f), metallic(0.0f), roughness(0.5f), 
      emissive(0.0f), tiling(0.3f), normalStrength(1.0f) {
}

Material::Material(const std::string& materialName)
    : name(materialName), albedo(1.0f), metallic(0.0f), roughness(0.5f), 
      emissive(0.0f), tiling(0.2f), normalStrength(1.0f) {
}

Material::~Material() {
}

void Material::setName(const std::string& materialName) {
    name = materialName;
}

void Material::setAlbedo(const glm::vec3& color) {
    albedo = color;
}

void Material::setMetallic(float value) {
    metallic = std::clamp(value, 0.0f, 1.0f);
}

void Material::setRoughness(float value) {
    roughness = std::clamp(value, 0.0f, 1.0f);
}

void Material::setEmissive(const glm::vec3& color) {
    emissive = color;
}

void Material::setTiling(const glm::vec2& tilingValue) {
    tiling = tilingValue;
}

void Material::setNormalStrength(float strength) {
    normalStrength = std::clamp(strength, 0.0f, 1.0f); // Permitir mayor intensidad para texturas detalladas
}

void Material::setAlbedoTexture(const std::string& filePath) {
    albedoTexture = std::make_shared<Texture>(filePath);
    if (!albedoTexture->getID()) {
        std::cerr << "Error al cargar textura de albedo: " << filePath << std::endl;
        albedoTexture = nullptr;
    } else {
        std::cout << "Albedo texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial(); // Configuración automática
    }
}

void Material::setNormalTexture(const std::string& filePath) {
    normalTexture = std::make_shared<Texture>(filePath);
    if (!normalTexture->getID()) {
        std::cerr << "Error al cargar textura normal: " << filePath << std::endl;
        normalTexture = nullptr;
    } else {
        std::cout << "Normal texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial(); // Configuración automática
    }
}

void Material::setMetallicTexture(const std::string& filePath) {
    metallicTexture = std::make_shared<Texture>(filePath);
    if (!metallicTexture->getID()) {
        std::cerr << "Error al cargar textura metallic: " << filePath << std::endl;
        metallicTexture = nullptr;
    } else {
        std::cout << "Metallic texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial(); // Configuración automática
    }
}

void Material::setRoughnessTexture(const std::string& filePath) {
    roughnessTexture = std::make_shared<Texture>(filePath);
    if (!roughnessTexture->getID()) {
        std::cerr << "Error al cargar textura roughness: " << filePath << std::endl;
        roughnessTexture = nullptr;
    } else {
        std::cout << "Roughness texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial(); // Configuración automática
    }
}

void Material::setEmissiveTexture(const std::string& filePath) {
    emissiveTexture = std::make_shared<Texture>(filePath);
    if (!emissiveTexture->getID()) {
        std::cerr << "Error al cargar textura emissive: " << filePath << std::endl;
        emissiveTexture = nullptr;
    } else {
        std::cout << "Emissive texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial();
    }
}

void Material::setAOTexture(const std::string& filePath) {
    aoTexture = std::make_shared<Texture>(filePath);
    if (!aoTexture->getID()) {
        std::cerr << "Error al cargar textura AO: " << filePath << std::endl;
        aoTexture = nullptr;
    } else {
        std::cout << "AO texture cargada para material '" << name << "': " << filePath << std::endl;
        autoConfigureMaterial(); 
    }
}

void Material::setAlbedoTexture(std::shared_ptr<Texture> texture) {
    albedoTexture = texture;
    if (albedoTexture && albedoTexture->getID()) {
        autoConfigureMaterial();
    }
}

void Material::setNormalTexture(std::shared_ptr<Texture> texture) {
    normalTexture = texture;
    if (normalTexture && normalTexture->getID()) {
        std::cout << "Normal texture establecida directamente para material '" << name << "'" << std::endl;
        autoConfigureMaterial();
    }
}

void Material::setMetallicTexture(std::shared_ptr<Texture> texture) {
    metallicTexture = texture;
    if (metallicTexture && metallicTexture->getID()) {
        std::cout << "Metallic texture establecida directamente para material '" << name << "'" << std::endl;
        autoConfigureMaterial();
    }
}

void Material::setRoughnessTexture(std::shared_ptr<Texture> texture) {
    roughnessTexture = texture;
    if (roughnessTexture && roughnessTexture->getID()) {
        std::cout << "Roughness texture establecida directamente para material '" << name << "'" << std::endl;
        autoConfigureMaterial();
    }
}

void Material::setEmissiveTexture(std::shared_ptr<Texture> texture) {
    emissiveTexture = texture;
    if (emissiveTexture && emissiveTexture->getID()) {
        std::cout << "Emissive texture establecida directamente para material '" << name << "'" << std::endl;
        autoConfigureMaterial();
    }
}

void Material::setAOTexture(std::shared_ptr<Texture> texture) {
    aoTexture = texture;
    if (aoTexture && aoTexture->getID()) {
        std::cout << "AO texture establecida directamente para material '" << name << "'" << std::endl;
        autoConfigureMaterial();
    }
}

// Configuración automática basada en las texturas cargadas
void Material::autoConfigureMaterial() {
    std::string materialType = detectMaterialType();
    applyAutoConfiguration(materialType);
}

// Detecta el tipo de material basándose en las texturas y nombres
std::string Material::detectMaterialType() const {
    // Buscar patrones en los nombres de texturas para detectar tipo
    std::vector<std::string> texturePaths;
    
    if (albedoTexture && albedoTexture->getID() != 0) {
        texturePaths.push_back(albedoTexture->getFilePath());
    }
    if (normalTexture && normalTexture->getID() != 0) {
        texturePaths.push_back(normalTexture->getFilePath());
    }
    if (metallicTexture && metallicTexture->getID() != 0) {
        texturePaths.push_back(metallicTexture->getFilePath());
    }
    if (roughnessTexture && roughnessTexture->getID() != 0) {
        texturePaths.push_back(roughnessTexture->getFilePath());
    }
    
    // Análisis de patrones en los nombres de archivo
    for (size_t i = 0; i < texturePaths.size(); ++i) {
        std::string path = texturePaths[i];
        std::string lowerPath = path;
        for (size_t j = 0; j < lowerPath.length(); ++j) {
            lowerPath[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowerPath[j])));
        }
        
        // Diamond Plate patterns
        if (lowerPath.find("diamond") != std::string::npos || 
            lowerPath.find("plate") != std::string::npos) {
            return "DiamondPlate";
        }
        
        // Metal patterns
        if (lowerPath.find("metal") != std::string::npos || 
            lowerPath.find("steel") != std::string::npos ||
            lowerPath.find("iron") != std::string::npos ||
            lowerPath.find("aluminum") != std::string::npos) {
            return "Metal";
        }
        
        // Fabric/Cloth patterns
        if (lowerPath.find("fabric") != std::string::npos || 
            lowerPath.find("cloth") != std::string::npos ||
            lowerPath.find("leather") != std::string::npos) {
            return "Fabric";
        }
        
        // Wood patterns
        if (lowerPath.find("wood") != std::string::npos || 
            lowerPath.find("timber") != std::string::npos ||
            lowerPath.find("plank") != std::string::npos) {
            return "Wood";
        }
        
        // Stone/Concrete patterns
        if (lowerPath.find("stone") != std::string::npos || 
            lowerPath.find("concrete") != std::string::npos ||
            lowerPath.find("brick") != std::string::npos ||
            lowerPath.find("rock") != std::string::npos) {
            return "Stone";
        }
        
        // Plastic patterns
        if (lowerPath.find("plastic") != std::string::npos) {
            return "Plastic";
        }
    }
    
    // Análisis basado en las texturas que tiene
    bool hasMetallic = hasMetallicTexture();
    bool hasNormal = hasNormalTexture();
    bool hasRoughness = hasRoughnessTexture();
    
    if (hasMetallic && hasNormal && hasRoughness) {
        return "FullPBR"; // Material PBR completo
    } else if (hasMetallic) {
        return "Metal";
    } else if (hasNormal && hasRoughness) {
        return "Rough";
    } else {
        return "Basic";
    }
}

// Aplica la configuración automática basada en el tipo detectado
void Material::applyAutoConfiguration(const std::string& materialType) {
    if (materialType == "DiamondPlate") {
        // Configuración optimizada para Diamond Plate
        setAlbedo(glm::vec3(0.8f, 0.8f, 0.85f));
        setMetallic(0.9f);
        setRoughness(0.3f);
        setTiling(glm::vec2(2.0f, 2.0f));
        setNormalStrength(3.5f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Metal") {
        // Configuración para metal genérico
        setAlbedo(glm::vec3(0.7f, 0.7f, 0.7f));
        setMetallic(1.0f);
        setRoughness(0.2f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(1.5f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Fabric") {
        // Configuración para telas
        setAlbedo(glm::vec3(0.8f, 0.8f, 0.8f));
        setMetallic(0.0f);
        setRoughness(0.8f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(1.0f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Wood") {
        // Configuración para madera
        setAlbedo(glm::vec3(0.6f, 0.4f, 0.2f));
        setMetallic(0.0f);
        setRoughness(0.7f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(1.2f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Stone") {
        // Configuración para piedra/concreto
        setAlbedo(glm::vec3(0.5f, 0.5f, 0.5f));
        setMetallic(0.0f);
        setRoughness(0.9f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(2.0f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Plastic") {
        // Configuración para plástico
        setAlbedo(glm::vec3(0.9f, 0.9f, 0.9f));
        setMetallic(0.0f);
        setRoughness(0.3f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(0.5f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "FullPBR") {
        // Material PBR completo - configuración balanceada
        setAlbedo(glm::vec3(0.8f, 0.8f, 0.8f));
        setMetallic(0.5f);
        setRoughness(0.5f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(2.0f);
        setEmissive(glm::vec3(0.0f));
        
    } else if (materialType == "Rough") {
        // Material rugoso
        setAlbedo(glm::vec3(0.6f, 0.6f, 0.6f));
        setMetallic(0.1f);
        setRoughness(0.9f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(2.0f);
        setEmissive(glm::vec3(0.0f));
        
    } else { // Basic
        // Configuración básica por defecto
        setAlbedo(glm::vec3(0.8f, 0.8f, 0.8f));
        setMetallic(0.0f);
        setRoughness(0.5f);
        setTiling(glm::vec2(1.0f, 1.0f));
        setNormalStrength(1.0f);
        setEmissive(glm::vec3(0.0f));
    }
}

void Material::bindTextures() const {
    // Bind albedo texture (slot 0)
    if (albedoTexture) {
        albedoTexture->bind(0);
    }
    
    // Bind normal texture (slot 1)
    if (normalTexture) {
        normalTexture->bind(1);
    }
    
    // Bind metallic texture (slot 2)
    if (metallicTexture) {
        metallicTexture->bind(2);
    }
    
    // Bind roughness texture (slot 3)
    if (roughnessTexture) {
        roughnessTexture->bind(3);
    }
    
    // Bind emissive texture (slot 4)
    if (emissiveTexture) {
        emissiveTexture->bind(4);
    }
    
    // Bind AO texture (slot 5)
    if (aoTexture) {
        aoTexture->bind(5);
    }
}

void Material::unbindTextures() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, 0);
} 