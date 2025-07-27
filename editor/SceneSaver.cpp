// SceneSaver.cpp - Version con debug para heap corruption
#include "SceneSaver.h"
#include <iostream>
#include <core/FileSystem.h>
#include "components/LightComponent.h"
#include "components/AudioSource.h"

using namespace nlohmann;

bool SceneSaver::SaveScene(const Scene* scene, const std::string& filepath) {
    if (!scene) {
        std::cerr << "Error: Null scene pointer" << std::endl;
        return false;
    }

    json MainJson;

    auto* activeScene = SceneManager::getInstance().getActiveScene();
    if (!activeScene) {
        std::cerr << "Error: No active scene" << std::endl;
        return false;
    }

    const auto& gameObjects = activeScene->getGameObjects();
    for (size_t i = 0; i < gameObjects.size(); i++) {
        GameObject* obj = gameObjects[i];
        if (!obj || !obj->isValid()) {
            continue;
        }

        json subObject;

        // Datos básicos del objeto
        subObject["Name"] = obj->Name;
        subObject["Tag"] = obj->Tag;
        subObject["position"] = { obj->getWorldPosition().x, obj->getWorldPosition().y, obj->getWorldPosition().z };
        subObject["rotation"] = { obj->getWorldRotationEuler().x, obj->getWorldRotationEuler().y, obj->getWorldRotationEuler().z };
        subObject["scale"] = { obj->getLocalScale().x, obj->getLocalScale().y, obj->getLocalScale().z };

        // Guardar componentes
        json components;

        // Light Component
        const auto* lightComp = obj->getComponent<LightComponent>();
        if (lightComp && lightComp->isValid()) {
            try {
                json lightData;
                lightData["type"] = static_cast<int>(lightComp->getType());
                lightData["enabled"] = lightComp->isEnabled();
                
                glm::vec3 color = lightComp->getColor();
                lightData["color"] = { color.r, color.g, color.b };
                lightData["intensity"] = lightComp->getIntensity();

                switch (lightComp->getType()) {
                    case LightType::Directional: {
                        break;
                    }
                    case LightType::Point: {
                        glm::vec3 attenuation = lightComp->getAttenuation();
                        lightData["attenuation"] = { attenuation.x, attenuation.y, attenuation.z };
                        lightData["minDistance"] = lightComp->getMinDistance();
                        lightData["maxDistance"] = lightComp->getMaxDistance();
                        break;
                    }
                    case LightType::Spot: {
                        lightData["cutOffAngle"] = glm::degrees(lightComp->getCutOffAngle());
                        lightData["outerCutOffAngle"] = glm::degrees(lightComp->getOuterCutOffAngle());
                        lightData["spotRange"] = lightComp->getSpotRange();
                        break;
                    }
                }
                components["LightComponent"] = lightData;
            }
            catch (const std::exception& e) {
                std::cerr << "Error saving light component: " << e.what() << std::endl;
            }
        }

        // Audio Source Component
        const auto* audioComp = obj->getComponent<AudioSource>();
        if (audioComp && audioComp->isValid()) {
            try {
                json audioData;
                audioData["volume"] = audioComp->getVolume();
                audioData["is3D"] = audioComp->is3DEnabled();
                
                if (audioComp->is3DEnabled()) {
                    audioData["minDistance"] = audioComp->getMinDistance();
                    audioData["maxDistance"] = audioComp->getMaxDistance();
                }
                
                components["AudioSource"] = audioData;
            }
            catch (const std::exception& e) {
                std::cerr << "Error saving audio component: " << e.what() << std::endl;
            }
        }

        // Mesh/Geometry
        if (obj->hasGeometry()) {
            try {
                auto* geometry = obj->getGeometry();
                if (geometry != nullptr) {
                    const std::string& modelPath = obj->getModelPath();
                    if (!modelPath.empty()) {
                        json geometryData;
                        geometryData["modelPath"] = modelPath;
                        components["Geometry"] = geometryData;
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error saving geometry component: " << e.what() << std::endl;
            }
        }

        // Material
        if (obj && obj->getMaterial()) {
            json materialData;
            materialData["exists"] = true;
            components["Material"] = materialData;
        }

        // Añadir componentes al objeto
        subObject["components"] = components;
        MainJson["objects"].push_back(subObject);
    }

    bool success = FileSystem::writeString(filepath, MainJson.dump(4));
    if (success) {
        std::cout << "Scene saved successfully to: " << filepath << std::endl;
    } else {
        std::cerr << "Failed to save scene to: " << filepath << std::endl;
    }
    
    return success;
}