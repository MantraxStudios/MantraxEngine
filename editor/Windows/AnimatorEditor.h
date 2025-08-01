#pragma once
#include "WindowBehaviour.h"
#include "render/Texture.h"
#include "components/SpriteAnimator.h"
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <memory>
#include <core/FileSystem.h>

// Forward declarations
class FileSystem;
class RenderPipeline;
class GameObject;

class AnimatorEditor : public WindowBehaviour {
public:
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Animator Editor";
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
    void LoadAnimators();
    void SaveAnimator(const std::string& animatorName);
    void LoadAnimator(const std::string& filePath);
    void CreateNewAnimator();
    void DeleteAnimator(int index);
    void ExportAnimatorToFile(const std::string& animatorName);
    void ImportAnimatorFromFile();
    
    // UI Helper methods
    void RenderAnimatorList();
    void RenderAnimatorProperties();
    void RenderStateEditor();
    void RenderTextureList();
    void RenderAnimationControls();
    void RenderExportImportSection();

    // Animator management
    void SelectAnimator(int index);
    void AddStateToCurrentAnimator();
    void RemoveStateFromCurrentAnimator(int stateIndex);
    void AddTextureToState(const std::string& stateName);
    void RemoveTextureFromState(const std::string& stateName, int textureIndex);
    void ShowFeedback(const std::string& message, float duration);
    
    // Drag & Drop functionality
    void RenderTextureDragDrop(const std::string& stateName);
    void HandleTextureDrop(const std::string& stateName, const std::string& texturePath);

    bool animatorsLoaded = false;
    std::vector<std::string> animatorNames;
    int selectedAnimator = 0;
    nlohmann::json animatorsConfigJson;
    std::string lastKey = "";

    // Current animator data
    std::shared_ptr<SpriteAnimator> currentAnimator;
    std::string currentAnimatorName = "";
    
    // Buffers para propiedades editables
    char animatorNameBuffer[64] = "";
    char stateNameBuffer[64] = "";
    char texturePathBuffer[256] = "";
    float animationSpeed = 1.0f;
    bool isPlaying = false;
    int currentFrame = 0;
    
    // UI state
    int selectedState = 0;
    int selectedTexture = 0;
    bool showTextureBrowser = false;
    
    // Feedback visual
    std::string feedbackMsg = "";
    float feedbackTime = 0.0f;
    
                // File paths
            std::string animatorsDirectory = FileSystem::getProjectPath() + "\\Content\\Animators\\";
            std::string animatorExtension = ".animator";
}; 