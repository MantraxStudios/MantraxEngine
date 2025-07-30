// Hierarchy.cpp

#include "Hierarchy.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "Selection.h"
#include "render/Light.h"
#include <imgui/imgui.h>

// Comprueba si `possibleParent` es ancestro de `child` para evitar ciclos.
bool Hierarchy::isChildOf(GameObject* possibleParent, GameObject* child) {
    if (!child) return false;
    auto* parent = child->getParent();
    while (parent) {
        if (parent == possibleParent)
            return true;
        parent = parent->getParent();
    }
    return false;
}

// Renderizado recursivo de GameObjects en árbol con Drag & Drop parenting.
void Hierarchy::RenderGameObjectNode(GameObject* gameObject) {
    if (!gameObject) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (Selection::GameObjectSelect == gameObject)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool hasChildren = !gameObject->getAllChildren().empty();
    bool open = ImGui::TreeNodeEx((void*)gameObject, flags | (hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf), "%s", gameObject->Name.c_str());

    // Selección por clic
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        Selection::GameObjectSelect = gameObject;
        Selection::LightSelect = nullptr;
    }

    // Menú contextual (opcional, puedes expandir)
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Select")) {
            Selection::GameObjectSelect = gameObject;
            Selection::LightSelect = nullptr;
        }
        if (ImGui::MenuItem("Deselect")) {
            Selection::GameObjectSelect = nullptr;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete")) {
            // TODO: Implementar borrado
            if (Selection::GameObjectSelect == gameObject) {
                Selection::GameObjectSelect = nullptr;
            }
        }
        ImGui::EndPopup();
    }

    // Drag & Drop: Source
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("GAMEOBJECT_DRAG", &gameObject, sizeof(GameObject*));
        ImGui::Text("Set parent: %s", gameObject->Name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drag & Drop: Target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_DRAG")) {
            GameObject* droppedObject = *(GameObject**)payload->Data;
            // Si arrastras sobre sí mismo o su padre, lo vuelves root
            if (droppedObject == gameObject || droppedObject->getParent() == gameObject) {
                droppedObject->setParent(nullptr);
            }
            // Si arrastras sobre otro objeto y no es su hijo, lo haces hijo de ese
            else if (droppedObject != gameObject && !isChildOf(droppedObject, gameObject)) {
                droppedObject->setParent(gameObject);
            }
            // else: intentas crear ciclo, no hacer nada.
        }
        ImGui::EndDragDropTarget();
    }

    // Renderizado recursivo de hijos
    if (open) {
        for (auto* child : gameObject->getAllChildren()) {
            RenderGameObjectNode(child);
        }
        ImGui::TreePop();
    }
}

void Hierarchy::OnRenderGUI() {
    if (!isOpen) return;

    ImGui::Begin("Hierarchy", &isOpen);

    // Object Service TreeNode (Jerarquía de objetos)
    if (ImGui::TreeNode("Object Service")) {
        if (ImGui::BeginPopupContextItem("ObjectServiceContext")) {
            if (ImGui::MenuItem("Create Empty Object")) {
                // TODO: Implementar creación de objeto vacío
            }
            ImGui::EndPopup();
        }

        // Obtener la escena activa
        auto* activeScene = SceneManager::getInstance().getActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
        }
        else {
            // Mostrar selección actual
            if (Selection::GameObjectSelect) {
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Selected: %s", Selection::GameObjectSelect->Name.c_str());
            }
            else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
            }
            ImGui::Separator();

            // Renderizar sólo objetos root (sin padre)
            const auto& gameObjects = activeScene->getGameObjects();
            bool anyRoot = false;
            for (GameObject* gameObject : gameObjects) {
                if (!gameObject || !gameObject->isValid()) continue;
                if (!gameObject->hasParent()) {
                    anyRoot = true;
                    RenderGameObjectNode(gameObject);
                }
            }
            if (!anyRoot) {
                ImGui::Text("No GameObjects in scene");
            }
        }
        ImGui::TreePop();
    }

    // Light Service TreeNode (igual que antes)
    if (ImGui::TreeNode("Light Service")) {
        if (ImGui::BeginPopupContextItem("LightServiceContext")) {
            auto* activeScene = SceneManager::getInstance().getActiveScene();
            if (activeScene) {
                if (ImGui::MenuItem("Add Directional Light")) {
                    auto newLight = std::make_shared<Light>(LightType::Directional);
                    newLight->setDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
                    newLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
                    newLight->setIntensity(1.0f);
                    activeScene->addLight(newLight);
                }
                if (ImGui::MenuItem("Add Point Light")) {
                    auto newLight = std::make_shared<Light>(LightType::Point);
                    newLight->setPosition(glm::vec3(0.0f, 3.0f, 0.0f));
                    newLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
                    newLight->setIntensity(1.0f);
                    newLight->setAttenuation(1.0f, 0.09f, 0.032f);
                    activeScene->addLight(newLight);
                }
                if (ImGui::MenuItem("Add Spot Light")) {
                    auto newLight = std::make_shared<Light>(LightType::Spot);
                    newLight->setPosition(glm::vec3(0.0f, 3.0f, 0.0f));
                    newLight->setDirection(glm::vec3(0.0f, -1.0f, 0.0f));
                    newLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
                    newLight->setIntensity(1.0f);
                    newLight->setCutOffAngle(12.5f);
                    newLight->setOuterCutOffAngle(17.5f);
                    activeScene->addLight(newLight);
                }
            }
            ImGui::EndPopup();
        }

        auto* activeScene = SceneManager::getInstance().getActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
        }
        else {
            const auto& lights = activeScene->getLights();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                "Total Lights: %zu", lights.size());
            if (Selection::LightSelect) {
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f),
                    "Selected: %s Light",
                    Selection::LightSelect->getType() == LightType::Directional ? "Directional" :
                    Selection::LightSelect->getType() == LightType::Point ? "Point" : "Spot");
            }
            else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No light selected");
            }
            ImGui::Separator();

            if (lights.empty()) {
                ImGui::Text("No Lights in scene");
            }
            else {
                for (size_t i = 0; i < lights.size(); i++) {
                    const auto& light = lights[i];
                    if (!light) continue;

                    ImGui::PushID(static_cast<int>(i));
                    const char* icon = "";
                    ImVec4 lightColor;
                    std::string typeName;
                    switch (light->getType()) {
                    case LightType::Directional:
                        icon = "[D]";
                        typeName = "Directional Light";
                        lightColor = ImVec4(1.0f, 0.9f, 0.5f, 1.0f);
                        break;
                    case LightType::Point:
                        icon = "[P]";
                        typeName = "Point Light";
                        lightColor = ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
                        break;
                    case LightType::Spot:
                        icon = "[S]";
                        typeName = "Spot Light";
                        lightColor = ImVec4(1.0f, 0.5f, 0.8f, 1.0f);
                        break;
                    }

                    bool isSelected = (light == Selection::LightSelect);

                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.8f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.7f, 1.0f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
                    }

                    std::string displayText = typeName;
                    if (isSelected) {
                        displayText = "> " + displayText;
                    }

                    if (ImGui::Selectable((icon + std::string(" ") + displayText).c_str(), isSelected)) {
                        Selection::LightSelect = (isSelected ? nullptr : light);
                        if (Selection::LightSelect) {
                            Selection::GameObjectSelect = nullptr;
                        }
                    }

                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Select")) {
                            Selection::LightSelect = light;
                            Selection::GameObjectSelect = nullptr;
                        }
                        if (ImGui::MenuItem("Deselect")) {
                            Selection::LightSelect = nullptr;
                        }
                        ImGui::EndPopup();
                    }

                    if (isSelected) {
                        ImGui::PopStyleColor(3);
                    }

                    ImGui::PopID();
                }
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}