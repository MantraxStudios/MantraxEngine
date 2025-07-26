#include "Inspector.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "components/LightComponent.h"
#include "render/RenderConfig.h"
#include "Selection.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>

void Inspector::OnRenderGUI() {
    ImGui::Begin("Inspector", &isOpen);

    GameObject* go = Selection::GameObjectSelect;
    std::shared_ptr<Light> light = Selection::LightSelect;

    if (go != nullptr) {
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
    if (!go) return;

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
        if (ImGui::TreeNode("[Audio Source]")) {
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
                    ImGui::SetTooltip("Distancia mínima antes de que el sonido comience a atenuarse");
                }

                float maxDist = audioSource->getMaxDistance();
                if (ImGui::DragFloat("Max Distance", &maxDist, 1.0f, minDist + 0.1f, 10000.0f)) {
                    audioSource->setMaxDistance(maxDist);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Distancia máxima a la que se puede escuchar el sonido");
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

            ImGui::TreePop();
        }
    }

    // Light Component
    if (auto lightComp = go->getComponent<LightComponent>()) {
        if (ImGui::TreeNode("[Light]")) {
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

            ImGui::TreePop();
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

        if (ImGui::MenuItem("[Collider]", "Adds collision detection")) {
            // TODO: Implementar sistema de colisiones
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Add collision detection to this object");
        }

        if (ImGui::MenuItem("[Rigidbody]", "Adds physics simulation")) {
            // TODO: Implementar sistema de física
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Add physics simulation to this object");
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
