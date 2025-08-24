#include "Inspector.h"
#include <components/SceneManager.h>
#include <components/GameObject.h>
#include <components/LightComponent.h>
#include <components/PhysicalObject.h>
#include <components/Rigidbody.h>
#include <components/Collider.h>
#include <components/ScriptExecutor.h>
#include <components/CharacterController.h>
#include <components/SpriteAnimator.h>
#include <components/TileComponent.h>
#include <components/GameBehaviourFactory.h>
#include <render/Texture.h>
#include <render/MaterialManager.h>
#include <render/RenderConfig.h>
#include <core/PhysicsManager.h>

#include "Selection.h"
#include "../EUI/UIBuilder.h"
#include "../EUI/EditorInfo.h"
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "FileExplorer.h"
#include <core/FileSystem.h>
#include "../EUI/ComponentSerializer.h"
#include "../Windows/RenderWindows.h"

void RenderStyledSeparator()
{
    ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(66, 150, 250, 100));
    ImGui::Separator();
    ImGui::PopStyleColor();
}

void RenderSectionTitle(const char *title)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 161, 0, 255));
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();
}

void RenderStyledButton(const char *label, const ImVec2 &size = ImVec2(0, 0))
{
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(71, 74, 77, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(82, 84, 87, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(66, 150, 250, 255));

    ImGui::Button(label, size);

    ImGui::PopStyleColor(3);
}

void RenderStyledInputs()
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(41, 43, 46, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(51, 53, 56, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(66, 150, 250, 100));
}

void PopStyledInputs()
{
    ImGui::PopStyleColor(3);
}
//
void Inspector::RenderTransformSection(GameObject *go)
{
    RenderStyledSeparator();
    RenderSectionTitle("Transform");

    bool shouldUpdateTransform = go->isTransformUpdateEnabled();
    if (ImGui::Checkbox("Enable Transform Updates", &shouldUpdateTransform))
    {
        go->setTransformUpdateEnabled(shouldUpdateTransform);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Toggle automatic transform matrix updates");
    }

    if (shouldUpdateTransform)
    {
        RenderStyledInputs();

        glm::vec3 pos = go->getLocalPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.1f))
        {
            go->setLocalPosition(pos);
        }

        glm::vec3 rotEuler = go->getLocalRotationEuler();
        if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotEuler), 0.5f))
        {
            go->setLocalRotationEuler(rotEuler);
        }

        glm::vec3 scl = go->getLocalScale();
        if (ImGui::DragFloat3("Scale", glm::value_ptr(scl), 0.1f))
        {
            go->setLocalScale(scl);
        }

        PopStyledInputs();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255));
        ImGui::Text("Transform updates disabled");
        ImGui::PopStyleColor();
    }
}

void Inspector::RenderModelSection(GameObject *go)
{
    RenderStyledSeparator();
    RenderSectionTitle("Model");

    static bool openModelPicker = false;
    static bool openMaterialPicker = false;
    static std::string selectedModelPath;

    RenderStyledButton("Choose Model...", ImVec2(-1, 30));
    if (ImGui::IsItemClicked())
    {
        openModelPicker = true;
    }

    RenderStyledButton("Choose Material...", ImVec2(-1, 30));
    if (ImGui::IsItemClicked())
    {
        openMaterialPicker = true;
    }

    if (openMaterialPicker)
    {
        ImGui::OpenPopup("Materials");
        openMaterialPicker = false;
    }

    RenderStyledButton("Glyphs Editor...", ImVec2(-1, 30));
    if (ImGui::IsItemClicked())
    {
        if (Selection::GameObjectSelect != nullptr)
        {
            RenderWindows::getInstance().GetWindow<EditorNode>()->OpenNewEditor(Selection::GameObjectSelect->_GlyphsEngine);
        }
        else
        {
            ImGui::OpenPopup("No Selection");
        }
    }

    if (ImGui::BeginPopup("No Selection"))
    {
        ImGui::Text("Please select a GameObject first to open the Glyphs Editor.");
        if (ImGui::Button("OK"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Materials"))
    {
        const auto &allMaterials = MaterialManager::getInstance().getAllMaterials();

        for (const auto &[materialName, materialPtr] : allMaterials)
        {
            if (materialPtr && ImGui::MenuItem(materialName.c_str()))
            {
                // Validar el material antes de asignarlo
                if (materialPtr->isValid())
                {
                    std::cout << "Inspector: Changing material for GameObject '" << go->Name << "' to '" << materialName << "'" << std::endl;

                    // Verificar si el material tiene texturas
                    if (materialPtr->hasAlbedoTexture())
                    {
                        std::cout << "  - Material has albedo texture: " << materialPtr->getAlbedoTexture()->getFilePath() << std::endl;
                    }
                    else
                    {
                        std::cout << "  - Material has NO albedo texture - object may not be visible!" << std::endl;
                    }

                    // Asignar el material
                    go->setMaterial(materialPtr);

                    // Verificar que se asignó correctamente
                    auto assignedMaterial = go->getMaterial();
                    if (assignedMaterial && assignedMaterial->getName() == materialName)
                    {
                        std::cout << "Inspector: Material successfully assigned to GameObject" << std::endl;

                        // Mark RenderPipeline as dirty to force material refresh on next render
                        if (EditorInfo::pipeline)
                        {
                            EditorInfo::pipeline->markMaterialsDirty();
                            std::cout << "Inspector: RenderPipeline marked as dirty - material will refresh on next render" << std::endl;
                        }
                        else
                        {
                            std::cout << "Inspector: Warning - No RenderPipeline access, material refresh may be delayed" << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << "Inspector: ERROR - Material assignment failed!" << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Inspector: ERROR - Material '" << materialName << "' is not valid!" << std::endl;
                }
            }
        }

        ImGui::EndPopup();
    }

    ImGui::Text("Currents : %s", go->getModelPath().c_str());

    if (openModelPicker)
    {
        ImGui::OpenPopup("File Explorer");
        openModelPicker = false;
    }

    std::string modelDir = FileSystem::getProjectPath() + "\\Content";
    std::string extension = ".fbx";

    if (FileExplorer::ShowPopup(modelDir, selectedModelPath, extension))
    {
        go->setModelPath(FileSystem::GetPathAfterContent(selectedModelPath));
        go->loadModelFromPath();
    }
}

void Inspector::RenderLayerSection(GameObject *go)
{
    RenderStyledSeparator();
    RenderSectionTitle("Layer Configuration:");

    static int currentLayerIndex = 0;
    const char *layerNames[] = {
        "Layer 0", "Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Layer 6", "Layer 7", "Layer 8", "Layer 9",
        "Layer 10", "Layer 11", "Layer 12", "Layer 13", "Layer 14", "Layer 15", "Layer 16", "Layer 17", "Layer 18", "Layer 19",
        "Trigger", "Player", "Enemy", "Environment"};

    physx::PxU32 currentLayer = go->getLayer();
    for (int i = 0; i < 24; i++)
    {
        physx::PxU32 layerValue = (1 << i);
        if (currentLayer == layerValue)
        {
            currentLayerIndex = i;
            break;
        }
    }

    RenderStyledInputs();

    if (ImGui::Combo("Layer", &currentLayerIndex, layerNames, 24))
    {
        physx::PxU32 newLayer = (1 << currentLayerIndex);
        go->setLayer(newLayer);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Select the layer this object belongs to");
    }

    PopStyledInputs();
}

void Inspector::RenderRenderingOptions(GameObject *go)
{
    RenderStyledSeparator();
    ImGui::Text("Rendering Options");

    bool shouldRender = go->isRenderEnabled();
    if (ImGui::Checkbox("Enable Rendering", &shouldRender))
    {
        go->setRenderEnabled(shouldRender);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Toggle mesh rendering for this object");
    }
}

void Inspector::RenderComponentsSection(GameObject *go)
{
    RenderStyledSeparator();
    RenderSectionTitle("Components");

    if (go->hasGeometry())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(51, 204, 51, 255));
        ImGui::Text("[Mesh]");
        ImGui::PopStyleColor();
    }

    // auto material = go->getMaterial();
    // if (material) {
    //     ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(51, 204, 51, 255));
    //     ImGui::Text("[Material]");
    //     ImGui::PopStyleColor();

    //     // Botón de debug del material
    //     if (ImGui::Button("Debug Material")) {
    //         go->debugMaterialState();
    //     }
    //     if (ImGui::IsItemHovered()) {
    //         ImGui::SetTooltip("Debug material properties and state");
    //     }
    //     ImGui::SameLine();
    //     // Botón para debug de texturas
    //     if (ImGui::Button("Debug Textures")) {
    //         auto material = go->getMaterial();
    //         if (material) {
    //             material->debugTextureState();
    //         } else {
    //             std::cout << "Inspector: No material to debug textures for" << std::endl;
    //         }
    //     }
    //     if (ImGui::IsItemHovered()) {
    //         ImGui::SetTooltip("Debug texture loading and binding state");
    //     }
    //     ImGui::SameLine();
    //     // Botón para forzar refresh del material
    //     if (ImGui::Button("Force Material Refresh")) {
    //         if (EditorInfo::pipeline) {
    //             EditorInfo::pipeline->forceMaterialRefresh();
    //             std::cout << "Inspector: Forced material refresh requested" << std::endl;
    //         } else {
    //             std::cout << "Inspector: Warning - No RenderPipeline access for forced refresh" << std::endl;
    //         }
    //     }
    //     if (ImGui::IsItemHovered()) {
    //         ImGui::SetTooltip("Force immediate material refresh in rendering pipeline");
    //     }
    // }

    ComponentSerializer::RenderAudioSourceComponent(go);
    ComponentSerializer::RenderLightComponent(go);
    ComponentSerializer::RenderSpriteAnimatorComponent(go);
    ComponentSerializer::RenderPhysicalObjectComponent(go);
    ComponentSerializer::RenderRigidbodyComponent(go);
    ComponentSerializer::RenderColliderComponent(go);
    ComponentSerializer::RenderScriptExecutorComponent(go);
    ComponentSerializer::RenderCharacterControllerComponent(go);
}

void Inspector::OnRenderGUI()
{
    if (!isOpen)
        return;

    ImGui::Begin("Inspector", &isOpen);

    GameObject *go = Selection::GameObjectSelect;

    if (go != nullptr && go->isValid())
    {
        RenderGameObjectInspector(go);
    }
    else
    {
        ImGui::Text("No object selected.");
    }

    ImGui::End();
}

void Inspector::RenderGameObjectInspector(GameObject *go)
{
    if (!go || !go->isValid())
    {
        ImGui::Text("Invalid GameObject.");
        return;
    }

    // Object name
    char nameBuffer[128] = {};
    strncpy_s(nameBuffer, sizeof(nameBuffer), go->Name.c_str(), _TRUNCATE);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
    {
        go->Name = std::string(nameBuffer);
    }

    RenderRenderingOptions(go);
    RenderTransformSection(go);
    RenderModelSection(go);
    RenderLayerSection(go);
    RenderComponentsSection(go);
    RenderAddComponentSection(go);
}

// ============================================================================
// COMPONENT RENDERERS (to be implemented)
// ============================================================================

void Inspector::RenderAddComponentSection(GameObject *go)
{
    // Styled Add Component button
    RenderStyledButton("Add Component", ImVec2(-1, 0));
    if (ImGui::IsItemClicked())
    {
        ImGui::OpenPopup("ComponentMenu");
    }

    // Estilo para el menú de componentes
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("ComponentMenu"))
    {
        // Styled category headers
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("AUDIO");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (!go->getComponent<AudioSource>())
        {
            if (ImGui::MenuItem("[Audio Source]", "Adds 3D audio capability"))
            {
                go->addComponent<AudioSource>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add audio playback capabilities to this object");
            }
        }

        RenderStyledSeparator();

        if (!go->getComponent<TileComponent>())
        {
            if (ImGui::MenuItem("[Tile Component]", "Add and draw"))
            {
                go->addComponent<TileComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Use it to draw 2D maps more quickly and efficiently.");
            }
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("RENDERING");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (!go->hasGeometry())
        {
            if (ImGui::MenuItem("[Mesh]", "Adds 3D geometry"))
            {
                // TODO: Implementar sistema de carga de geometría
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add 3D geometry to this object");
            }
        }

        if (!go->getMaterial())
        {
            if (ImGui::MenuItem("[Material]", "Adds material properties"))
            {
                // TODO: Implementar sistema de materiales
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add material properties for rendering");
            }
        }

        if (!go->getComponent<SpriteAnimator>())
        {
            if (ImGui::MenuItem("[Sprite Animator]", "Adds sprite animation capability"))
            {
                go->addComponent<SpriteAnimator>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add sprite animation capabilities with texture support");
            }
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("PHYSICS");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (!go->getComponent<PhysicalObject>())
        {
            if (ImGui::MenuItem("[PhysicalObject]", "Add a physical object"))
            {
                auto physicalObject = go->addComponent<PhysicalObject>(go);
                if (physicalObject)
                {
                    physicalObject->initializePhysics();
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Adds both collisions and rigid objects (legacy)");
            }
        }

        if (!go->getComponent<Rigidbody>())
        {
            if (ImGui::MenuItem("[Rigidbody]", "Add physics body"))
            {
                auto rigidbody = go->addComponent<Rigidbody>(go);
                if (rigidbody)
                {
                    rigidbody->initializePhysics();

                    // Auto-attach existing collider if present
                    auto collider = go->getComponent<Collider>();
                    if (collider)
                    {
                        collider->attachToRigidbody(rigidbody);
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Adds physics simulation for dynamic/static/kinematic bodies");
            }
        }

        if (!go->getComponent<Collider>())
        {
            if (ImGui::MenuItem("[Collider]", "Add collision detection"))
            {
                auto collider = go->addComponent<Collider>(go);
                if (collider)
                {
                    collider->initializeCollider();

                    // Auto-attach to rigidbody if present
                    auto rigidbody = go->getComponent<Rigidbody>();
                    if (rigidbody)
                    {
                        collider->attachToRigidbody(rigidbody);
                    }
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Adds collision shapes and detection (Box, Sphere, Capsule, Plane)");
            }
        }

        if (!go->getComponent<CharacterController>())
        {
            if (ImGui::MenuItem("[Character Controller]", "Add a controllable character"))
            {
                go->addComponent<CharacterController>(go);
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Adds a character controller for player or AI movement");
            }
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("SCRIPT");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (!go->getComponent<ScriptExecutor>())
        {
            if (ImGui::MenuItem("[Script Executor]", "Adds script execution capability"))
            {
                go->addComponent<ScriptExecutor>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add script execution capabilities to this object");
            }
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("UTILITIES");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (ImGui::MenuItem("[Raycast Tool]", "Test raycast from this object"))
        {
            // This opens a raycast testing tool
            // TODO: Implement raycast testing window
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Test raycast functionality from this object's position");
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("EFFECTS");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (ImGui::MenuItem("[Particle System]", "Adds particle effects"))
        {
            // TODO: Implementar sistema de partículas
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Add particle effects to this object");
        }

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("UI");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        RenderStyledSeparator();

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 150, 255)); // textGray
        ImGui::Text("LIGHTING");
        ImGui::PopStyleColor();

        RenderStyledSeparator();

        if (!go->getComponent<LightComponent>())
        {
            if (ImGui::BeginMenu("[Light]"))
            {
                if (ImGui::MenuItem("Directional Light"))
                {
                    go->addComponent<LightComponent>(LightType::Directional);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Point Light"))
                {
                    go->addComponent<LightComponent>(LightType::Point);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Spot Light"))
                {
                    go->addComponent<LightComponent>(LightType::Spot);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Add lighting capabilities to this object");
            }
        }

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void Inspector::RenderLightInspector(std::shared_ptr<Light> light)
{
    if (!light)
    {
        ImGui::Text("Invalid Light.");
        return;
    }

    // Tipo de luz
    const char *lightType =
        light->getType() == LightType::Directional ? "Directional Light" : light->getType() == LightType::Point ? "Point Light"
                                                                                                                : "Spot Light";
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", lightType);
    ImGui::Separator();

    // Enabled/Disabled
    bool enabled = light->isEnabled();
    if (ImGui::Checkbox("Enabled", &enabled))
    {
        light->setEnabled(enabled);
    }

    // Color
    glm::vec3 color = light->getColor();
    if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
    {
        light->setColor(color);
    }

    // Intensidad
    float intensity = light->getIntensity();
    if (ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 10.0f))
    {
        light->setIntensity(intensity);
    }

    ImGui::Separator();

    // Propiedades específicas según el tipo de luz
    switch (light->getType())
    {
    case LightType::Directional:
    {
        glm::vec3 direction = light->getDirection();
        if (ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.1f))
        {
            light->setDirection(glm::normalize(direction));
        }
        break;
    }
    case LightType::Point:
    {
        glm::vec3 position = light->getPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f))
        {
            light->setPosition(position);
        }

        glm::vec3 attenuation = light->getAttenuation();
        bool attChanged = false;
        attChanged |= ImGui::DragFloat("Constant", &attenuation.x, 0.01f, 0.0f, 2.0f);
        attChanged |= ImGui::DragFloat("Linear", &attenuation.y, 0.01f, 0.0f, 2.0f);
        attChanged |= ImGui::DragFloat("Quadratic", &attenuation.z, 0.01f, 0.0f, 2.0f);

        if (attChanged)
        {
            light->setAttenuation(attenuation.x, attenuation.y, attenuation.z);
        }

        // Rango de la luz
        float minDist = light->getMinDistance();
        float maxDist = light->getMaxDistance();
        bool rangeChanged = false;

        ImGui::Separator();
        ImGui::Text("Range Settings");

        rangeChanged |= ImGui::DragFloat("Min Distance", &minDist, 0.1f, 0.1f, maxDist - 0.1f);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Minimum distance where the light starts to affect objects");
        }

        rangeChanged |= ImGui::DragFloat("Max Distance", &maxDist, 0.1f, minDist + 0.1f, 1000.0f);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Maximum distance where the light's effect becomes zero");
        }

        if (rangeChanged)
        {
            light->setRange(minDist, maxDist);
        }

        break;
    }
    case LightType::Spot:
    {
        glm::vec3 position = light->getPosition();
        if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f))
        {
            light->setPosition(position);
        }

        glm::vec3 direction = light->getDirection();
        if (ImGui::DragFloat3("Direction", glm::value_ptr(direction), 0.1f))
        {
            light->setDirection(glm::normalize(direction));
        }

        // Convertir ángulos a grados para la UI
        float cutOffDegrees = glm::degrees(light->getCutOffAngle());
        float outerCutOffDegrees = glm::degrees(light->getOuterCutOffAngle());
        bool anglesChanged = false;

        ImGui::Separator();
        ImGui::Text("Cone Settings");

        anglesChanged |= ImGui::DragFloat("Inner Angle", &cutOffDegrees, 0.5f, 0.0f, 89.0f);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Inner angle of the spotlight cone in degrees");
        }

        anglesChanged |= ImGui::DragFloat("Outer Angle", &outerCutOffDegrees, 0.5f, cutOffDegrees + 0.1f, 90.0f);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Outer angle of the spotlight cone in degrees");
        }

        if (anglesChanged)
        {
            light->setCutOffAngle(cutOffDegrees);
            light->setOuterCutOffAngle(outerCutOffDegrees);
        }

        ImGui::Separator();
        ImGui::Text("Range Settings");

        float spotRange = light->getSpotRange();
        if (ImGui::DragFloat("Max Range", &spotRange, 0.5f, 0.1f, 1000.0f))
        {
            light->setSpotRange(spotRange);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Maximum distance where the spotlight's effect becomes zero");
        }

        break;
    }
    }
}