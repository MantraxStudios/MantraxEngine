#include "Inspector.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "components/LightComponent.h"
#include "components/PhysicalObject.h"
#include "render/RenderConfig.h"
#include "Selection.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <MantraxPhysics/MBody.h>
#include <MantraxPhysics/MCollision.h>

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

    // Control de actualización del transform
    bool shouldUpdateTransform = go->isTransformUpdateEnabled();
    if (ImGui::Checkbox("Enable Transform Updates", &shouldUpdateTransform)) {
        go->setTransformUpdateEnabled(shouldUpdateTransform);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle automatic transform matrix updates");
    }

    ImGui::Separator();
    ImGui::Text("Transform");

    // Solo mostrar controles de transform si está habilitada la actualización
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
    } else {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Transform updates disabled");
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
            // Botón de opciones alineado a la derecha pero dentro de la ventana
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
                    // TODO: Implementar copia de configuración
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

                // Controles de reproducción
                if (ImGui::Button(audioSource->isPlaying() ? "Stop" : "Play")) {
                    if (audioSource->isPlaying()) {
                        audioSource->stop();
                    } else {
                        audioSource->play();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button(audioSource->isPaused() ? "Resume" : "Pause")) {
                    if (audioSource->isPaused()) {
                        audioSource->resume();
                    } else {
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
            // Botón de opciones alineado a la derecha pero dentro de la ventana
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
                    // TODO: Implementar copia de configuración
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

                // Propiedades específicas según el tipo de luz
                switch (lightComp->getType()) {
                    case LightType::Point: {
                        // Atenuación
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
                        // Ángulos del cono
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

    // RigidBody Component
    if (auto rigidBody = go->getComponent<PhysicalObject>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNode("[Physical Object]");
        
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

            if (!removeComponent && rigidBody != nullptr) {
                // Enabled/Disabled
                bool enabled = rigidBody->isActive();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    if (enabled) {
                        rigidBody->enable();
                    } else {
                        rigidBody->disable();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Body Properties");

                // Body Type
                BodyType currentType = rigidBody->getBodyType();
                const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
                int currentItem = static_cast<int>(currentType);
                if (ImGui::Combo("Body Type", &currentItem, bodyTypes, 3)) {
                    rigidBody->setBodyType(static_cast<BodyType>(currentItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Static: Immovable, Dynamic: Affected by physics, Kinematic: Movable but not affected by forces");
                }

                // Mass
                float mass = rigidBody->getMass();
                if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 1000.0f)) {
                    rigidBody->setMass(mass);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mass of the object (affects physics behavior)");
                }

                // Gravity Factor
                float gravityFactor = rigidBody->getGravityFactor();
                if (ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f, 0.0f, 10.0f)) {
                    rigidBody->setGravityFactor(gravityFactor);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Multiplier for gravity effect (0 = no gravity, 1 = normal gravity)");
                }

                ImGui::Separator();
                ImGui::Text("Physics Properties");

                // Linear Velocity
                glm::vec3 linearVel = rigidBody->getLinearVelocity();
                if (ImGui::DragFloat3("Linear Velocity", glm::value_ptr(linearVel), 0.1f)) {
                    rigidBody->setLinearVelocity(linearVel);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current linear velocity of the object");
                }

                // Angular Velocity
                glm::vec3 angularVel = rigidBody->getAngularVelocity();
                if (ImGui::DragFloat3("Angular Velocity", glm::value_ptr(angularVel), 0.1f)) {
                    rigidBody->setAngularVelocity(angularVel);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current angular velocity of the object");
                }

                // Linear Damping
                float linearDamping = rigidBody->getLinearDamping();
                if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 1.0f)) {
                    rigidBody->setLinearDamping(linearDamping);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Damping factor for linear velocity (0 = no damping, 1 = full damping)");
                }

                // Angular Damping
                float angularDamping = rigidBody->getAngularDamping();
                if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 1.0f)) {
                    rigidBody->setAngularDamping(angularDamping);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Damping factor for angular velocity (0 = no damping, 1 = full damping)");
                }

                ImGui::Separator();
                ImGui::Text("Material Properties");

                // Friction
                float friction = rigidBody->getFriction();
                if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 2.0f)) {
                    rigidBody->setFriction(friction);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Friction coefficient for contact with other objects");
                }

                // Restitution
                float restitution = rigidBody->getRestitution();
                if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
                    rigidBody->setRestitution(restitution);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Bounciness factor (0 = no bounce, 1 = perfect bounce)");
                }

                ImGui::Separator();
                ImGui::Text("Collision");

                // Collision toggle checkbox
                MBody* mBody = rigidBody->getMBody();
                bool hasCollision = (mBody && mBody->AttachShaped);
                
                if (ImGui::Checkbox("Enable Collision", &hasCollision)) {
                    if (hasCollision) {
                        // Enable collision - add new MCollisionBox
                        MCollisionBox* boxCollision = new MCollisionBox();
                        boxCollision->Size = Vector3(1.0f, 1.0f, 1.0f); // Default size
                        boxCollision->contactListener = false;
                        boxCollision->isTrigger = false;
                        mBody->AttachShaped = boxCollision;
                    } else {
                        // Disable collision - remove existing collision
                        delete mBody->AttachShaped;
                        mBody->AttachShaped = nullptr;
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Enable/disable collision detection with a box shape");
                }

                // Show collision properties if collision exists
                if (hasCollision && mBody && mBody->AttachShaped) {
                    // Since MCollisionBehaviour is not polymorphic, we'll assume it's MCollisionBox
                    // as that's what we create when enabling collision
                    MCollisionBox* boxCollision = static_cast<MCollisionBox*>(mBody->AttachShaped);
                        ImGui::Separator();
                        ImGui::Text("Collision Properties");
                        
                        // Size property
                        Vector3 size = boxCollision->Size;
                        float sizeArray[3] = { size.x, size.y, size.z };
                        if (ImGui::DragFloat3("Size", sizeArray, 0.1f, 0.1f, 10.0f)) {
                            boxCollision->Size = Vector3(sizeArray[0], sizeArray[1], sizeArray[2]);
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Size of the collision box (X, Y, Z)");
                        }
                        
                        // Is Trigger property
                        bool isTrigger = boxCollision->isTrigger;
                        if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
                            boxCollision->isTrigger = isTrigger;
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Trigger colliders don't cause physical collisions but can detect overlaps");
                        }
                        
                        // Contact Listener property
                        bool contactListener = boxCollision->contactListener;
                        if (ImGui::Checkbox("Contact Listener", &contactListener)) {
                            boxCollision->contactListener = contactListener;
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Enable contact callbacks for this collision shape");
                        }
                }

                ImGui::Separator();
                ImGui::Text("Physics State");

                // Is Awake
                bool isAwake = rigidBody->isAwake();
                ImGui::Text("Is Awake: %s", isAwake ? "Yes" : "No");
                
                if (ImGui::Button("Wake Up")) {
                    rigidBody->wakeUp();
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
                        rigidBody->addForce(forceVector);
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
                        rigidBody->addTorque(torqueVector);
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
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<PhysicalObject>();
        }
    }

    // Botón de agregar componente con menú desplegable
    if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
        ImGui::OpenPopup("ComponentMenu");
    }

    // Estilo para el menú de componentes
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
                // TODO: Implementar sistema de carga de geometría
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
                go->addComponent<PhysicalObject>(go);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Adds both collisions and rigid objects");
            }
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "EFFECTS");
        ImGui::Separator();

        if (ImGui::MenuItem("[Particle System]", "Adds particle effects")) {
            // TODO: Implementar sistema de partículas
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

    // Propiedades específicas según el tipo de luz
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

            // Atenuación
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

            // Convertir ángulos a grados para la UI
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
