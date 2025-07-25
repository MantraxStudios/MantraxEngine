#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Scene.h"
#include "../render/RenderPipeline.h"
#include "../core/CoreExporter.h"

class MANTRAXCORE_API SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void addScene(std::unique_ptr<Scene> scene);
    void setActiveScene(const std::string& sceneName);
    Scene* getActiveScene() { return activeScene; }
    Scene* getScene(const std::string& sceneName);
    
    void update(float deltaTime);
    void setupRenderPipeline(RenderPipeline& pipeline);
    void setRenderPipeline(RenderPipeline* pipeline) { renderPipeline = pipeline; }
    void initializeAllScenes();

private:
    SceneManager() : activeScene(nullptr) {}
    ~SceneManager() = default;
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* activeScene;
    RenderPipeline* renderPipeline = nullptr;
}; 