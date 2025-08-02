#include "TileEditor.h"
#include "core/FileSystem.h"
#include <map>
#include <memory>
#include <cstring>
#include <iostream>
#include "../EUI/UIBuilder.h"
#include "components/EventSystem.h"
#include "../EUI/EditorInfo.h"

void TileEditor::OnRenderGUI() {
    ImGui::Begin("Tile Editor", nullptr, ImGuiWindowFlags_NoCollapse);

    // Sección para crear nuevos tiles
    ImGui::Text("Create New Tile");
    ImGui::Separator();
    
    newTileName = UIBuilder::InputText("Tile Name", newTileName);
    newTexturePath = UIBuilder::InputText("Tile Path", newTexturePath);
    
    auto result = UIBuilder::Drag_Objetive("TextureClass");
    if (result.has_value()) {
        newTexturePath = result.value();
    }

    if (ImGui::Button("Save Tile to Library")) {
        if (!newTileName.empty() && !newTexturePath.empty()) {
            TileData newTile(newTileName, newTexturePath);
            savedTiles.push_back(newTile);
            
            // Limpiar campos
            newTileName = "New Tile";
            newTexturePath = "";
            
            ImGui::OpenPopup("Tile Saved!");
        }
    }

    // Popup de confirmación
    if (ImGui::BeginPopupModal("Tile Saved!", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Tile saved successfully to library!");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Sección de configuración del Grid
    ImGui::Text("Grid Settings");
    ImGui::Separator();
    
    ImGui::Checkbox("Enable Grid Snap", &gridSnapEnabled);
    if (gridSnapEnabled) {
        ImGui::SliderFloat("Grid Size", &gridSize, 0.1f, 5.0f);
        ImGui::Text("Grid Size: %.2f", gridSize);
    }
    
    ImGui::Spacing();
    ImGui::Spacing();

    // Sección para tiles guardados
    ImGui::Text("Saved Tiles Library");
    ImGui::Separator();

    if (savedTiles.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No tiles saved yet. Create a tile above to get started.");
    } else {
        // Lista de tiles guardados
        for (int i = 0; i < savedTiles.size(); i++) {
            ImGui::PushID(i);
            
            bool isSelected = (selectedTileIndex == i);
            if (ImGui::Selectable(savedTiles[i].name.c_str(), isSelected)) {
                selectedTileIndex = i;
            }
            
            // Mostrar información del tile al hacer hover
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Texture: %s", savedTiles[i].texturePath.c_str());
                ImGui::EndTooltip();
            }
            
            ImGui::PopID();
        }

        ImGui::Spacing();

        if (selectedTileIndex >= 0 && selectedTileIndex < savedTiles.size()) {
            ImGui::Text("Selected: %s", savedTiles[selectedTileIndex].name.c_str());
            
            if (ImGui::Button("Instantiate Selected Tile")) {
                // Crear objeto con el tile seleccionado
                GameObject* newObject = new GameObject("Plane.fbx");
                newObject->Name = savedTiles[selectedTileIndex].name + "_Instance";
                
                // Aplicar grid snap si está habilitado
                glm::vec3 position = { 0.0f, 0.0f, 0.0f };
                if (gridSnapEnabled) {
                    position = applyGridSnap(position);
                }
                
                newObject->setLocalPosition(position);
                newObject->setMaterial(savedTiles[selectedTileIndex].material);
                
                SceneManager::getInstance().getActiveScene()->addGameObject(newObject);
            }
            
            if (ImGui::Button("Delete Selected Tile")) {
                savedTiles.erase(savedTiles.begin() + selectedTileIndex);
                selectedTileIndex = -1;
            }
            
            if (ImGui::Button("Clear Selection")) {
                selectedTileIndex = -1;
            }
        }

        ImGui::Spacing();
        
        if (ImGui::Button("Instantiate All Tile")) {
            for (const auto& tile : savedTiles) {
                GameObject* newObject = new GameObject("Plane.fbx");
                newObject->Name = tile.name + "_Instance";
                
                // Aplicar grid snap si está habilitado
                glm::vec3 position = { 0.0f, 0.0f, 0.0f };
                if (gridSnapEnabled) {
                    position = applyGridSnap(position);
                }
                
                newObject->setLocalPosition(position);
                newObject->setMaterial(tile.material);
                
                SceneManager::getInstance().getActiveScene()->addGameObject(newObject);
            }
        }

        if (ImGui::IsMouseClicked(0) && EditorInfo::IsHoveringScene && selectedTileIndex != -1) {
            glm::vec2 _P = EventSystem::get_mouse_position_in_viewport(glm::vec2(0.0f), glm::vec2(0.0f));

            GameObject* newObject = new GameObject("Plane.fbx");
            newObject->Name = savedTiles[selectedTileIndex].name + "_Instance";
            
            // Aplicar grid snap si está habilitado
            glm::vec3 position = { _P.x, _P.y, 0.0f };
            if (gridSnapEnabled) {
                position = applyGridSnap(position);
            }
            
            newObject->setLocalPosition(position);
            newObject->setMaterial(savedTiles[selectedTileIndex].material);

            SceneManager::getInstance().getActiveScene()->addGameObject(newObject);
        }
    }

    ImGui::End();
}

// Implementación de la función applyGridSnap
glm::vec3 TileEditor::applyGridSnap(const glm::vec3& position) const {
    if (!gridSnapEnabled) {
        return position;
    }
    
    return glm::vec3(
        std::round(position.x / gridSize) * gridSize,
        std::round(position.y / gridSize) * gridSize,
        std::round(position.z / gridSize) * gridSize
    );
}