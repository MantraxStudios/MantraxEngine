#include "SceneManager.h"
#include <stdexcept>
#include <iostream>
#include "../components/PhysicalObject.h"

SceneManager::SceneManager() : activeScene(nullptr), physicsInitialized(false) {
}

SceneManager::~SceneManager() {
    cleanupPhysics();
}

bool SceneManager::initializePhysics() {
    if (physicsInitialized) {
        return true;
    }

    try {
        if (PhysicsManager::getInstance().initialize()) {
            physicsInitialized = true;
            std::cout << "Physics system initialized successfully" << std::endl;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize physics system: " << e.what() << std::endl;
    }
    return false;
}

void SceneManager::cleanupPhysics() {
    if (physicsInitialized) {
        PhysicsManager::getInstance().cleanup();
        physicsInitialized = false;
    }
}

void SceneManager::addScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
        throw std::invalid_argument("Cannot add null scene");
    }

    const std::string& sceneName = scene->getName();
    std::cout << "SceneManager::addScene() - Adding scene: " << sceneName << std::endl;
    
    if (scenes.find(sceneName) != scenes.end()) {
        throw std::runtime_error("Scene with name '" + sceneName + "' already exists");
    }

    scenes[sceneName] = std::move(scene);
    
    // Set RenderPipeline if available
    if (renderPipeline) {
        std::cout << "SceneManager::addScene() - RenderPipeline available, setting for " << sceneName << std::endl;
        scenes[sceneName]->setRenderPipeline(renderPipeline);
    } else {
        std::cout << "SceneManager::addScene() - No RenderPipeline available yet for " << sceneName << std::endl;
    }
    
    // If this is the first scene, make it active but DON'T initialize yet
    if (!activeScene) {
        activeScene = scenes[sceneName].get();
        std::cout << "SceneManager::addScene() - Set " << sceneName << " as active scene (not initialized yet)" << std::endl;
    }
}

void SceneManager::setActiveScene(const std::string& sceneName) {
    auto it = scenes.find(sceneName);
    if (it == scenes.end()) {
        throw std::runtime_error("Scene '" + sceneName + "' not found");
    }

    // Only change if it's a different scene
    if (activeScene != it->second.get()) {
        // Cleanup the current scene if it exists
        if (activeScene) {
            activeScene->cleanup();
        }
        
        // Set the new scene
        activeScene = it->second.get();
        
        // Ensure new scene has RenderPipeline access
        if (renderPipeline) {
            activeScene->setRenderPipeline(renderPipeline);
        }
        
        // Only initialize if not already initialized
        if (!activeScene->isInitialized()) {
            activeScene->initialize();
            activeScene->setInitialized(true);
        }
    }
}

Scene* SceneManager::getScene(const std::string& sceneName) {
    auto it = scenes.find(sceneName);
    if (it == scenes.end()) {
        return nullptr;
    }
    return it->second.get();
}

void SceneManager::update(float deltaTime) {
    // Update physics first
    if (physicsInitialized) {
        PhysicsManager::getInstance().update(deltaTime);
    }

    // Then update scene and game objects
    if (activeScene) {
        activeScene->update(deltaTime);
        activeScene->updateNative(deltaTime);
        
        // Initialize any PhysicalObjects that haven't been initialized yet
        if (physicsInitialized) {
            for (auto* obj : activeScene->getGameObjects()) {
                if (auto physicalObject = obj->getComponent<PhysicalObject>()) {
                    if (!physicalObject->isInitialized()) {
                        physicalObject->initializePhysics();
                    }
                }
            }
        }
    }
}

void SceneManager::setupRenderPipeline(RenderPipeline& pipeline) {
    if (!activeScene) return;

    // Add all game objects from the active scene
    for (auto* obj : activeScene->getGameObjects()) {
        pipeline.AddGameObject(obj);
    }

    // Add all lights from the active scene
    for (const auto& light : activeScene->getLights()) {
        pipeline.AddLight(light);
    }
}

void SceneManager::initializeAllScenes() {
    std::cout << "SceneManager::initializeAllScenes() called" << std::endl;
    
    // Initialize physics system first
    if (!physicsInitialized) {
        if (!initializePhysics()) {
            std::cerr << "WARNING: Failed to initialize physics system!" << std::endl;
        }
    }

    if (!renderPipeline) {
        std::cerr << "ERROR: Cannot initialize scenes without RenderPipeline!" << std::endl;
        return;
    }
    
    std::cout << "Found " << scenes.size() << " scenes to initialize" << std::endl;
    
    // Set RenderPipeline for all scenes but only initialize the active one
    for (auto& pair : scenes) {
        std::cout << "Setting RenderPipeline for scene: " << pair.first << std::endl;
        pair.second->setRenderPipeline(renderPipeline);
        
        // Only initialize the active scene
        if (pair.second.get() == activeScene) {
            std::cout << "Initializing active scene: " << pair.first << std::endl;
            pair.second->initialize();
            pair.second->setInitialized(true);
            std::cout << "Active scene " << pair.first << " initialized. Has camera: " << (pair.second->getCamera() ? "YES" : "NO") << std::endl;
        }
    }
    
    std::cout << "RenderPipeline set for all scenes, active scene initialized" << std::endl;
} 