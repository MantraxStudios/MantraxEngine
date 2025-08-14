#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <GL/glew.h>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <render/Texture.h>
#include <render/Material.h>
#include <render/MaterialManager.h>
#include <render/RenderPipeline.h>
#include <components/GameObject.h>
#include <components/SceneManager.h>

// Estructura para almacenar datos de un tile
struct TileData {
    std::string name;
    std::string texturePath;
    std::shared_ptr<Material> material;
    
    TileData(const std::string& tileName, const std::string& path) 
        : name(tileName), texturePath(path) {
        material = std::make_shared<Material>(name);
        material->setAlbedoTexture(texturePath);
        material->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
        material->setAlpha(1.0f);
        material->setMetallic(0.0f);
        material->setRoughness(0.5f);
        material->setEmissive(glm::vec3(0.0f, 0.0f, 0.0f));
        material->setTiling(glm::vec2(1.0f, 1.0f));
    }
};

class TileEditor : public WindowBehaviour {
public:
    TileEditor();
    ~TileEditor();
    
    void OnRenderGUI() override;

    const std::string& getName() const override {
        static const std::string name = "Tile Editor";
        return name;
    }

    // Función helper para aplicar grid snap
    glm::vec3 applyGridSnap(const glm::vec3& position) const;

    std::shared_ptr<Material> tileMat;
    std::string tileName = "New Tile Texture";
    std::string texturePath;
    
    // Variables para tiles guardados
    std::vector<TileData> savedTiles;
    int selectedTileIndex = -1;
    std::string newTileName = "New Tile";
    std::string newTexturePath = "";
    
    // Variables para grid system
    float gridSize = 1.0f;
    bool gridSnapEnabled = true;
    
    // Cache de texturas para ImageButtons
    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    
    // Helper para obtener textura del cache
    GLuint getCachedTexture(const std::string& texturePath);
    
    // Cleanup method for texture cache
    void cleanupTextureCache();
    
    // Métodos para guardar y cargar tiles de/desde JSON
    nlohmann::json serializeTilesToJson() const;
    bool loadTilesFromJson(const nlohmann::json& tileDataArray);
    
    // Variables para el popup de preview
    bool showTilePreview = false;
    float previewTimer = 0.0f;
    inline static constexpr float PREVIEW_DURATION = 2.0f; // Duración en segundos
};
