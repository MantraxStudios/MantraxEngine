#include "Scene.h"
#include "../render/RenderPipeline.h"

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