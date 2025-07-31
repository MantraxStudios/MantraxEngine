#include "SceneSaver.h"
#include <iostream>
#include <core/FileSystem.h>
#include <components/LightComponent.h>
#include <components/AudioSource.h>
#include <components/PhysicalObject.h>
#include <components/CharacterController.h>
#include <components/ScriptExecutor.h>
#include "windows/Selection.h"
#include <render/RenderPipeline.h>
#include "EUI/EditorInfo.h"
#include "render/DefaultShaders.h"
#include "EUI/EditorInfo.h"
#include "Windows/FileExplorer.h" 

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
                return;

            try {
                json data = json::parse(serialized);

                // Si el campo "type" NO existe, lo insertamos automáticamente
                if (!data.contains("type")) {
                    std::string rawType = typeid(*comp).name();

                    // Remueve "class " o "struct " al inicio (si existe)
                    const std::string prefixes[] = { "class ", "struct " };
                    for (const auto& prefix : prefixes) {
                        if (rawType.find(prefix) == 0) {
                            rawType = rawType.substr(prefix.length());
                            break;
                        }
                    }
                    data["type"] = rawType;
                }

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
            components["MaterialName"] = obj->getMaterial().get()->getName();
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

bool SceneSaver::LoadScene(const std::string& filepath) {
    auto& sceneManager = SceneManager::getInstance();

    // 1. Leer el archivo JSON de escena
    std::string jsonStr;
    if (!FileSystem::readString(filepath, jsonStr)) {
        std::cerr << "Error: No se pudo leer el archivo " << filepath << std::endl;
        return false;
    }

    json MainJson;
    try {
        MainJson = json::parse(jsonStr);
    }
    catch (const json::parse_error& e) {
        std::cerr << "Error al parsear JSON: " << e.what() << std::endl;
        return false;
    }

    // 2. Crear la nueva escena con nombre desde el JSON
    std::string sceneName = MainJson.contains("name") ? MainJson["name"].get<std::string>() : "ExampleScene";
    auto newScene = std::make_unique<Scene>(sceneName);

    // 3. Crear la cámara por defecto (puedes mejorar para buscarla en los GameObjects cargados)
    auto camera = std::make_unique<Camera>(65.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    camera->setPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    camera->setTarget(glm::vec3(0.0f));
    newScene->setCamera(std::move(camera));

    // 4. Configurar el RenderPipeline
    if (auto currentScene = sceneManager.getActiveScene()) {
        newScene->setRenderPipeline(currentScene->getRenderPipeline());
        std::cout << "Loading Pipeline in current scene for the new scene" << std::endl;
    }
    else {
        std::unique_ptr<DefaultShaders> shaders = std::make_unique<DefaultShaders>();
        auto pipeline = std::make_unique<RenderPipeline>(newScene->getCamera(), shaders.get());
        if (!pipeline->loadMaterialsFromConfig("config/materials_config.json")) {
            std::cerr << "Error: Failed to load materials configuration" << std::endl;
        }
        else {
            std::cout << "Materials Loaded" << std::endl;
        }
        newScene->setRenderPipeline(pipeline.get());
        // Si necesitas ownership del pipeline, guárdalo donde corresponda
    }

    // 5. Cargar GameObjects desde el JSON (igual que antes)
    if (!MainJson.contains("objects")) {
        std::cerr << "No hay 'objects' en el archivo de escena." << std::endl;
        return false;
    }
    for (const auto& subObject : MainJson["objects"]) {
        GameObject* obj = new GameObject();
        obj->Name = subObject.value("Name", "New Object");
        obj->Tag = subObject.value("Tag", "");

        if (subObject.contains("position")) {
            auto pos = subObject["position"];
            obj->setWorldPosition(glm::vec3(pos[0], pos[1], pos[2]));
        }
        if (subObject.contains("rotation")) {
            auto rot = subObject["rotation"];
            obj->setWorldRotationEuler(glm::vec3(rot[0], rot[1], rot[2]));
        }
        if (subObject.contains("scale")) {
            auto scl = subObject["scale"];
            obj->setLocalScale(glm::vec3(scl[0], scl[1], scl[2]));
        }

        // Cargar componentes...
        if (subObject.contains("components")) {
            const json& components = subObject["components"];
            if (components.contains("component")) {
                for (const auto& compData : components["component"]) {
                    if (compData.contains("type")) {
                        try {
                            std::string type = compData.at("type").get<std::string>(); // Usa .at() para throw si no existe

                            if (type == "ScriptExecutor") {
                                auto* scriptComp = obj->addComponent<ScriptExecutor>();
                                scriptComp->deserialize(compData.dump());
                            }
                            else if (type == "LightComponent") {
                                auto* lightComp = obj->addComponent<LightComponent>();
                                lightComp->deserialize(compData.dump());
                            }
                            else if (type == "AudioSource") {
                                auto* audioComp = obj->addComponent<AudioSource>();
                                audioComp->deserialize(compData.dump());
                            }
                            else if (type == "PhysicalObject") {
                                auto* physComp = obj->addComponent<PhysicalObject>(obj);
                                physComp->deserialize(compData.dump());
                            }
                            else if (type == "CharacterController") {
                                auto* ccComp = obj->addComponent<CharacterController>();
                                ccComp->deserialize(compData.dump());
                            }
                            // ...otros tipos
                        }
                        catch (const nlohmann::json::exception& e) {
                            std::cerr << "Error al cargar componente: " << e.what() << std::endl;
                            std::cerr << "CompData problemático:\n" << compData.dump(4) << std::endl;
                        }
                    }
                }
            }
            if (components.contains("Geometry") && obj->hasGeometry()) {
                const std::string& modelPath = components["Geometry"]["modelPath"].get<std::string>();
                obj->setModelPath(modelPath);
            }

            EditorInfo::pipeline->listMaterials();

            if (components.contains("MaterialName")) {
                std::string matName = components["MaterialName"].get<std::string>();

                auto material = newScene.get()->getRenderPipeline()->getMaterial(matName);
                if (material) {
                    obj->setMaterial(material);
                    std::cout << "MaterialSetup" << std::endl;
                }
            }
        }

        newScene->addGameObject(obj);
    }

    // 6. Inicializar la escena
    newScene->initialize();
    newScene->setInitialized(true);

    // 7. Agregar la escena al manager y activar
    sceneManager.addScene(std::move(newScene));
    sceneManager.setActiveScene(sceneName);

    std::cout << "Scene loaded successfully from: " << filepath << std::endl;
    return true;
}


Scene* SceneSaver::MakeNewScene(std::string sceneName) {
    auto& sceneManager = SceneManager::getInstance();
    Scene* newScene = new Scene(sceneName);

    auto camera = std::make_unique<Camera>(65.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    camera->setPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    camera->setTarget(glm::vec3(0.0f));
    newScene->setCamera(std::move(camera));

    if (auto currentScene = sceneManager.getActiveScene()) {
        newScene->setRenderPipeline(currentScene->getRenderPipeline());
    }
    else {
        std::unique_ptr<DefaultShaders> shaders = std::make_unique<DefaultShaders>();
        RenderPipeline* pipeline = new RenderPipeline(newScene->getCamera(), shaders.get());

        if (!pipeline->loadMaterialsFromConfig("config/materials_config.json")) {
            std::cerr << "Warning: Failed to load materials configuration" << std::endl;
        }

        newScene->setRenderPipeline(pipeline);
    }

    newScene->initialize();
    newScene->setInitialized(true);
    sceneManager.addScene(std::unique_ptr<Scene>(newScene));
    sceneManager.setActiveScene(sceneName);

    return newScene;
}