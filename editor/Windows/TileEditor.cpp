#include "TileEditor.h"
#include "core/FileSystem.h"
#include <map>
#include <memory>
#include "Selection.h"
#include <imgui/ImGuizmo.h>
using namespace ImGuizmo;
#include <cstring>
#include <iostream>
#include <exception>
#include "../EUI/UIBuilder.h"
#include "components/EventSystem.h"
#include "../EUI/EditorInfo.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TileEditor::TileEditor() {
    // Constructor initialization
}

TileEditor::~TileEditor() {
    cleanupTextureCache();
}

void TileEditor::cleanupTextureCache() {
    textureCache.clear();
    // Si tienes un storage de objetos Texture, también lo puedes limpiar aquí
    // texturesStorage.clear();
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

GLuint TileEditor::getCachedTexture(const std::string& texturePath) {
    auto it = textureCache.find(texturePath);
    if (it != textureCache.end()) {
        return it->second->getID();
    }
    auto texture = std::make_shared<Texture>(texturePath);
    textureCache[texturePath] = texture;
    return texture->getID();
}

void TileEditor::OnRenderGUI() {
    ImGui::Begin("Tile Editor", nullptr, ImGuiWindowFlags_NoCollapse);

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
    
    ImGui::Spacing();
    ImGui::Text("Controls:");
    ImGui::Text("- Left Click: Place selected tile");
    ImGui::Text("- Ctrl + Left Click: Delete object under cursor");
    ImGui::Text("- Ctrl + Mouse Wheel: Change selected tile");
    ImGui::Separator();
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
        // Grid de ImageButtons para tiles guardados
        float buttonSize = 64.0f;
        float padding = 8.0f;
        float windowWidth = ImGui::GetWindowWidth() - 20.0f;
        int columns = static_cast<int>(windowWidth / (buttonSize + padding));
        if (columns < 1) columns = 1;
        
        ImGui::BeginChild("tiles_grid", ImVec2(0, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        for (int i = 0; i < savedTiles.size(); i++) {
            ImGui::PushID(i);
            
            // Obtener textura del cache para el ImageButton
            GLuint textureID = getCachedTexture(savedTiles[i].texturePath);
            
            // Crear ImageButton con la textura
            bool isSelected = (selectedTileIndex == i);
            ImVec4 tintColor = isSelected ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            
            if (textureID != 0) {
                // Asegurar que la textura esté vinculada antes de usarla
                // glBindTexture(GL_TEXTURE_2D, textureID); // Eliminado
                
                // Usar ImTextureID con cast apropiado
                ImTextureID imguiTextureID = (ImTextureID)(intptr_t)textureID;
                
                if (ImGui::ImageButton(("##" + std::to_string(i)).c_str(), imguiTextureID, 
                    ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1), 
                    ImVec4(0, 0, 0, 0), tintColor)) {
                    selectedTileIndex = i;
                    showTilePreview = true;
                    previewTimer = PREVIEW_DURATION;
                }
            } else {
                // Fallback: botón con color sólido si no se puede cargar la textura
                ImGui::PushStyleColor(ImGuiCol_Button, isSelected ? ImVec4(0.3f, 0.3f, 0.0f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                if (ImGui::Button(("##" + std::to_string(i)).c_str(), ImVec2(buttonSize, buttonSize))) {
                    selectedTileIndex = i;
                    showTilePreview = true;
                    previewTimer = PREVIEW_DURATION;
                }
                ImGui::PopStyleColor();
                
                // Mostrar icono de error en el botón
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() - buttonSize);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "!");
            }
            
            // Mostrar nombre del tile debajo del botón
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - buttonSize - padding);
            ImGui::TextWrapped(savedTiles[i].name.c_str());
            
            // Mostrar información del tile al hacer hover
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Name: %s", savedTiles[i].name.c_str());
                ImGui::Text("Texture: %s", savedTiles[i].texturePath.c_str());
                if (textureID == 0) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Texture failed to load!");
                }
                ImGui::EndTooltip();
            }
            
            // Organizar en grid
            if ((i + 1) % columns != 0) {
                ImGui::SameLine();
            }
            
            ImGui::PopID();
        }
        
        ImGui::EndChild();

        ImGui::Spacing();

        if (selectedTileIndex >= 0 && selectedTileIndex < savedTiles.size()) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Selected: %s (%d/%d)", 
                savedTiles[selectedTileIndex].name.c_str(), 
                selectedTileIndex + 1, 
                (int)savedTiles.size());
            
            // Preview de la textura seleccionada
            ImGui::Text("Texture Preview:");
            GLuint previewTextureID = getCachedTexture(savedTiles[selectedTileIndex].texturePath);
            
            if (previewTextureID != 0) {
                // Asegurar que la textura esté vinculada
                // glBindTexture(GL_TEXTURE_2D, previewTextureID); // Eliminado
                
                ImTextureID imguiPreviewTextureID = (ImTextureID)(intptr_t)previewTextureID;
                ImGui::Image(imguiPreviewTextureID, ImVec2(128, 128), 
                            ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            } else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Texture preview not available");
            }
            
            ImGui::Spacing();
            
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
            
            ImGui::SameLine();
            if (ImGui::Button("Clear Texture Cache")) {
                textureCache.clear();
            }
            
            ImGui::SameLine();
            if (ImGui::Button("Reload Textures")) {
                // Limpiar cache y recargar todas las texturas
                cleanupTextureCache();
                for (const auto& tile : savedTiles) {
                    getCachedTexture(tile.texturePath);
                }
            }
        }

        ImGui::Spacing();
        
        // Mostrar estadísticas del cache de texturas
        int validTextures = 0;
        int invalidTextures = 0;
        for (const auto& pair : textureCache) {
            if (pair.second && pair.second->getID() != 0) {
                validTextures++;
            } else {
                invalidTextures++;
            }
        }
        
        ImGui::Text("Texture Cache: %d valid, %d invalid", validTextures, invalidTextures);
        
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

        // Clic izquierdo para colocar tiles
        if (ImGui::IsMouseClicked(0) && EditorInfo::IsHoveringScene && selectedTileIndex != -1 && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
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
        
        // Ctrl + clic para borrar objetos
        if (ImGui::IsMouseClicked(0) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && EditorInfo::IsHoveringScene) {
            try {
                // Si hay un objeto seleccionado, eliminarlo directamente
        if (Selection::GameObjectSelect && Selection::GameObjectSelect->hasGeometry()) {
            GameObject* objectToDelete = Selection::GameObjectSelect;
            Selection::GameObjectSelect = nullptr; // Limpiar selección
                    SceneManager::getInstance().getActiveScene()->removeGameObject(objectToDelete);
                } else {
                    // Si no hay objeto seleccionado, usar raycast para encontrar uno
                    CastData* data = new CastData();
                    EventSystem::ViewportRenderPosition = glm::vec2(EditorInfo::RenderPositionX, EditorInfo::RenderPositionY);
                    glm::vec2 WorldPoint = EventSystem::screen_to_viewport(SceneManager::getInstance().getActiveScene()->getCamera());

                    if (EventSystem::MouseCast2D_Precise(WorldPoint, data, SceneManager::getInstance().getActiveScene()->getCamera()) && !IsOver()) {
                        if (data && data->object && data->object->hasGeometry()) {
                            SceneManager::getInstance().getActiveScene()->removeGameObject(data->object);
                        }
                    }
                    
                    delete data;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error al borrar objeto: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Error desconocido al borrar objeto" << std::endl;
            }
        }
        
        // Ctrl + rueda del ratón para cambiar tile seleccionado
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && !savedTiles.empty()) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                int previousIndex = selectedTileIndex;
                
                if (selectedTileIndex == -1) {
                    // Si no hay tile seleccionado, seleccionar el primero
                    selectedTileIndex = 0;
                } else {
                    // Cambiar al siguiente o anterior tile
                    if (wheel > 0.0f) {
                        // Rueda hacia arriba - siguiente tile
                        selectedTileIndex = (selectedTileIndex + 1) % savedTiles.size();
                    } else {
                        // Rueda hacia abajo - tile anterior
                        selectedTileIndex = (selectedTileIndex - 1 + savedTiles.size()) % savedTiles.size();
                    }
                }
                
                // Mostrar popup de preview si cambió el tile
                if (previousIndex != selectedTileIndex) {
                    showTilePreview = true;
                    previewTimer = PREVIEW_DURATION;
                }
            }
        }
    }
    
    // Mostrar popup de preview del tile seleccionado
    if (showTilePreview && selectedTileIndex >= 0 && selectedTileIndex < savedTiles.size()) {
        // Obtener posición del mouse
        ImVec2 mousePos = ImGui::GetMousePos();
        
        // Posicionar el popup arriba y a la izquierda del cursor
        ImVec2 popupPos = ImVec2(mousePos.x - 160, mousePos.y - 220);
        ImGui::SetNextWindowPos(popupPos);
        
        // Configurar el popup
        ImGui::SetNextWindowSize(ImVec2(140, 140));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 0.0f, 0.8f));
        
        if (ImGui::Begin("TilePreview", nullptr, 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {
            
            // Mostrar nombre del tile
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", 
                savedTiles[selectedTileIndex].name.c_str());
            
            // Mostrar preview de la textura
            GLuint previewTextureID = getCachedTexture(savedTiles[selectedTileIndex].texturePath);
            if (previewTextureID != 0) {
                ImTextureID imguiPreviewTextureID = (ImTextureID)(intptr_t)previewTextureID;
                ImGui::Image(imguiPreviewTextureID, ImVec2(80, 60), 
                            ImVec2(0, 0), ImVec2(1, 1), 
                            ImVec4(1, 1, 1, 1), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            } else {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No texture");
            }
            
            // Mostrar indicador de posición
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%d/%d", 
                selectedTileIndex + 1, (int)savedTiles.size());
        }
        
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
        
        // Actualizar timer
        previewTimer -= ImGui::GetIO().DeltaTime;
        if (previewTimer <= 0.0f) {
            showTilePreview = false;
        }
    }

    ImGui::End();
}