#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "Texture.h"
#include "../core/CoreExporter.h"


class MANTRAXCORE_API Material {
public:
    Material();
    Material(const std::string& name);
    ~Material();

    // Validación del material
    bool isValid() const { return !name.empty(); }

    // Propiedades del material
    void setName(const std::string& materialName);
    void setAlbedo(const glm::vec3& color);
    void setMetallic(float value);
    void setRoughness(float value);
    void setEmissive(const glm::vec3& color);
    void setTiling(const glm::vec2& tiling);
    void setNormalStrength(float strength);

    // Texturas PBR (todas opcionales) - CONFIGURACIÓN AUTOMÁTICA
    void setAlbedoTexture(const std::string& filePath);
    void setNormalTexture(const std::string& filePath);
    void setMetallicTexture(const std::string& filePath);
    void setRoughnessTexture(const std::string& filePath);
    void setEmissiveTexture(const std::string& filePath);
    void setAOTexture(const std::string& filePath);
    
    // Métodos para establecer texturas directamente desde objetos Texture
    void setAlbedoTexture(std::shared_ptr<Texture> texture);
    void setNormalTexture(std::shared_ptr<Texture> texture);
    void setMetallicTexture(std::shared_ptr<Texture> texture);
    void setRoughnessTexture(std::shared_ptr<Texture> texture);
    void setEmissiveTexture(std::shared_ptr<Texture> texture);
    void setAOTexture(std::shared_ptr<Texture> texture);
    
    // Configuración automática basada en las texturas cargadas
    void autoConfigureMaterial();

    // Getters
    glm::vec3 getAlbedo() const { return albedo; }
    float getMetallic() const { return metallic; }
    float getRoughness() const { return roughness; }
    glm::vec3 getEmissive() const { return emissive; }
    glm::vec2 getTiling() const { return tiling; }
    float getNormalStrength() const { return normalStrength; }
    
    std::shared_ptr<Texture> getAlbedoTexture() const { return albedoTexture; }
    std::shared_ptr<Texture> getNormalTexture() const { return normalTexture; }
    std::shared_ptr<Texture> getMetallicTexture() const { return metallicTexture; }
    std::shared_ptr<Texture> getRoughnessTexture() const { return roughnessTexture; }
    std::shared_ptr<Texture> getEmissiveTexture() const { return emissiveTexture; }
    std::shared_ptr<Texture> getAOTexture() const { return aoTexture; }

    // Bind/Unbind texturas
    void bindTextures() const;
    void unbindTextures() const;

    // Nombre del material
    std::string getName() const { return name; }

    // Flags para verificar si tiene texturas
    bool hasAlbedoTexture() const { return albedoTexture != nullptr && albedoTexture->getID() != 0; }
    bool hasNormalTexture() const { return normalTexture != nullptr && normalTexture->getID() != 0; }
    bool hasMetallicTexture() const { return metallicTexture != nullptr && metallicTexture->getID() != 0; }
    bool hasRoughnessTexture() const { return roughnessTexture != nullptr && roughnessTexture->getID() != 0; }
    bool hasEmissiveTexture() const { return emissiveTexture != nullptr && emissiveTexture->getID() != 0; }
    bool hasAOTexture() const { return aoTexture != nullptr && aoTexture->getID() != 0; }

    // Check if material has any valid textures
    bool hasAnyValidTextures() const;

    // Debug method to diagnose texture issues
    void debugTextureState() const;

private:
    std::string name;
    
    // Propiedades del material
    glm::vec3 albedo;
    float metallic;
    float roughness;
    glm::vec3 emissive;
    glm::vec2 tiling;
    float normalStrength;
    
    // Texturas PBR (todas opcionales)
    std::shared_ptr<Texture> albedoTexture;
    std::shared_ptr<Texture> normalTexture;
    std::shared_ptr<Texture> metallicTexture;
    std::shared_ptr<Texture> roughnessTexture;
    std::shared_ptr<Texture> emissiveTexture;
    std::shared_ptr<Texture> aoTexture;
    
    // Métodos privados para detección automática
    std::string detectMaterialType() const;
    void applyAutoConfiguration(const std::string& materialType);
}; 