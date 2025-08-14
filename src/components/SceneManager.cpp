#include "SceneManager.h"
#include <stdexcept>
#include <iostream>
#include "../components/PhysicalObject.h"
#include "../components/Collider.h"
#include "../components/Rigidbody.h"

SceneManager::SceneManager() : activeScene(nullptr), physicsInitialized(false) {
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
        std::cout << "SceneManager: Starting physics cleanup..." << std::endl;
        
        // First, cleanup all scenes to destroy all PhysicalObjects
        for (auto& scenePair : scenes) {
            if (scenePair.second) {
                std::cout << "SceneManager: Cleaning up scene: " << scenePair.first << std::endl;
                scenePair.second->cleanup();
            }
        }
        
        // Clear the active scene reference
        activeScene = nullptr;
        
        // Force cleanup all remaining physics objects
        std::cout << "SceneManager: Force cleaning up all physics objects..." << std::endl;
        PhysicsManager::getInstance().forceCleanupAllObjects();
        
        // Now cleanup the PhysicsManager
        std::cout << "SceneManager: Cleaning up PhysicsManager..." << std::endl;
        PhysicsManager::getInstance().cleanup();
        physicsInitialized = false;
        
        std::cout << "SceneManager: Physics cleanup completed." << std::endl;
    }
}

void SceneManager::addScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
        throw std::invalid_argument("Cannot add null scene");
    }

    const std::string& sceneName = scene->getName();
    std::cout << "SceneManager::addScene() - Adding scene: " << sceneName << std::endl;
    
    if (scenes.find(sceneName) != scenes.end()) {
        std::cout << ("Scene with name '" + sceneName + "' already exists") << std::endl;
        return;
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
            std::cout << "Switching from scene: " << activeScene->getName() << " to: " << sceneName << std::endl;
            // Cleanup physics components before switching scenes
            cleanupPhysicsComponents(activeScene);
            // No llamar cleanup() aquí para preservar los objetos
            // activeScene->cleanup();
        }
        
        // Set the new scene
        activeScene = it->second.get();
        
        // Ensure new scene has RenderPipeline access
        if (renderPipeline) {
            activeScene->setRenderPipeline(renderPipeline);
        }
        
        // Always reinitialize the scene to ensure objects are properly loaded
        if (!activeScene->isInitialized()) {
            activeScene->initialize();
            activeScene->setInitialized(true);
        }
        
        // Reinitialize physics components for the new scene
        reinitializePhysicsComponents();
        
        std::cout << "Scene switched successfully. Objects in scene: " << activeScene->getGameObjects().size() << std::endl;
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
    // Initialize physics if not already done
    if (!physicsInitialized) {
        initializePhysics();
    }
    
    // Update physics
    if (physicsInitialized) {
        PhysicsManager::getInstance().update(deltaTime);
    }
    
    // Update active scene
    if (activeScene) {
        activeScene->updateNative(deltaTime);
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

void SceneManager::cleanupPhysicsComponents(Scene* scene) {
    if (!scene) return;
    
    std::cout << "SceneManager: Cleaning up physics components for scene: " << scene->getName() << std::endl;
    
    // First, cleanup all physics objects from the scene using PhysicsManager
    PhysicsManager::getInstance().cleanupScenePhysicsComponents();
    
    // Get all game objects from the scene
    const auto& gameObjects = scene->getGameObjects();
    
    for (auto* gameObject : gameObjects) {
        if (!gameObject) continue;
        
        // Cleanup Collider components
        auto* collider = gameObject->getComponent<Collider>();
        if (collider) {
            std::cout << "SceneManager: Cleaning up Collider for GameObject: " << gameObject->Name << std::endl;
            collider->destroy();
        }
        
        // Cleanup Rigidbody components
        auto* rigidbody = gameObject->getComponent<Rigidbody>();
        if (rigidbody) {
            std::cout << "SceneManager: Cleaning up Rigidbody for GameObject: " << gameObject->Name << std::endl;
            rigidbody->destroy();
        }
        
        // Cleanup PhysicalObject components (if they exist)
        auto* physicalObject = gameObject->getComponent<PhysicalObject>();
        if (physicalObject) {
            std::cout << "SceneManager: Cleaning up PhysicalObject for GameObject: " << gameObject->Name << std::endl;
            physicalObject->destroy();
        }
    }
    
    std::cout << "SceneManager: Physics components cleanup completed for scene: " << scene->getName() << std::endl;
}

void SceneManager::reinitializePhysicsComponents() {
    if (!activeScene) return;
    
    std::cout << "SceneManager: Reinitializing physics components for scene: " << activeScene->getName() << std::endl;
    
    // Get all game objects from the active scene
    const auto& gameObjects = activeScene->getGameObjects();
    
    for (auto* gameObject : gameObjects) {
        if (!gameObject) continue;
        
        // Reinitialize Collider components
        auto* collider = gameObject->getComponent<Collider>();
        if (collider) {
            std::cout << "SceneManager: Reinitializing Collider for GameObject: " << gameObject->Name << std::endl;
            collider->start();
        }
        
        // Reinitialize Rigidbody components
        auto* rigidbody = gameObject->getComponent<Rigidbody>();
        if (rigidbody) {
            std::cout << "SceneManager: Reinitializing Rigidbody for GameObject: " << gameObject->Name << std::endl;
            rigidbody->start();
        }
        
        // Reinitialize PhysicalObject components (if they exist)
        auto* physicalObject = gameObject->getComponent<PhysicalObject>();
        if (physicalObject) {
            std::cout << "SceneManager: Reinitializing PhysicalObject for GameObject: " << gameObject->Name << std::endl;
            physicalObject->start();
        }
    }
    
    std::cout << "SceneManager: Physics components reinitialization completed for scene: " << activeScene->getName() << std::endl;
}

void SceneManager::removeScene(const std::string& sceneName) {
    auto it = scenes.find(sceneName);
    if (it == scenes.end()) {
        std::cout << "SceneManager: Scene '" << sceneName << "' not found for removal" << std::endl;
        return;
    }
    
    std::cout << "SceneManager: Removing scene: " << sceneName << std::endl;
    
    // Si es la escena activa, limpiar primero
    if (activeScene == it->second.get()) {
        std::cout << "SceneManager: Scene '" << sceneName << "' is active, cleaning up first..." << std::endl;
        
        // Limpiar componentes de física
        cleanupPhysicsComponents(activeScene);
        
        // Limpiar la escena
        activeScene->cleanup();
        
        // Resetear la escena activa
        activeScene = nullptr;
    }
    
    // Remover la escena del map
    scenes.erase(it);
    
    std::cout << "SceneManager: Scene '" << sceneName << "' removed successfully" << std::endl;
} 