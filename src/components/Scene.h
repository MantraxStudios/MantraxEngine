#pragma once
#include <string>
#include <memory>
#include <vector>
#include "GameObject.h"
#include "../render/Camera.h"
#include "../render/Light.h"
#include "../render/RenderPipeline.h"
#include "../core/CoreExporter.h"

// Forward declaration to avoid circular dependency
class RenderPipeline;

class MANTRAXCORE_API Scene {
public:
    Scene(const std::string& name = "New Scene");
    virtual ~Scene() = default;

    virtual void initialize() {}
    virtual void update(float deltaTime) {}
    void updateNative(float deltaTime);
    virtual void cleanup() {
        // Delete all game objects
        for (auto* obj : gameObjects) {
            delete obj;
        }
        gameObjects.clear();
        
        // Clear lights from RenderPipeline first
        if (renderPipeline) {
            renderPipeline->clearLights();
        }
        
        // Clear lights
        lights.clear();
        
        // Clear camera
        camera.reset();
        
        // Reset initialization flag
        initialized = false;
    }

    const std::string& getName() const { return name; }
    bool isInitialized() const { return initialized; }
    void setInitialized(bool value) { initialized = value; }

    void addGameObject(GameObject* object);
    
    // Agregar objeto sin sincronización automática (para casos especiales)
    void addGameObjectNoSync(GameObject* object);
    
    // RenderPipeline access
    void setRenderPipeline(RenderPipeline* pipeline) { renderPipeline = pipeline; }
    RenderPipeline* getRenderPipeline() const { return renderPipeline; }

    void addLight(std::shared_ptr<Light> light);

    void removeLight(std::shared_ptr<Light> light);

    const std::vector<GameObject*>& getGameObjects() const { return gameObjects; }
    const std::vector<std::shared_ptr<Light>>& getLights() const { return lights; }

    const Camera* getCamera() const { return camera.get(); }
    Camera* getCamera() { return camera.get(); }
    void setCamera(std::unique_ptr<Camera> newCamera) { camera = std::move(newCamera); }

protected:
    std::string name;
    std::vector<GameObject*> gameObjects;
    std::vector<std::shared_ptr<Light>> lights;
    std::unique_ptr<Camera> camera;
    RenderPipeline* renderPipeline = nullptr;
    bool initialized;
}; 