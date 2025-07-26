#include "Scene.h"
#include "../render/RenderPipeline.h"
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

void Scene::update(float deltaTime) {
    // Update camera if available
    if (camera) {
        camera->update(deltaTime);
    }

    // Update all game objects
    for (auto& gameObject : gameObjects) {
        if (gameObject) {
            gameObject->update(deltaTime);
        }
    }
} 