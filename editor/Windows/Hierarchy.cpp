#include "Hierarchy.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "Selection.h"
#include "render/Light.h"

void Hierarchy::OnRenderGUI() {
    if (!isOpen) return;

    ImGui::Begin("Hierarchy", &isOpen);

    // Object Service TreeNode
    if (ImGui::TreeNode("Object Service")) {
        if (ImGui::BeginPopupContextItem("ObjectServiceContext")) {
            // Menú contextual para Object Service
            if (ImGui::MenuItem("Create Empty Object")) {
                // TODO: Implementar creación de objeto vacío
            }
            ImGui::EndPopup();
        }
        
        // Obtener la escena activa
        auto* activeScene = SceneManager::getInstance().getActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
        } else {
            const auto& gameObjects = activeScene->getGameObjects();

            // Mostrar información de selección actual
            if (Selection::GameObjectSelect) {
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Selected: %s", Selection::GameObjectSelect->Name.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No object selected");
            }
            ImGui::Separator();

            // Mostrar mensaje si no hay objetos
            if (gameObjects.empty()) {
                ImGui::Text("No GameObjects in scene");
            } else {
                for (size_t i = 0; i < gameObjects.size(); i++) {
                    GameObject* gameObject = gameObjects[i];
                    if (!gameObject || !gameObject->isValid()) continue;

                    ImGui::PushID(gameObject->ObjectID.c_str());

                    // Verificar si este objeto está seleccionado
                    bool isSelected = (gameObject == Selection::GameObjectSelect);

                    // Aplicar color diferente para objetos seleccionados
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.8f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.7f, 1.0f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
                    }

                    // Crear texto con icono para objetos seleccionados
                    std::string displayText = gameObject->Name;
                    if (isSelected) {
                        displayText = "> " + displayText;
                    }

                    if (ImGui::Selectable(displayText.c_str(), isSelected)) {
                        Selection::GameObjectSelect = (isSelected ? nullptr : gameObject);
                    }

                    // Menú contextual con clic derecho
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Select")) {
                            Selection::GameObjectSelect = gameObject;
                        }
                        if (ImGui::MenuItem("Deselect")) {
                            Selection::GameObjectSelect = nullptr;
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Delete")) {
                            // TODO: Implementar eliminación de GameObject
                            if (Selection::GameObjectSelect == gameObject) {
                                Selection::GameObjectSelect = nullptr;
                            }
                        }
                        ImGui::EndPopup();
                    }

                    // Restaurar colores si estaba seleccionado
                    if (isSelected) {
                        ImGui::PopStyleColor(3);
                    }

                    ImGui::PopID();
                }
            }
        }
        ImGui::TreePop();
    }

    // Light Service TreeNode
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

        // Obtener la escena activa
        auto* activeScene = SceneManager::getInstance().getActiveScene();
        if (!activeScene) {
            ImGui::Text("No active scene");
        } else {
            const auto& lights = activeScene->getLights();

            // Mostrar contador de luces e información de selección
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), 
                "Total Lights: %zu", lights.size());
            
            if (Selection::LightSelect) {
                ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), 
                    "Selected: %s Light", 
                    Selection::LightSelect->getType() == LightType::Directional ? "Directional" :
                    Selection::LightSelect->getType() == LightType::Point ? "Point" : "Spot");
            } else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No light selected");
            }
            ImGui::Separator();

            // Mostrar mensaje si no hay luces
            if (lights.empty()) {
                ImGui::Text("No Lights in scene");
            } else {
                // Iterar sobre todas las luces
                for (size_t i = 0; i < lights.size(); i++) {
                    const auto& light = lights[i];
                    if (!light) continue;

                    ImGui::PushID(static_cast<int>(i));

                    // Determinar el tipo de luz y su icono
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

                    // Verificar si esta luz está seleccionada
                    bool isSelected = (light == Selection::LightSelect);

                    // Aplicar estilo para luz seleccionada
                    if (isSelected) {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.8f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.7f, 1.0f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
                    }

                    // Crear texto con icono para luces seleccionadas
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

                    // Menú contextual
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

                    // Restaurar colores si estaba seleccionada
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