#include "Inspector.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
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
    // --- ID ---
    ImGui::Text("ID: %s", go->ObjectID.c_str());

    // --- Name ---
    char nameBuffer[128] = {};
    strncpy_s(nameBuffer, sizeof(nameBuffer), go->Name.c_str(), _TRUNCATE);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        go->Name = std::string(nameBuffer);
    }

    // --- Tag ---
    char tagBuffer[64] = {};
    strncpy_s(tagBuffer, sizeof(tagBuffer), go->Tag.c_str(), _TRUNCATE);
    if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer))) {
        go->Tag = std::string(tagBuffer);
    }

    ImGui::Separator();
    ImGui::Text("Transform");

    // Posición
    glm::vec3 pos = go->getLocalPosition();
    if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f)) {
        go->setLocalPosition(pos);
    }

    // Rotación
    glm::vec3 rotEuler = go->getLocalRotationEuler();
    if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotEuler), 0.5f)) {
        go->setLocalRotationEuler(rotEuler);
    }

    // Escala
    glm::vec3 scl = go->getLocalScale();
    if (ImGui::DragFloat3("Scale", glm::value_ptr(scl), 0.1f)) {
        go->setLocalScale(scl);
    }

    // Información de bounding box
    ImGui::Separator();
    ImGui::Text("Bounding Box:");
    BoundingBox worldBox = go->getWorldBoundingBox();
    ImGui::Text("Min: %.2f, %.2f, %.2f", worldBox.min.x, worldBox.min.y, worldBox.min.z);
    ImGui::Text("Max: %.2f, %.2f, %.2f", worldBox.max.x, worldBox.max.y, worldBox.max.z);
    
    // Información de geometría
    ImGui::Separator();
    ImGui::Text("Geometry:");
    if (go->hasGeometry()) {
        ImGui::Text("Has Geometry: Yes");
    } else {
        ImGui::Text("Has Geometry: No");
    }
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

            float cutOff = light->getCutOffAngle();
            if (ImGui::DragFloat("Cut Off Angle", &cutOff, 1.0f, 0.0f, 90.0f)) {
                light->setCutOffAngle(cutOff);
            }

            float outerCutOff = light->getOuterCutOffAngle();
            if (ImGui::DragFloat("Outer Cut Off", &outerCutOff, 1.0f, cutOff, 90.0f)) {
                light->setOuterCutOffAngle(outerCutOff);
            }
            break;
        }
    }
}
