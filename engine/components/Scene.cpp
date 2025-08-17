#include "Scene.h"
#include "../render/RenderPipeline.h"
#include "../components/PhysicalObject.h"
#include "SceneManager.h"
#include <iostream>

Scene::Scene(const std::string& name) : name(name), initialized(false), camera(nullptr), renderPipeline(nullptr) {
    std::cout << "Scene: Creating new scene: " << name << std::endl;
    std::cout << "Scene: Initial camera state: " << (camera ? "Valid" : "Null") << std::endl;
}

void Scene::initialize() {
    std::cout << "Scene: Initializing scene: " << name << std::endl;
    
    // Verificar que la cámara esté configurada
    if (!camera) {
        std::cerr << "Scene: ERROR - Cannot initialize scene without camera!" << std::endl;
        std::cerr << "Scene: Please set a camera before initializing the scene" << std::endl;
        return;
    }
    
    // Verificar que el RenderPipeline esté configurado
    if (!renderPipeline) {
        std::cerr << "Scene: WARNING - Scene initialized without RenderPipeline" << std::endl;
        std::cerr << "Scene: Some functionality may be limited" << std::endl;
    }
    
    std::cout << "Scene: Scene initialized successfully: " << name << std::endl;
    std::cout << "Scene: Camera position: (" << camera->getPosition().x << ", " 
              << camera->getPosition().y << ", " << camera->getPosition().z << ")" << std::endl;
    std::cout << "Scene: RenderPipeline: " << (renderPipeline ? "Configured" : "Not configured") << std::endl;
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

void Scene::removeGameObject(GameObject* object) {
    if (object) {
        // Verificar que el objeto esté en la lista antes de eliminarlo
        auto it = std::find(gameObjects.begin(), gameObjects.end(), object);
        if (it != gameObjects.end()) {
            // Remover del RenderPipeline primero
            if (renderPipeline) {
                renderPipeline->RemoveGameObject(object);
            }
            // Remover de la lista de game objects
            gameObjects.erase(it);
            // Eliminar el objeto de la memoria
            delete object;
        }
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

void Scene::setCamera(std::unique_ptr<Camera> newCamera) {
    if (!newCamera) {
        std::cerr << "Scene: ERROR - Attempting to set null camera!" << std::endl;
        return;
    }
    
    std::cout << "Scene: Setting camera for scene: " << name << std::endl;
    std::cout << "Scene: Camera position: (" << newCamera->getPosition().x << ", " 
              << newCamera->getPosition().y << ", " << newCamera->getPosition().z << ")" << std::endl;
    
    // Mover la cámara a la escena
    camera = std::move(newCamera);
    
    // Verificar que se estableció correctamente
    if (camera) {
        std::cout << "Scene: Camera set successfully for scene: " << name << std::endl;
        std::cout << "Scene: Final camera position: (" << camera->getPosition().x << ", " 
                  << camera->getPosition().y << ", " << camera->getPosition().z << ")" << std::endl;
    } else {
        std::cerr << "Scene: ERROR - Camera not properly set after move!" << std::endl;
    }
} 