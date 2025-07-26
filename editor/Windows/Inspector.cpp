#include "Inspector.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"
#include "Selection.h"
#include <glm/gtc/type_ptr.hpp> // Para glm::value_ptr
#include <cstring> // para strncpy_s

void Inspector::OnRenderGUI() {
    ImGui::Begin("Inspector", &isOpen);

    GameObject* go = Selection::GameObjectSelect;

    if (go != nullptr)
    {
        // --- ID ---
        ImGui::Text("ID: %s", go->ObjectID.c_str());

        // --- Name ---
        char nameBuffer[128] = {};
        strncpy_s(nameBuffer, sizeof(nameBuffer), go->Name.c_str(), _TRUNCATE);
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
        {
            go->Name = std::string(nameBuffer);
        }

        // --- Tag ---
        char tagBuffer[64] = {};
        strncpy_s(tagBuffer, sizeof(tagBuffer), go->Tag.c_str(), _TRUNCATE);
        if (ImGui::InputText("Tag", tagBuffer, sizeof(tagBuffer)))
        {
            go->Tag = std::string(tagBuffer);
        }

        ImGui::Separator();
        ImGui::Text("Transform");

        // Posición
        glm::vec3 pos = go->getLocalPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f))
        {
            go->setLocalPosition(pos);
        }

        // Rotación
        glm::vec3 rotEuler = go->getLocalRotationEuler();
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotEuler), 0.5f))
        {
            go->setLocalRotationEuler(rotEuler);
        }

        // Escala
        glm::vec3 scl = go->getLocalScale();
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scl), 0.1f))
        {
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
    else
    {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}
