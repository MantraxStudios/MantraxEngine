#pragma once
#include <iostream> 
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
        std::cout << "Scene: cleanup - start" << std::endl;

        std::cout << "Scene: cleaning gameObjects..." << std::endl;
        gameObjects.clear();

        std::cout << "renderPipeline ptr: " << renderPipeline << std::endl;
        if (renderPipeline) {
            std::cout << "renderPipeline is not nullptr" << std::endl;
            renderPipeline->clearLights();
        }
        else {
            std::cout << "renderPipeline IS nullptr!" << std::endl;
        }


        std::cout << "Scene: clearing lights..." << std::endl;
        lights.clear();

        std::cout << "Scene: clearing camera..." << std::endl;
        camera.reset();

        initialized = false;
        std::cout << "Scene: cleanup - end" << std::endl;
    }

    const std::string& getName() const { return name; }
    void setName(std::string newName) { name = newName; }
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