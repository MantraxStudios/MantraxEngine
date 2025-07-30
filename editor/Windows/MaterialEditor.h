#pragma once
#include "WindowBehaviour.h"
#include "render/Texture.h"
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

enum class MaterialType { Simple, PBR };

// Forward declarations
class FileSystem;
class RenderPipeline;

class MaterialEditor : public WindowBehaviour {
public:
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Material Editor";
        return name;
    }

    static ImTextureID GetTexturePreview(const char* path) {
        static std::map<std::string, std::shared_ptr<Texture>> previewTextures;
        if (!path || path[0] == 0) return 0;
        std::string sPath(path);
        auto it = previewTextures.find(sPath);
        if (it != previewTextures.end()) {
            return (ImTextureID)(uintptr_t)it->second->getID();
        }
        else {
            auto tex = std::make_shared<Texture>();
            if (tex->loadFromFile(sPath)) {
                previewTextures[sPath] = tex;
                return (ImTextureID)(uintptr_t)tex->getID();
            }
        }
        return 0;
    }

private:
    void LoadMaterials();
    void SaveMaterials();
    void CreateMaterial(MaterialType type);
    void DeleteMaterial(int index);

    bool materialesCargados = false;
    std::vector<std::string> materialNames;
    int selectedMaterial = 0;
    nlohmann::json materialsConfigJson;
    std::string lastKey = "";

    // Buffers para propiedades editables
    char nameBuffer[64] = "";
    float albedo[3] = { 1.0f, 1.0f, 1.0f };
    float metallic = 0.0f;
    float roughness = 1.0f;
    float emissive[3] = { 0.0f, 0.0f, 0.0f };
    float tiling[2] = { 1.0f, 1.0f };
    float normalStrength = 1.0f;

    // Buffers para texturas (asumiendo nombres de archivo)
    char albedoTexture[128] = "";
    char normalTexture[128] = "";
    char metallicTexture[128] = "";
    char roughnessTexture[128] = "";
    char emissiveTexture[128] = "";
    char aoTexture[128] = "";

    ImTextureID previewAlbedo = 0;
    ImTextureID previewNormal = 0;
    ImTextureID previewMetallic = 0;
    ImTextureID previewRoughness = 0;
    ImTextureID previewEmissive = 0;
    ImTextureID previewAO = 0;

    // Feedback visual
    std::string feedbackMsg = "";
    float feedbackTime = 0.0f;
};
