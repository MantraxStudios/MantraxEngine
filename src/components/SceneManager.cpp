#include "SceneManager.h"
#include <stdexcept>
#include <iostream>

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
        
        // Set and initialize the new scene
        activeScene = it->second.get();
        
        // Ensure new scene has RenderPipeline access
        if (renderPipeline) {
            activeScene->setRenderPipeline(renderPipeline);
        }
        
        activeScene->initialize();
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
    if (activeScene) {
        activeScene->update(deltaTime);
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
    
    if (!renderPipeline) {
        std::cerr << "ERROR: Cannot initialize scenes without RenderPipeline!" << std::endl;
        return;
    }
    
    std::cout << "Found " << scenes.size() << " scenes to initialize" << std::endl;
    
    // Set RenderPipeline for all scenes and initialize them
    for (auto& pair : scenes) {
        std::cout << "Initializing scene: " << pair.first << std::endl;
        pair.second->setRenderPipeline(renderPipeline);
        pair.second->initialize();
        std::cout << "Scene " << pair.first << " initialized. Has camera: " << (pair.second->getCamera() ? "YES" : "NO") << std::endl;
    }
    
    std::cout << "All scenes initialized with RenderPipeline" << std::endl;
} 