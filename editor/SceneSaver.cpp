#include "SceneSaver.h"
#include <iostream>
#include <core/FileSystem.h>
#include <components/LightComponent.h>
#include <components/AudioSource.h>
#include <components/PhysicalObject.h>
#include <components/CharacterController.h>
#include <components/ScriptExecutor.h>
#include "windows/Selection.h"

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

        json components;


        auto saveComponent = [](const Component* comp, json& salida) {
            if (!comp) return;

            std::string serialized = comp->serializeComponent();
            if (serialized.find_first_not_of(" \t\n\r") == std::string::npos)
                return; // string vacío o solo espacios

            try {
                json data = json::parse(serialized);
                salida["component"].push_back(data);
            }
            catch (const json::parse_error& e) {
                std::cerr << "Error al parsear JSON del componente (" << typeid(*comp).name() << "): " << e.what() << "\n";
            }
        };


        for (auto& get : obj->getAllComponents())
        {
            saveComponent(get, components);
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
    }
    else {
        std::cerr << "Failed to save scene to: " << filepath << std::endl;
    }

    return success;
}