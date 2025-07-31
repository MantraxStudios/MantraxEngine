#include "Inspector.h"
#include "../../src/components/SceneManager.h"
#include "../../src/components/GameObject.h"
#include "../../src/components/LightComponent.h"
#include "../../src/components/PhysicalObject.h"
#include "../../src/components/ScriptExecutor.h"
#include "../../src/components/CharacterController.h"
#include "../../src/components/GameBehaviourFactory.h"
#include "../../src/render/MaterialManager.h"
#include "../../src/core/PhysicsManager.h"
#include "../../src/render/RenderConfig.h"
#include "Selection.h"
#include "../EUI/UIBuilder.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <iostream>
#include <filesystem>
#include "FileExplorer.h"
#include <core/FileSystem.h>

// Helper function to calculate collision mask from checkboxes
CollisionMask CalculateCollisionMask(bool staticObj, bool dynamicObj, bool trigger, bool player, bool enemy, bool projectile, bool sensor) {
    CollisionMask mask = static_cast<CollisionMask>(0);
    if (staticObj) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::STATIC_MASK));
    if (dynamicObj) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::DYNAMIC_MASK));
    if (trigger) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::TRIGGER_MASK));
    if (player) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::PLAYER_MASK));
    if (enemy) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::ENEMY_MASK));
    if (projectile) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::PROJECTILE_MASK));
    if (sensor) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::SENSOR_MASK));
    return mask;
}

void Inspector::OnRenderGUI() {
    if (!isOpen) return;

    ImGui::Begin("Inspector", &isOpen);

    GameObject* go = Selection::GameObjectSelect;
    std::shared_ptr<Light> light = Selection::LightSelect;

    if (go != nullptr && go->isValid()) {
        RenderGameObjectInspector(go);
    }
    else if (light != nullptr) {
        RenderLightInspector(light);
    }
    else {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}

void Inspector::RenderGameObjectInspector(GameObject* go) {
    if (!go || !go->isValid()) {
        ImGui::Text("Invalid GameObject.");
        return;
    }

    // Nombre del objeto
    char nameBuffer[128] = {};
    strncpy_s(nameBuffer, sizeof(nameBuffer), go->Name.c_str(), _TRUNCATE);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        go->Name = std::string(nameBuffer);
    }

    ImGui::Separator();
    ImGui::Text("Rendering Options");

    // Control de renderizado
    bool shouldRender = go->isRenderEnabled();
    if (ImGui::Checkbox("Enable Rendering", &shouldRender)) {
        go->setRenderEnabled(shouldRender);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle mesh rendering for this object");
    }

    // Control de actualizaci�n del transform
    bool shouldUpdateTransform = go->isTransformUpdateEnabled();
    if (ImGui::Checkbox("Enable Transform Updates", &shouldUpdateTransform)) {
        go->setTransformUpdateEnabled(shouldUpdateTransform);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle automatic transform matrix updates");
    }

    ImGui::Separator();
    ImGui::Text("Transform");

    // Solo mostrar controles de transform si est� habilitada la actualizaci�n
    if (shouldUpdateTransform) {
        glm::vec3 pos = go->getLocalPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f)) {
            go->setLocalPosition(pos);
        }

        glm::vec3 rotEuler = go->getLocalRotationEuler();
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotEuler), 0.5f)) {
            go->setLocalRotationEuler(rotEuler);
        }

        glm::vec3 scl = go->getLocalScale();
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scl), 0.1f)) {
            go->setLocalScale(scl);
        }
    }
    else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Transform updates disabled");
    }

    ImGui::Separator();
    ImGui::Text("Model");

    // ===== ALTERNATIVA SIMPLE (sin modificar FileExplorer) =====

    // Si no quieres modificar FileExplorer, puedes usar esto:
    static bool openModelPicker = false;
    static bool openMaterialPicker = false;
    static std::string selectedModelPath;

    if (ImGui::Button("Choose Model...")) {
        openModelPicker = true;
    }

    // Variable necesaria (agregar como miembro de clase o variable estática)
    ImGui::SameLine();
    // Código del popup de selección de materiales
    if (ImGui::Button("Choose Material...")) {
        openMaterialPicker = true;
    }

    if (openMaterialPicker) {
        ImGui::OpenPopup("Materials");
        openMaterialPicker = false;
    }

    if (ImGui::BeginPopup("Materials")) {
        const auto& allMaterials = MaterialManager::getInstance().getAllMaterials();

        for (const auto& [materialName, materialPtr] : allMaterials) {
            if (materialPtr && ImGui::MenuItem(materialName.c_str())) {
                go->setMaterial(MaterialManager::getInstance().getMaterial(materialName));
            }
        }

        ImGui::EndPopup();
    }

    ImGui::Text("Currents : %s", go->getModelPath().c_str());

    // Llamar directamente al FileExplorer (sin popup adicional)
    if (openModelPicker) {
        ImGui::OpenPopup("File Explorer"); // Usar el nombre que espera FileExplorer
        openModelPicker = false;
    }

    std::string modelDir = FileSystem::getProjectPath() + "\\Content";
    std::string extension = ".fbx";

    // Llamada directa - FileExplorer maneja su propio popup
    if (FileExplorer::ShowPopup(modelDir, selectedModelPath, extension)) {
        go->setModelPath(FileSystem::GetPathAfterContent(selectedModelPath));
        go->loadModelFromPath();
    }


    // Layer Configuration
    ImGui::Separator();
    ImGui::Text("Layer Configuration:");
    
    // Layer selection
    static int currentLayerIndex = 0;
    const char* layerNames[] = {
        "Layer 0", "Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Layer 6", "Layer 7", "Layer 8", "Layer 9",
        "Layer 10", "Layer 11", "Layer 12", "Layer 13", "Layer 14", "Layer 15", "Layer 16", "Layer 17", "Layer 18", "Layer 19",
        "Trigger", "Player", "Enemy", "Environment"
    };
    
    // Find current layer index
    physx::PxU32 currentLayer = go->getLayer();
    for (int i = 0; i < 24; i++) {
        physx::PxU32 layerValue = (1 << i);
        if (currentLayer == layerValue) {
            currentLayerIndex = i;
            break;
        }
    }
    
    if (ImGui::Combo("Layer", &currentLayerIndex, layerNames, 24)) {
        physx::PxU32 newLayer = (1 << currentLayerIndex);
        go->setLayer(newLayer);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Select the layer this object belongs to");
    }

    // Componentes
    ImGui::Separator();
    ImGui::Text("Components");

    // Mostrar componentes existentes
    if (go->hasGeometry()) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Mesh]");
    }

    auto material = go->getMaterial();
    if (material) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[Material]");
    }

    // Audio Source Component
    if (auto audioSource = go->getComponent<AudioSource>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNode("[Audio Source]");

        if (treeNodeOpen) {
            // Bot�n de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##AudioSource", ImVec2(30, 0))) {
                ImGui::OpenPopup("AudioSourceOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("AudioSourceOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                if (ImGui::MenuItem("Reset")) {
                    // TODO: Implementar reset de valores
                }
                if (ImGui::MenuItem("Copy Settings")) {
                    // TODO: Implementar copia de configuraci�n
                }
                ImGui::EndPopup();
            }

            if (!removeComponent && audioSource != nullptr) {
                // Path del sonido
                static char soundPath[256] = "";
                if (ImGui::InputText("Sound Path", soundPath, sizeof(soundPath))) {
                    if (strlen(soundPath) > 0) {
                        audioSource->setSound(soundPath, audioSource->is3DEnabled());
                    }
                }

                auto result = UIBuilder::Drag_Objetive("AudioClass");
                if (result.has_value()) {
                    strncpy_s(soundPath, sizeof(soundPath), result.value().c_str(), _TRUNCATE);

                    audioSource->setSound(soundPath, audioSource->is3DEnabled());
                }


                // Control de volumen
                float volume = audioSource->getVolume();
                if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
                    audioSource->setVolume(volume);
                }

                // Toggle 3D
                bool is3D = audioSource->is3DEnabled();
                if (ImGui::Checkbox("3D Sound", &is3D)) {
                    audioSource->set3DAttributes(is3D);
                }

                // Controles 3D
                if (is3D) {
                    ImGui::Separator();
                    ImGui::Text("3D Settings");

                    float minDist = audioSource->getMinDistance();
                    if (ImGui::DragFloat("Min Distance", &minDist, 0.1f, 0.1f, audioSource->getMaxDistance())) {
                        audioSource->setMinDistance(minDist);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Minimum distance before sound starts to attenuate");
                    }

                    float maxDist = audioSource->getMaxDistance();
                    if (ImGui::DragFloat("Max Distance", &maxDist, 1.0f, minDist + 0.1f, 10000.0f)) {
                        audioSource->setMaxDistance(maxDist);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Maximum distance at which the sound can be heard");
                    }
                }

                // Controles de reproducci�n
                if (ImGui::Button(audioSource->isPlaying() ? "Stop" : "Play")) {
                    if (audioSource->isPlaying()) {
                        audioSource->stop();
                    }
                    else {
                        audioSource->play();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button(audioSource->isPaused() ? "Resume" : "Pause")) {
                    if (audioSource->isPaused()) {
                        audioSource->resume();
                    }
                    else {
                        audioSource->pause();
                    }
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<AudioSource>();
        }
    }

    // Light Component
    if (auto lightComp = go->getComponent<LightComponent>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNode("[Light]");

        if (treeNodeOpen) {
            // Bot�n de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##Light", ImVec2(30, 0))) {
                ImGui::OpenPopup("LightOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("LightOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                if (ImGui::MenuItem("Reset")) {
                    // TODO: Implementar reset de valores
                }
                if (ImGui::MenuItem("Copy Settings")) {
                    // TODO: Implementar copia de configuraci�n
                }
                ImGui::EndPopup();
            }

            if (!removeComponent && lightComp != nullptr) {
                // Tipo de luz (solo mostrar, no editable)
                const char* lightType =
                    lightComp->getType() == LightType::Directional ? "Directional Light" :
                    lightComp->getType() == LightType::Point ? "Point Light" : "Spot Light";
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", lightType);
                ImGui::Separator();

                // Enabled/Disabled
                bool enabled = lightComp->isEnabled();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    lightComp->setEnabled(enabled);
                }

                // Color
                glm::vec3 color = lightComp->getColor();
                if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
                    lightComp->setColor(color);
                }

                // Intensidad
                float intensity = lightComp->getIntensity();
                if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f)) {
                    lightComp->setIntensity(intensity);
                }

                // Propiedades espec�ficas seg�n el tipo de luz
                switch (lightComp->getType()) {
                case LightType::Point: {
                    // Atenuaci�n
                    glm::vec3 attenuation = lightComp->getAttenuation();
                    bool attChanged = false;
                    attChanged |= ImGui::DragFloat("Constant", &attenuation.x, 0.01f, 0.0f, 2.0f);
                    attChanged |= ImGui::DragFloat("Linear", &attenuation.y, 0.01f, 0.0f, 2.0f);
                    attChanged |= ImGui::DragFloat("Quadratic", &attenuation.z, 0.01f, 0.0f, 2.0f);

                    if (attChanged) {
                        lightComp->setAttenuation(attenuation.x, attenuation.y, attenuation.z);
                    }

                    // Rango
                    float minDist = lightComp->getMinDistance();
                    float maxDist = lightComp->getMaxDistance();
                    bool rangeChanged = false;

                    ImGui::Separator();
                    ImGui::Text("Range Settings");

                    rangeChanged |= ImGui::DragFloat("Min Distance", &minDist, 0.1f, 0.1f, maxDist - 0.1f);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Minimum distance where the light starts to affect objects");
                    }

                    rangeChanged |= ImGui::DragFloat("Max Distance", &maxDist, 0.1f, minDist + 0.1f, 1000.0f);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Maximum distance where the light's effect becomes zero");
                    }

                    if (rangeChanged) {
                        lightComp->setRange(minDist, maxDist);
                    }
                    break;
                }
                case LightType::Spot: {
                    // �ngulos del conoah
                    float cutOffDegrees = glm::degrees(lightComp->getCutOffAngle());
                    float outerCutOffDegrees = glm::degrees(lightComp->getOuterCutOffAngle());
                    bool anglesChanged = false;

                    ImGui::Separator();
                    ImGui::Text("Cone Settings");

                    anglesChanged |= ImGui::DragFloat("Inner Angle", &cutOffDegrees, 0.5f, 0.0f, 89.0f);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Inner angle of the spotlight cone in degrees");
                    }

                    anglesChanged |= ImGui::DragFloat("Outer Angle", &outerCutOffDegrees, 0.5f, cutOffDegrees + 0.1f, 90.0f);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Outer angle of the spotlight cone in degrees");
                    }

                    if (anglesChanged) {
                        lightComp->setCutOffAngle(cutOffDegrees);
                        lightComp->setOuterCutOffAngle(outerCutOffDegrees);
                    }

                    ImGui::Separator();
                    ImGui::Text("Range Settings");

                    float spotRange = lightComp->getSpotRange();
                    if (ImGui::DragFloat("Max Range", &spotRange, 0.5f, 0.1f, 1000.0f)) {
                        lightComp->setSpotRange(spotRange);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Maximum distance where the spotlight's effect becomes zero");
                    }
                    break;
                }
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<LightComponent>();
        }
    }

    // PhysicalObject Component
    if (auto physicalObject = go->getComponent<PhysicalObject>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Physical Object]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            // Botón de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##PhysicalObject", ImVec2(30, 0))) {
                ImGui::OpenPopup("PhysicalObjectOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("PhysicalObjectOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                if (ImGui::MenuItem("Reset")) {
                    // TODO: Implementar reset de valores
                }
                if (ImGui::MenuItem("Copy Settings")) {
                    // TODO: Implementar copia de configuración
                }
                ImGui::EndPopup();
            }

            if (!removeComponent && physicalObject != nullptr) {
                // Enabled/Disabled
                bool enabled = physicalObject->isActive();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    if (enabled) {
                        physicalObject->enable();
                    }
                    else {
                        physicalObject->disable();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Body Properties");

                // Body Type
                BodyType currentType = physicalObject->getBodyType();
                const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
                int currentItem = static_cast<int>(currentType);
                if (ImGui::Combo("Body Type", &currentItem, bodyTypes, 3)) {
                    physicalObject->setBodyType(static_cast<BodyType>(currentItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Static: Immovable, Dynamic: Affected by physics, Kinematic: Movable but not affected by forces");
                }

                // Mass
                float mass = physicalObject->getMass();
                if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 1000.0f)) {
                    physicalObject->setMass(mass);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mass of the object (affects physics behavior)");
                }

                // Gravity Factor
                float gravityFactor = physicalObject->getGravityFactor();
                if (ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f, 0.0f, 10.0f)) {
                    physicalObject->setGravityFactor(gravityFactor);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Multiplier for gravity effect (0 = no gravity, 1 = normal gravity)");
                }

                ImGui::Separator();
                ImGui::Text("Physics Properties");

                // Linear Velocity
                glm::vec3 velocity = physicalObject->getVelocity();
                if (ImGui::DragFloat3("Linear Velocity", glm::value_ptr(velocity), 0.1f)) {
                    physicalObject->setVelocity(velocity);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current linear velocity of the object");
                }

                // Damping
                float damping = physicalObject->getDamping();
                if (ImGui::DragFloat("Damping", &damping, 0.01f, 0.0f, 1.0f)) {
                    physicalObject->setDamping(damping);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Damping factor for velocity (0 = no damping, 1 = full damping)");
                }

                ImGui::Separator();
                ImGui::Text("Material Properties");

                // Friction
                float friction = physicalObject->getFriction();
                if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 2.0f)) {
                    physicalObject->setFriction(friction);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Friction coefficient for contact with other objects");
                }

                // Restitution
                float restitution = physicalObject->getRestitution();
                if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
                    physicalObject->setRestitution(restitution);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Bounciness factor (0 = no bounce, 1 = perfect bounce)");
                }

                ImGui::Separator();
                ImGui::Text("Shape Properties");

                // Shape Type
                ShapeType currentShapeType = physicalObject->getShapeType();
                const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Plane" };
                int currentShapeItem = static_cast<int>(currentShapeType);
                if (ImGui::Combo("Shape Type", &currentShapeItem, shapeTypes, 4)) {
                    physicalObject->setShapeType(static_cast<ShapeType>(currentShapeItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Shape of the collision volume");
                }

                // Shape-specific properties
                switch (currentShapeType) {
                case ShapeType::Box: {
                    glm::vec3 boxExtents = physicalObject->getBoxHalfExtents();
                    if (ImGui::DragFloat3("Box Half Extents", glm::value_ptr(boxExtents), 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setBoxHalfExtents(boxExtents);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half extents of the box shape (X, Y, Z)");
                    }
                    break;
                }
                case ShapeType::Sphere: {
                    float radius = physicalObject->getSphereRadius();
                    if (ImGui::DragFloat("Sphere Radius", &radius, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setSphereRadius(radius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the sphere shape");
                    }
                    break;
                }
                case ShapeType::Capsule: {
                    float capsuleRadius = physicalObject->getCapsuleRadius();
                    float capsuleHalfHeight = physicalObject->getCapsuleHalfHeight();
                    
                    if (ImGui::DragFloat("Capsule Radius", &capsuleRadius, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setCapsuleRadius(capsuleRadius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the capsule shape");
                    }
                    
                    if (ImGui::DragFloat("Capsule Half Height", &capsuleHalfHeight, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setCapsuleHalfHeight(capsuleHalfHeight);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half height of the capsule shape");
                    }
                    break;
                }
                case ShapeType::Plane: {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Plane shape - no additional properties");
                    break;
                }
                }

                ImGui::Separator();
                ImGui::Text("Trigger Properties");

                // Is Trigger checkbox
                bool isTrigger = physicalObject->isTrigger();
                
                bool checkboxChanged = ImGui::Checkbox("Is Trigger", &isTrigger);
                
                if (checkboxChanged) {
                    physicalObject->setTrigger(isTrigger);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Trigger shapes don't cause physical collisions but can detect overlaps");
                }

                // Show trigger status
                if (isTrigger) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Trigger Active");
                }

                ImGui::Separator();
                ImGui::Text("Collision Filters");

                // Collision Group (Word0)
                static int currentCollisionGroup = 0;
                const char* collisionGroups[] = { 
                    "Static", "Dynamic", "Trigger", "Player", "Enemy", "Projectile", "Sensor" 
                };
                if (ImGui::Combo("Collision Group (Word0)", &currentCollisionGroup, collisionGroups, 7)) {
                    CollisionGroup group = static_cast<CollisionGroup>(1 << currentCollisionGroup);
                    physicalObject->setCollisionGroup(group);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word0: What type of object this is for collision detection");
                }

                // Collision Mask (Word1 - what this object can collide with)
                ImGui::Text("Collision Mask (Word1 - What this object can collide with):");
                
                static bool collisionMaskStatic = true;
                static bool collisionMaskDynamic = true;
                static bool collisionMaskTrigger = false;
                static bool collisionMaskPlayer = false;
                static bool collisionMaskEnemy = false;
                static bool collisionMaskProjectile = false;
                static bool collisionMaskSensor = false;

                if (ImGui::Checkbox("Static Objects", &collisionMaskStatic)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with static objects (walls, floors)");
                }

                if (ImGui::Checkbox("Dynamic Objects", &collisionMaskDynamic)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with dynamic objects (moving objects)");
                }

                if (ImGui::Checkbox("Triggers", &collisionMaskTrigger)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with trigger zones");
                }

                if (ImGui::Checkbox("Player", &collisionMaskPlayer)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with player objects");
                }

                if (ImGui::Checkbox("Enemy", &collisionMaskEnemy)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with enemy objects");
                }

                if (ImGui::Checkbox("Projectiles", &collisionMaskProjectile)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with projectile objects");
                }

                if (ImGui::Checkbox("Sensors", &collisionMaskSensor)) {
                    CollisionMask mask = CalculateCollisionMask(collisionMaskStatic, collisionMaskDynamic, collisionMaskTrigger, 
                                                              collisionMaskPlayer, collisionMaskEnemy, collisionMaskProjectile, collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with sensor objects");
                }

                // Collision visualization
                ImGui::Separator();
                ImGui::Text("Collision Info (Word0/Word1):");
                
                // Show current collision group (Word0)
                const char* currentGroupName = collisionGroups[currentCollisionGroup];
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Word0 (Group): %s", currentGroupName);
                
                // Show what this object can collide with (Word1)
                ImGui::Text("Word1 (Can collide with):");
                if (collisionMaskStatic) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Static Objects");
                if (collisionMaskDynamic) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Dynamic Objects");
                if (collisionMaskTrigger) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Triggers");
                if (collisionMaskPlayer) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Player");
                if (collisionMaskEnemy) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Enemy");
                if (collisionMaskProjectile) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Projectiles");
                if (collisionMaskSensor) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Sensors");

                // Collider Reference Management
                ImGui::Separator();
                ImGui::Text("Collider Reference Management");
                
                // Show collider reference status
                physx::PxShape* colliderRef = physicalObject->getColliderReference();
                if (colliderRef) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Collider Reference: SET");
                    
                    // Buttons to sync collider with reference
                    if (ImGui::Button("Sync Collider from Reference")) {
                        physicalObject->updateColliderFromReference();
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Update this object's collider to match the reference collider");
                    }
                    
                    ImGui::SameLine();
                    if (ImGui::Button("Sync Reference from Collider")) {
                        physicalObject->updateReferenceFromCollider();
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Update the reference collider to match this object's collider");
                    }
                    
                    // Show reference collider info
                    if (ImGui::TreeNode("Reference Collider Info")) {
                        physx::PxGeometryHolder refGeometry = colliderRef->getGeometry();
                        physx::PxShapeFlags refFlags = colliderRef->getFlags();
                        physx::PxFilterData refFilterData = colliderRef->getSimulationFilterData();
                        
                        ImGui::Text("Geometry Type: %d", (int)refGeometry.getType());
                        ImGui::Text("Is Trigger: %s", refFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE) ? "Yes" : "No");
                        ImGui::Text("Is Simulation Shape: %s", refFlags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE) ? "Yes" : "No");
                        ImGui::Text("Filter Data - Word0: %u, Word1: %u, Word2: %u", 
                                   refFilterData.word0, refFilterData.word1, refFilterData.word2);
                        
                        // Direct modification of reference collider
                        ImGui::Separator();
                        ImGui::Text("Modify Reference Collider:");
                        
                        // Modify geometry
                        if (ImGui::TreeNode("Modify Geometry")) {
                            static int refShapeType = 0;
                            const char* refShapeTypes[] = { "Box", "Sphere", "Capsule", "Plane" };
                            
                            if (ImGui::Combo("Reference Shape Type", &refShapeType, refShapeTypes, 4)) {
                                auto& physicsManager = PhysicsManager::getInstance();
                                if (physicsManager.getPhysics()) {
                                    physx::PxShape* newRefCollider = nullptr;
                                    
                                    switch (refShapeType) {
                                        case 0: // Box
                                            newRefCollider = physicsManager.createBoxShape(
                                                physx::PxVec3(1.0f, 1.0f, 1.0f), 
                                                physicsManager.getDefaultMaterial()
                                            );
                                            break;
                                        case 1: // Sphere
                                            newRefCollider = physicsManager.createSphereShape(1.0f, physicsManager.getDefaultMaterial());
                                            break;
                                        case 2: // Capsule
                                            newRefCollider = physicsManager.createCapsuleShape(0.5f, 1.0f, physicsManager.getDefaultMaterial());
                                            break;
                                        case 3: // Plane
                                            newRefCollider = physicsManager.createPlaneShape(physicsManager.getDefaultMaterial());
                                            break;
                                    }
                                    
                                    if (newRefCollider) {
                                        // Copy flags and filter data from old reference
                                        newRefCollider->setFlags(refFlags);
                                        newRefCollider->setSimulationFilterData(refFilterData);
                                        newRefCollider->setQueryFilterData(refFilterData);
                                        
                                        // Release old reference and set new one
                                        if (colliderRef) {
                                            colliderRef->release();
                                        }
                                        physicalObject->setColliderReference(newRefCollider);
                                        
                                        std::cout << "Changed reference collider geometry for " << go->Name << std::endl;
                                    }
                                }
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        // Modify flags
                        if (ImGui::TreeNode("Modify Flags")) {
                            bool refIsTrigger = refFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
                            bool refIsSimulation = refFlags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
                            
                            if (ImGui::Checkbox("Reference Is Trigger", &refIsTrigger)) {
                                colliderRef->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, refIsTrigger);
                                colliderRef->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !refIsTrigger);
                                std::cout << "Updated reference collider flags for " << go->Name << std::endl;
                            }
                            
                            if (ImGui::Checkbox("Reference Is Simulation Shape", &refIsSimulation)) {
                                colliderRef->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, refIsSimulation);
                                colliderRef->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, !refIsSimulation);
                                std::cout << "Updated reference collider flags for " << go->Name << std::endl;
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        // Modify filter data
                        if (ImGui::TreeNode("Modify Filter Data")) {
                            static physx::PxU32 refWord0 = refFilterData.word0;
                            static physx::PxU32 refWord1 = refFilterData.word1;
                            static physx::PxU32 refWord2 = refFilterData.word2;
                            
                            if (ImGui::DragScalar("Reference Word0 (Layer)", ImGuiDataType_U32, &refWord0, 1.0f)) {
                                refFilterData.word0 = refWord0;
                                colliderRef->setSimulationFilterData(refFilterData);
                                colliderRef->setQueryFilterData(refFilterData);
                                std::cout << "Updated reference collider Word0 for " << go->Name << std::endl;
                            }
                            
                            if (ImGui::DragScalar("Reference Word1 (Layer Mask)", ImGuiDataType_U32, &refWord1, 1.0f)) {
                                refFilterData.word1 = refWord1;
                                colliderRef->setSimulationFilterData(refFilterData);
                                colliderRef->setQueryFilterData(refFilterData);
                                std::cout << "Updated reference collider Word1 for " << go->Name << std::endl;
                            }
                            
                            if (ImGui::DragScalar("Reference Word2 (Trigger Flag)", ImGuiDataType_U32, &refWord2, 1.0f)) {
                                refFilterData.word2 = refWord2;
                                colliderRef->setSimulationFilterData(refFilterData);
                                colliderRef->setQueryFilterData(refFilterData);
                                std::cout << "Updated reference collider Word2 for " << go->Name << std::endl;
                            }
                            
                            ImGui::TreePop();
                        }
                        
                        ImGui::TreePop();
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "✗ Collider Reference: NOT SET");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No reference collider available");
                    
                    // Button to create a reference collider
                    if (ImGui::Button("Create Reference Collider")) {
                        // Create a new collider as reference
                        auto& physicsManager = PhysicsManager::getInstance();
                        if (physicsManager.getPhysics()) {
                            // Create a basic box collider as reference
                            physx::PxShape* refCollider = physicsManager.createBoxShape(
                                physx::PxVec3(1.0f, 1.0f, 1.0f), 
                                physicsManager.getDefaultMaterial()
                            );
                            
                            if (refCollider) {
                                // Configure the reference collider
                                refCollider->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
                                refCollider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                                
                                // Set basic filter data
                                physx::PxFilterData filterData;
                                filterData.word0 = LAYER_0;
                                filterData.word1 = LAYER_0 | LAYER_1 | LAYER_2 | LAYER_3 | LAYER_4 | LAYER_5 | LAYER_6 | LAYER_7 | LAYER_8 | LAYER_9 | LAYER_10 | LAYER_11 | LAYER_12 | LAYER_13 | LAYER_14 | LAYER_15 | LAYER_16 | LAYER_17 | LAYER_18 | LAYER_19 | LAYER_TRIGGER | LAYER_PLAYER | LAYER_ENEMY | LAYER_ENVIRONMENT;
                                filterData.word2 = 0x0;
                                
                                refCollider->setSimulationFilterData(filterData);
                                refCollider->setQueryFilterData(filterData);
                                
                                // Set as reference
                                physicalObject->setColliderReference(refCollider);
                                
                                std::cout << "Created reference collider for " << go->Name << std::endl;
                            }
                        }
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Create a new collider as reference for this object");
                    }
                }

                ImGui::Separator();
                ImGui::Text("Physics State");

                // Is Awake
                bool isAwake = physicalObject->isAwake();
                ImGui::Text("Is Awake: %s", isAwake ? "Yes" : "No");

                if (ImGui::Button("Wake Up")) {
                    physicalObject->wakeUp();
                }
                ImGui::SameLine();
                static glm::vec3 forceVector(0.0f, 0.0f, 0.0f);
                static bool showForceDialog = false;

                if (ImGui::Button("Add Force")) {
                    showForceDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply a force to the object");
                }

                // Force application dialog
                if (showForceDialog) {
                    ImGui::OpenPopup("Add Force");
                }

                if (ImGui::BeginPopupModal("Add Force", &showForceDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Force to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Force Vector", glm::value_ptr(forceVector), 0.1f)) {
                        // Force vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Force", ImVec2(120, 0))) {
                        physicalObject->addForce(forceVector);
                        forceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        forceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // Torque application
                static glm::vec3 torqueVector(0.0f, 0.0f, 0.0f);
                static bool showTorqueDialog = false;

                if (ImGui::Button("Add Torque")) {
                    showTorqueDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply a torque to the object");
                }

                // Torque application dialog
                if (showTorqueDialog) {
                    ImGui::OpenPopup("Add Torque");
                }

                if (ImGui::BeginPopupModal("Add Torque", &showTorqueDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Torque to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Torque Vector", glm::value_ptr(torqueVector), 0.1f)) {
                        // Torque vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Torque", ImVec2(120, 0))) {
                        physicalObject->addTorque(torqueVector);
                        torqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showTorqueDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        torqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showTorqueDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // Impulse application
                static glm::vec3 impulseVector(0.0f, 0.0f, 0.0f);
                static bool showImpulseDialog = false;

                if (ImGui::Button("Add Impulse")) {
                    showImpulseDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply an impulse to the object");
                }

                ImGui::Separator();
                ImGui::Text("Raycast Testing");

                static glm::vec3 raycastOrigin(0.0f, 0.0f, 0.0f);
                static glm::vec3 raycastDirection(0.0f, 0.0f, 1.0f);
                static float raycastDistance = 100.0f;
                static RaycastHit lastRaycastHit;

                if (ImGui::DragFloat3("Raycast Origin", glm::value_ptr(raycastOrigin), 0.1f)) {
                    // Origin updated
                }
                if (ImGui::DragFloat3("Raycast Direction", glm::value_ptr(raycastDirection), 0.1f)) {
                    // Direction updated
                }
                if (ImGui::DragFloat("Raycast Distance", &raycastDistance, 1.0f, 1.0f, 1000.0f)) {
                    // Distance updated
                }

                if (ImGui::Button("Perform Raycast")) {
                    lastRaycastHit = PhysicalObject::raycast(raycastOrigin, raycastDirection, raycastDistance);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Perform a raycast from the specified origin in the specified direction");
                }

                ImGui::SameLine();
                if (ImGui::Button("Camera Raycast")) {
                    // Get camera from active scene
                    auto& sceneManager = SceneManager::getInstance();
                    if (auto* activeScene = sceneManager.getActiveScene()) {
                                            if (auto* camera = activeScene->getCamera()) {
                        glm::vec3 cameraPos = camera->getPosition();
                        glm::vec3 cameraDir = camera->getForward();
                        lastRaycastHit = PhysicalObject::raycast(cameraPos, cameraDir, raycastDistance);
                    }
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Perform a raycast from the camera position in the camera direction");
                }

                if (lastRaycastHit.hit) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Raycast Hit!");
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", lastRaycastHit.position.x, lastRaycastHit.position.y, lastRaycastHit.position.z);
                    ImGui::Text("Normal: (%.2f, %.2f, %.2f)", lastRaycastHit.normal.x, lastRaycastHit.normal.y, lastRaycastHit.normal.z);
                    ImGui::Text("Distance: %.2f", lastRaycastHit.distance);
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "No raycast hit");
                }

                // Impulse application dialog
                if (showImpulseDialog) {
                    ImGui::OpenPopup("Add Impulse");
                }

                if (ImGui::BeginPopupModal("Add Impulse", &showImpulseDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Impulse to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Impulse Vector", glm::value_ptr(impulseVector), 0.1f)) {
                        // Impulse vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Impulse", ImVec2(120, 0))) {
                        physicalObject->addImpulse(impulseVector);
                        impulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showImpulseDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        impulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        showImpulseDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<PhysicalObject>();
        }
    }

    if (auto scriptExecutor = go->getComponent<ScriptExecutor>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Script Executor]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            // Botón de opciones
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##ScriptExecutor", ImVec2(30, 0))) {
                ImGui::OpenPopup("ScriptExecutorOptions");
            }

            if (ImGui::BeginPopup("ScriptExecutorOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                ImGui::EndPopup();
            }

            if (!removeComponent && scriptExecutor != nullptr) {
                // Campo de edición de ruta del script
                static char scriptPathBuffer[256];
                strncpy_s(scriptPathBuffer, sizeof(scriptPathBuffer), scriptExecutor->luaPath.c_str(), _TRUNCATE);
                if (ImGui::InputText("Script Path", scriptPathBuffer, sizeof(scriptPathBuffer))) {
                    scriptExecutor->luaPath = std::string(scriptPathBuffer);
                }

                // Botón para recargar script
                if (ImGui::Button("Reload Script")) {
                    scriptExecutor->reloadScript();
                }

                // Mostrar estado del script
                ImGui::Text("Script Loaded: %s", scriptExecutor->isScriptLoaded() ? "Yes" : "No");

                if (!scriptExecutor->isScriptLoaded()) {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Error: %s", scriptExecutor->getLastError().c_str());
                }
            }

            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<ScriptExecutor>();
        }
    }

    // CharacterController Component
    if (auto cc = go->getComponent<CharacterController>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Character Controller]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##CharacterController", ImVec2(30, 0))) {
                ImGui::OpenPopup("CharacterControllerOptions");
            }

            // Popup
            if (ImGui::BeginPopup("CharacterControllerOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                if (ImGui::MenuItem("Reset")) {
                    // Reset de propiedades si quieres
                }
                ImGui::EndPopup();
            }

            if (!removeComponent) {

                // Tipo de controller
                const char* types[] = { "Capsule", "Box" };
                int typeIndex = (cc->getController() && cc->getController()->getActor()->is<physx::PxRigidDynamic>())
                    ? (cc->getController()->getActor()->is<physx::PxRigidDynamic>() ? 0 : 1)
                    : 0;
                if (ImGui::Combo("Controller Type", &typeIndex, types, 2)) {
                    // Tendrías que recrear el controller si cambias tipo
                }

                // Propiedades básicas
                float height = cc->getHeight();
                if (ImGui::DragFloat("Height", &height, 0.05f, 0.5f, 3.0f)) {
                    cc->setHeight(height);
                }

                float radius = cc->getRadius();
                if (ImGui::DragFloat("Radius", &radius, 0.05f, 0.1f, 2.0f)) {
                    cc->setRadius(radius);
                }

                float step = cc->getStepOffset();
                if (ImGui::DragFloat("Step Offset", &step, 0.01f, 0.0f, 1.0f)) {
                    cc->setStepOffset(step);
                }

                float slope = cc->getSlopeLimit();
                if (ImGui::DragFloat("Slope Limit", &slope, 1.0f, 0.0f, 90.0f)) {
                    cc->setSlopeLimit(slope);
                }

                // Propiedades de movimiento
                float walk = cc->getWalkSpeed();
                if (ImGui::DragFloat("Walk Speed", &walk, 0.1f, 0.0f, 20.0f)) cc->setWalkSpeed(walk);

                float run = cc->getRunSpeed();
                if (ImGui::DragFloat("Run Speed", &run, 0.1f, 0.0f, 20.0f)) cc->setRunSpeed(run);

                float crouch = cc->getCrouchSpeed();
                if (ImGui::DragFloat("Crouch Speed", &crouch, 0.1f, 0.0f, 20.0f)) cc->setCrouchSpeed(crouch);

                float jump = cc->getJumpForce();
                if (ImGui::DragFloat("Jump Force", &jump, 0.1f, 0.0f, 20.0f)) cc->setJumpForce(jump);

                float grav = cc->getGravity();
                if (ImGui::DragFloat("Gravity", &grav, 0.1f, -30.0f, 30.0f)) cc->setGravity(grav);

                float air = cc->getAirControl();
                if (ImGui::DragFloat("Air Control", &air, 0.01f, 0.0f, 1.0f)) cc->setAirControl(air);

                ImGui::Separator();
                ImGui::Text("State");
                ImGui::Text("Grounded: %s", cc->isGroundedState() ? "Yes" : "No");
                ImGui::Text("Jumping: %s", cc->isJumpingState() ? "Yes" : "No");

                glm::vec3 vel = cc->getVelocity();
                ImGui::Text("Velocity: %.2f %.2f %.2f", vel.x, vel.y, vel.z);

                // Botones de control
                if (ImGui::Button("Force Jump")) {
                    cc->jump();
                }
                ImGui::SameLine();
                if (ImGui::Button("Teleport to Origin")) {
                    cc->teleport(glm::vec3(0, 2, 0));
                }
            }

            ImGui::TreePop();
        }

        if (removeComponent) {
            go->removeComponent<CharacterController>();
        }
    }



    if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
        ImGui::OpenPopup("ComponentMenu");
    }

    // Estilo para el men� de componentes
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("ComponentMenu")) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "AUDIO");
        ImGui::Separator();

        if (!go->getComponent<AudioSource>()) {
            if (ImGui::MenuItem("[Audio Source]", "Adds 3D audio capability")) {
                go->addComponent<AudioSource>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add audio playback capabilities to this object");
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "RENDERING");
        ImGui::Separator();



        if (!go->hasGeometry()) {
            if (ImGui::MenuItem("[Mesh]", "Adds 3D geometry")) {
                // TODO: Implementar sistema de carga de geometr�a
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add 3D geometry to this object");
            }
        }

        if (!go->getMaterial()) {
            if (ImGui::MenuItem("[Material]", "Adds material properties")) {
                // TODO: Implementar sistema de materiales
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add material properties for rendering");
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "PHYSICS");
        ImGui::Separator();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Add collision detection to this object");
        }

        if (!go->getComponent<PhysicalObject>()) {
            if (ImGui::MenuItem("[PhysicalObject]", "Add a physical object")) {
                auto physicalObject = go->addComponent<PhysicalObject>(go);
                if (physicalObject) {
                    physicalObject->initializePhysics();
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Adds both collisions and rigid objects");
            }
        }


        if (!go->getComponent<CharacterController>()) {
            if (ImGui::MenuItem("[Character Controller]", "Add a controllable character")) {
                go->addComponent<CharacterController>(go);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Adds a character controller for player or AI movement");
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "SCRIPT");
        ImGui::Separator();

        if (!go->getComponent<ScriptExecutor>()) {
            if (ImGui::MenuItem("[Script Executor]", "Adds script execution capability")) {
                go->addComponent<ScriptExecutor>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add script execution capabilities to this object");
            }
        }


        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "EFFECTS");
        ImGui::Separator();


        if (ImGui::MenuItem("[Particle System]", "Adds particle effects")) {
            // TODO: Implementar sistema de part�culas
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Add particle effects to this object");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "LIGHTING");
        ImGui::Separator();

        if (!go->getComponent<LightComponent>()) {
            if (ImGui::BeginMenu("[Light]")) {
                if (ImGui::MenuItem("Directional Light")) {
                    go->addComponent<LightComponent>(LightType::Directional);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Point Light")) {
                    go->addComponent<LightComponent>(LightType::Point);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Spot Light")) {
                    go->addComponent<LightComponent>(LightType::Spot);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Add lighting capabilities to this object");
            }
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();


}

void Inspector::RenderLightInspector(std::shared_ptr<Light> light) {
    if (!light) {
        ImGui::Text("Invalid Light.");
        return;
    }

    // Tipo de luz
    const char* lightType =
        light->getType() == LightType::Directional ? "Directional Light" :
        light->getType() == LightType::Point ? "Point Light" : "Spot Light";
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", lightType);
    ImGui::Separator();

    // Enabled/Disabled
    bool enabled = light->isEnabled();
    if (ImGui::Checkbox("Enabled", &enabled)) {
        light->setEnabled(enabled);
    }

    // Color
    glm::vec3 color = light->getColor();
    if (ImGui::ColorEdit3("Color", glm::value_ptr(color))) {
        light->setColor(color);
    }

    // Intensidad
    float intensity = light->getIntensity();
    if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f)) {
        light->setIntensity(intensity);
    }

    ImGui::Separator();

    // Propiedades espec�ficas seg�n el tipo de luz
    switch (light->getType()) {
    case LightType::Directional: {
        glm::vec3 direction = light->getDirection();
        if (ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.1f)) {
            light->setDirection(glm::normalize(direction));
        }
        break;
    }
    case LightType::Point: {
        glm::vec3 position = light->getPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
            light->setPosition(position);
        }

        // Atenuaci�n
        glm::vec3 attenuation = light->getAttenuation();
        bool attChanged = false;
        attChanged |= ImGui::DragFloat("Constant", &attenuation.x, 0.01f, 0.0f, 2.0f);
        attChanged |= ImGui::DragFloat("Linear", &attenuation.y, 0.01f, 0.0f, 2.0f);
        attChanged |= ImGui::DragFloat("Quadratic", &attenuation.z, 0.01f, 0.0f, 2.0f);

        if (attChanged) {
            light->setAttenuation(attenuation.x, attenuation.y, attenuation.z);
        }

        // Rango de la luz
        float minDist = light->getMinDistance();
        float maxDist = light->getMaxDistance();
        bool rangeChanged = false;

        ImGui::Separator();
        ImGui::Text("Range Settings");

        rangeChanged |= ImGui::DragFloat("Min Distance", &minDist, 0.1f, 0.1f, maxDist - 0.1f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Minimum distance where the light starts to affect objects");
        }

        rangeChanged |= ImGui::DragFloat("Max Distance", &maxDist, 0.1f, minDist + 0.1f, 1000.0f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum distance where the light's effect becomes zero");
        }

        if (rangeChanged) {
            light->setRange(minDist, maxDist);
        }

        break;
    }
    case LightType::Spot: {
        glm::vec3 position = light->getPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
            light->setPosition(position);
        }

        glm::vec3 direction = light->getDirection();
        if (ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.1f)) {
            light->setDirection(glm::normalize(direction));
        }

        // Convertir �ngulos a grados para la UI
        float cutOffDegrees = glm::degrees(light->getCutOffAngle());
        float outerCutOffDegrees = glm::degrees(light->getOuterCutOffAngle());
        bool anglesChanged = false;

        ImGui::Separator();
        ImGui::Text("Cone Settings");

        anglesChanged |= ImGui::DragFloat("Inner Angle", &cutOffDegrees, 0.5f, 0.0f, 89.0f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Inner angle of the spotlight cone in degrees");
        }

        anglesChanged |= ImGui::DragFloat("Outer Angle", &outerCutOffDegrees, 0.5f, cutOffDegrees + 0.1f, 90.0f);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Outer angle of the spotlight cone in degrees");
        }

        if (anglesChanged) {
            light->setCutOffAngle(cutOffDegrees);
            light->setOuterCutOffAngle(outerCutOffDegrees);
        }

        ImGui::Separator();
        ImGui::Text("Range Settings");

        float spotRange = light->getSpotRange();
        if (ImGui::DragFloat("Max Range", &spotRange, 0.5f, 0.1f, 1000.0f)) {
            light->setSpotRange(spotRange);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum distance where the spotlight's effect becomes zero");
        }

        break;
    }
    }
}