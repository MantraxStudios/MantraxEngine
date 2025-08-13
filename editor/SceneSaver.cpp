#include "SceneSaver.h"
#include <iostream>
#include <core/FileSystem.h>
#include <components/LightComponent.h>
#include <components/AudioSource.h>
#include <components/PhysicalObject.h>
#include <components/CharacterController.h>
#include <components/SpriteAnimator.h>
#include <components/ScriptExecutor.h>
#include "windows/Selection.h"
#include <render/RenderPipeline.h>
#include "EUI/EditorInfo.h"
#include "render/DefaultShaders.h"
#include "EUI/EditorInfo.h"
#include "Windows/FileExplorer.h"
#include <glm/glm.hpp>
#include <cmath> 

using namespace nlohmann;

bool SceneSaver::SaveScene(const Scene* scene, const std::string& filepath) {

    if (!scene) {
        std::cerr << "Error: Null scene pointer" << std::endl;
        return false;
    }

    json MainJson;

    Scene* activeScene = SceneManager::getInstance().getActiveScene();
    RenderPipeline* pipeline = activeScene->getRenderPipeline();

    MainJson["name"] = FileSystem::getFileNameWithoutExtension(filepath);

    // Guardar información de la cámara
    if (scene->getCamera()) {
        MainJson["Camera"]["Position"] = { 
            scene->getCamera()->getPosition().x, 
            scene->getCamera()->getPosition().y, 
            scene->getCamera()->getPosition().z 
        };
        // Nota: Camera no tiene getTarget(), usamos getForward() como alternativa
        MainJson["Camera"]["Forward"] = { 
            scene->getCamera()->getForward().x, 
            scene->getCamera()->getForward().y, 
            scene->getCamera()->getForward().z 
        };
        MainJson["Camera"]["ProjectionType"] = scene->getCamera()->getProjectionType() == ProjectionType::Orthographic ? 1 : 0;
        MainJson["Camera"]["FOV"] = scene->getCamera()->getFOV();
        MainJson["Camera"]["OrthographicSize"] = scene->getCamera()->getOrthographicSize();
        MainJson["Camera"]["NearPlane"] = scene->getCamera()->getNearClip();
        MainJson["Camera"]["FarPlane"] = scene->getCamera()->getFarClip();
    }
    
    MainJson["Settings"]["CameraType"] = scene->getCamera()->getProjectionType() == ProjectionType::Orthographic ? 1 : 0;
    MainJson["Settings"]["CameraFov"] = scene->getCamera()->getOrthographicSize();
    MainJson["Settings"]["AmbientIntensity"] = pipeline->getAmbientIntensity();
    MainJson["Settings"]["LowAmbient"] = pipeline->getLowAmbient();
    MainJson["Settings"]["LightType"] = pipeline->getUsePBR();
    MainJson["Settings"]["FrustrumOn"] = pipeline->getFrustumCulling();

    // Post-processing settings
    DefaultShaders* shaders = pipeline->getShaders();
    if (shaders) {
        float exposure = shaders->getExposure();
        float saturation = shaders->getSaturation();
        float smoothness = shaders->getSmoothness();
        
        MainJson["Settings"]["PostProcessing"]["Exposure"] = exposure;
        MainJson["Settings"]["PostProcessing"]["Saturation"] = saturation;
        MainJson["Settings"]["PostProcessing"]["Smoothness"] = smoothness;
        
        std::cout << "Saving Post-Processing - Exposure: " << exposure 
                  << ", Saturation: " << saturation 
                  << ", Smoothness: " << smoothness << std::endl;
    }

    if (!activeScene) {
        std::cerr << "Error: No active scene" << std::endl;
        return false;
    }


    //MainJson["TileData"][]

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
        subObject["ObjectID"] = obj->ObjectID;
        subObject["position"] = { obj->getWorldPosition().x, obj->getWorldPosition().y, obj->getWorldPosition().z };
        subObject["rotation"] = { obj->getWorldRotationEuler().x, obj->getWorldRotationEuler().y, obj->getWorldRotationEuler().z };
        subObject["scale"] = { obj->getLocalScale().x, obj->getLocalScale().y, obj->getLocalScale().z };

        // Guardar información del padre si existe
        if (obj->hasParent()) {
            GameObject* parent = obj->getParent();
            if (parent && parent->isValid()) {
                subObject["parentID"] = parent->ObjectID;
            }
        }

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

    std::string sceneName = FileSystem::getFileNameWithoutExtension(filepath);
    auto newScene = std::make_unique<Scene>(sceneName);

    // 2. Configurar el RenderPipeline
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
    }

    // 3. Crear la cámara con información guardada o valores por defecto
    std::unique_ptr<Camera> camera;
    
    if (MainJson.contains("Camera")) {
        // Cargar información de la cámara desde el archivo
        const json& cameraData = MainJson["Camera"];
        
        float fov = cameraData.value("FOV", 65.0f);
        float aspectRatio = cameraData.value("AspectRatio", 1200.0f / 800.0f);
        float nearPlane = cameraData.value("NearPlane", 0.1f);
        float farPlane = cameraData.value("FarPlane", 1000.0f);
        
        camera = std::make_unique<Camera>(fov, aspectRatio, nearPlane, farPlane);
        
        // Cargar posición y forward
        if (cameraData.contains("Position")) {
            auto pos = cameraData["Position"];
            camera->setPosition(glm::vec3(pos[0], pos[1], pos[2]));
        }
        
        // Nota: Camera no tiene setTarget(), usamos setRotation() para orientar la cámara
        if (cameraData.contains("Forward")) {
            auto forward = cameraData["Forward"];
            glm::vec3 forwardVec = glm::vec3(forward[0], forward[1], forward[2]);
            // Calcular yaw y pitch desde el vector forward
            float yaw = atan2(forwardVec.x, forwardVec.z);
            float pitch = asin(-forwardVec.y);
            camera->setRotation(yaw, pitch);
        }
        
        // Cargar tipo de proyección
        if (cameraData.contains("ProjectionType")) {
            int projectionType = cameraData["ProjectionType"];
            ProjectionType type = (projectionType == 1) ? ProjectionType::Orthographic : ProjectionType::Perspective;
            camera->setProjectionType(type, true);
        }
        
        // Cargar tamaño ortográfico si es necesario
        if (cameraData.contains("OrthographicSize")) {
            float orthoSize = cameraData["OrthographicSize"];
            camera->setOrthographicSize(orthoSize);
        }
        
        std::cout << "Loaded camera from scene file" << std::endl;
    } else {
        // Crear cámara por defecto si no hay información guardada
        camera = std::make_unique<Camera>(65.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
        camera->setPosition(glm::vec3(0.0f, 5.0f, 10.0f));
        camera->setTarget(glm::vec3(0.0f));
        std::cout << "Created default camera" << std::endl;
    }
    
    newScene->setCamera(std::move(camera));

    if (!MainJson.contains("objects")) {
        std::cerr << "No hay 'objects' en el archivo de escena." << std::endl;
        return false;
    }

    // Primera pasada: crear todos los objetos y almacenar información de parent-child
    std::vector<std::pair<GameObject*, std::string>> parentChildPairs; // pair<child, parentID>
    
    for (const auto& subObject : MainJson["objects"]) {
        GameObject* obj = new GameObject();
        obj->Name = subObject.value("Name", "New Object");
        obj->Tag = subObject.value("Tag", "");
        
        // Cargar ObjectID si existe, sino mantener el generado automáticamente
        if (subObject.contains("ObjectID")) {
            obj->ObjectID = subObject["ObjectID"].get<std::string>();
        }

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

        // Almacenar información de parent-child para procesar después
        if (subObject.contains("parentID")) {
            std::string parentID = subObject["parentID"].get<std::string>();
            parentChildPairs.push_back({obj, parentID});
        }

        newScene->addGameObject(obj);
    }

    // Segunda pasada: establecer parent-child relationships
    for (const auto& pair : parentChildPairs) {
        GameObject* child = pair.first;
        const std::string& parentID = pair.second;
        
        // Buscar el padre en la escena por ID
        GameObject* parent = nullptr;
        for (GameObject* obj : newScene->getGameObjects()) {
            if (obj->ObjectID == parentID) {
                parent = obj;
                break;
            }
        }
        
        if (parent && parent->isValid()) {
            child->setParent(parent);
            std::cout << "Established parent-child relationship: '" << parent->Name << "' (ID: " << parentID << ") -> '" << child->Name << "' (ID: " << child->ObjectID << ")" << std::endl;
        } else {
            std::cerr << "Warning: Parent GameObject with ID '" << parentID << "' not found for object '" << child->Name << "' (ID: " << child->ObjectID << ")." << std::endl;
        }
    }

    // Tercera pasada: cargar componentes para todos los objetos
    for (size_t i = 0; i < MainJson["objects"].size(); i++) {
        const auto& subObject = MainJson["objects"][i];
        GameObject* obj = newScene->getGameObjects()[i];

        if (subObject.contains("components")) {
            const json& components = subObject["components"];
            if (components.contains("component")) {
                for (const auto& compData : components["component"]) {
                    if (compData.contains("type")) {
                        try {
                            std::string type = compData.at("type").get<std::string>(); 

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
                                physComp->initializePhysics();
                                physComp->deserialize(compData.dump());
                            }
                            else if (type == "CharacterController") {
                                auto* ccComp = obj->addComponent<CharacterController>();
                                ccComp->deserialize(compData.dump());
                            }
                            else if (type == "SpriteAnimator") {
                                auto* spaComp = obj->addComponent<SpriteAnimator>();
                                spaComp->deserialize(compData.dump());
                            }
                        }
                        catch (const nlohmann::json::exception& e) {
                            std::cerr << "Error al cargar componente: " << e.what() << std::endl;
                            std::cerr << "CompData problemático:\n" << compData.dump(4) << std::endl;
                        }
                    }
                }
            }

            if (components.contains("Geometry")) {
                const std::string& modelPath = components["Geometry"]["modelPath"].get<std::string>();
                obj->setModelPath(modelPath);
                obj->loadModelFromPath();

                std::cout << "Loading Model: " << modelPath << std::endl;
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
    }

    newScene->initialize();
    newScene->setInitialized(true);

    auto& settings = MainJson["Settings"];

    // CameraType: setear el tipo de proyección
    if (settings.contains("CameraType")) {
        int cameraType = settings["CameraType"];
        ProjectionType type = (cameraType == 1) ? ProjectionType::Orthographic : ProjectionType::Perspective;
        newScene->getCamera()->setProjectionType(type, true);
    }

    // AmbientIntensity
    if (settings.contains("AmbientIntensity")) {
        float ambientIntensity = settings["AmbientIntensity"];
        newScene.get()->getRenderPipeline()->setAmbientIntensity(ambientIntensity);
    }

    // LowAmbient
    if (settings.contains("LowAmbient")) {
        float lowAmbient = settings["LowAmbient"];
        newScene.get()->getRenderPipeline()->setLowAmbient(lowAmbient);
    }

    // LightType
    if (settings.contains("LightType")) {
        bool usePBR = settings["LightType"];
        newScene.get()->getRenderPipeline()->setUsePBR(usePBR);
    }

    // CameraFov
    if (settings.contains("CameraFov")) {
        float cameraFovAmount = settings["CameraFov"];
        newScene.get()->getCamera()->setOrthographicSize(cameraFovAmount);
    }

    // FrustrumOn
    if (settings.contains("FrustrumOn")) {
        bool frustumOn = settings["FrustrumOn"];
        newScene.get()->getRenderPipeline()->setFrustumCulling(frustumOn);
    }

    // 7. Agregar la escena al manager y activar
    sceneManager.addScene(std::move(newScene));
    sceneManager.setActiveScene(sceneName);
    
    // Verificar que los objetos se cargaron correctamente
    Scene* loadedScene = sceneManager.getActiveScene();
    if (loadedScene) {
        std::cout << "Scene loaded successfully. Total objects: " << loadedScene->getGameObjects().size() << std::endl;
        for (const auto* obj : loadedScene->getGameObjects()) {
            std::cout << "  - Object: " << obj->Name << " (ID: " << obj->ObjectID << ")" << std::endl;
        }
    }
    
    // Post-processing settings - CARGAR DESPUÉS DE CONFIGURAR LA ESCENA
    if (settings.contains("PostProcessing")) {
        const json& postProcessing = settings["PostProcessing"];
        Scene* loadedScene = sceneManager.getActiveScene();
        if (loadedScene) {
            RenderPipeline* loadedPipeline = loadedScene->getRenderPipeline();
            if (loadedPipeline) {
                DefaultShaders* shaders = loadedPipeline->getShaders();
                if (shaders) {
                    if (postProcessing.contains("Exposure")) {
                        float exposure = postProcessing["Exposure"];
                        shaders->setExposure(exposure);
                        std::cout << "Loaded Exposure: " << exposure << std::endl;
                    }
                    
                    if (postProcessing.contains("Saturation")) {
                        float saturation = postProcessing["Saturation"];
                        shaders->setSaturation(saturation);
                        std::cout << "Loaded Saturation: " << saturation << std::endl;
                    }
                    
                    if (postProcessing.contains("Smoothness")) {
                        float smoothness = postProcessing["Smoothness"];
                        shaders->setSmoothness(smoothness);
                        std::cout << "Loaded Smoothness: " << smoothness << std::endl;
                    }
                }
            }
        }
    }

    std::cout << "Scene loaded successfully from: " << filepath << std::endl;
    return true;
}


Scene* SceneSaver::MakeNewScene(std::string sceneName) {
    auto& sceneManager = SceneManager::getInstance();
    Scene* newScene = new Scene(sceneName);

    EditorInfo::currentScenePath = "";

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