#include "Scene.h"
#include "../render/RenderPipeline.h"
#include "../components/PhysicalObject.h"
#include "SceneManager.h"
#include <iostream>

Scene::Scene(const std::string& name) : name(name), initialized(false) {
}

void Scene::addGameObject(GameObject* object) {
    if (object) {
        gameObjects.push_back(object);
        
        // Sincronizar automáticamente con RenderPipeline si está disponible
        if (renderPipeline) {
            renderPipeline->AddGameObject(object);
        }
    }
}

void Scene::addGameObjectNoSync(GameObject* object) {
    if (object) {
        gameObjects.push_back(object);
    }
}

void Scene::updateNative(float deltaTime) {
    // Update all game objects
    for (auto* obj : gameObjects) {
        if (obj) {
            obj->update(deltaTime);
        }
    }
    
    // Initialize physics components if physics is available
    auto& sceneManager = SceneManager::getInstance();
    if (sceneManager.getPhysicsManager().getPhysics()) {
        for (auto* obj : gameObjects) {
            if (obj) {
                // Initialize any PhysicalObject components that haven't been initialized yet
                if (auto physicalObject = obj->getComponent<PhysicalObject>()) {
                    if (!physicalObject->isInitialized()) {
                        physicalObject->initializePhysics();
                    }
                }
            }
        }
    }
}

void Scene::addLight(std::shared_ptr<Light> light) {
    if (light) {
        lights.push_back(light);
        // Sincronizar con RenderPipeline
        if (renderPipeline) {
            renderPipeline->AddLight(light);
        }
    }
}


void Scene::removeLight(std::shared_ptr<Light> light) {
    if (light) {
        // Remover del RenderPipeline primero
        if (renderPipeline) {
            renderPipeline->RemoveLight(light);
        }
        // Remover de la lista de luces
        lights.erase(std::remove(lights.begin(), lights.end(), light), lights.end());
    }
} 