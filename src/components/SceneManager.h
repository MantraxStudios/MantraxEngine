#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Scene.h"
#include "../render/RenderPipeline.h"
#include "../core/CoreExporter.h"
#include "../core/PhysicsManager.h"

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
    
    // Remove scene completely from the manager
    void removeScene(const std::string& sceneName);

    void update(float deltaTime);
    void setupRenderPipeline(RenderPipeline& pipeline);
    void setRenderPipeline(RenderPipeline* pipeline) { renderPipeline = pipeline; }
    void initializeAllScenes();

    // Physics system initialization
    bool initializePhysics();
    void cleanupPhysics();
    PhysicsManager& getPhysicsManager() { return PhysicsManager::getInstance(); }

    // Physics components cleanup for scene switching
    void cleanupPhysicsComponents(Scene* scene);
    
    // Reinitialize physics components for the active scene
    void reinitializePhysicsComponents();

private:
    SceneManager();
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* activeScene;
    RenderPipeline* renderPipeline = nullptr;
    bool physicsInitialized = false;
}; 