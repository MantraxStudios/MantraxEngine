#include "AnimatorEditor.h"
#include "FileExplorer.h"
#include "FileSaveUtils.h"
#include "components/GameObject.h"
#include "components/SceneManager.h"
#include "core/Time.h"
#include "core/FileSystem.h"
#include "../EUI/UIBuilder.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>

void AnimatorEditor::OnRenderGUI() {
    if (!isOpen) return;

    ImGui::Begin("Animator Editor", &isOpen);

    // Main layout with two columns
    ImGui::Columns(2, "AnimatorEditorColumns", true);
    
    // Left column - Animator list and properties
    ImGui::BeginChild("LeftPanel", ImVec2(0, 0), true);
    
    RenderAnimatorList();
    ImGui::Separator();
    RenderAnimatorProperties();
    ImGui::Separator();
    RenderAnimationControls();
    ImGui::Separator();
    RenderExportImportSection();
    
    ImGui::EndChild();
    
    ImGui::NextColumn();
    
    // Right column - State editor and texture list
    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    
    RenderStateEditor();
    ImGui::Separator();
    RenderTextureList();
    
    ImGui::EndChild();
    
    ImGui::Columns(1);
    
    // Feedback message
    if (!feedbackMsg.empty() && feedbackTime > 0.0f) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.8f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin("Feedback", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", feedbackMsg.c_str());
        }
        ImGui::End();
        
        feedbackTime -= Time::getDeltaTime();
        if (feedbackTime <= 0.0f) {
            feedbackMsg.clear();
        }
    }

    ImGui::End();
}

void AnimatorEditor::LoadAnimators() {
    if (animatorsLoaded) return;
    
    animatorsConfigJson.clear();
    animatorNames.clear();
    
    // Create directory if it doesn't exist
    std::filesystem::create_directories(animatorsDirectory);
    
    // Load existing animator files
    for (const auto& entry : std::filesystem::directory_iterator(animatorsDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == animatorExtension) {
            std::string animatorName = entry.path().stem().string();
            animatorNames.push_back(animatorName);
        }
    }
    
    animatorsLoaded = true;
}

void AnimatorEditor::SaveAnimator(const std::string& animatorName) {
    if (!currentAnimator) {
        ShowFeedback("No animator selected", 2.0f);
        return;
    }
    
    nlohmann::json animatorData;
    
    // Save animator properties
    animatorData["name"] = animatorName;
    animatorData["animationSpeed"] = currentAnimator->animationSpeed;
    animatorData["isPlaying"] = currentAnimator->isPlaying;
    animatorData["currentFrame"] = currentAnimator->currentFrame;
    animatorData["currentState"] = currentAnimator->currentState;
    animatorData["playbackState"] = currentAnimator->playbackState;
    
    // Save states
    animatorData["states"] = nlohmann::json::array();
    for (const auto& state : currentAnimator->SpriteStates) {
        nlohmann::json stateData;
        stateData["name"] = state.state_name;
        stateData["texturePaths"] = state.texturePaths;
        animatorData["states"].push_back(stateData);
    }
    
    // Save to file
    std::string filePath = animatorsDirectory + animatorName + animatorExtension;
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << animatorData.dump(4);
        file.close();
        ShowFeedback("Animator saved successfully: " + animatorName, 2.0f);
    } else {
        ShowFeedback("Failed to save animator: " + animatorName, 2.0f);
    }
}

void AnimatorEditor::LoadAnimator(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        ShowFeedback("Failed to open animator file: " + filePath, 2.0f);
        return;
    }
    
    try {
        nlohmann::json animatorData;
        file >> animatorData;
        file.close();
        
        // Create new animator
        currentAnimator = std::make_shared<SpriteAnimator>();
        currentAnimatorName = animatorData["name"];
        
        // Load properties
        currentAnimator->animationSpeed = animatorData["animationSpeed"];
        currentAnimator->isPlaying = animatorData["isPlaying"];
        currentAnimator->currentFrame = animatorData["currentFrame"];
        currentAnimator->currentState = animatorData["currentState"];
        currentAnimator->playbackState = animatorData["playbackState"];
        
        // Load states
        currentAnimator->SpriteStates.clear();
        for (const auto& stateData : animatorData["states"]) {
            SpriteArray state;
            state.state_name = stateData["name"];
            state.texturePaths = stateData["texturePaths"].get<std::vector<std::string>>();
            currentAnimator->SpriteStates.push_back(state);
        }
        
        // Preload textures
        currentAnimator->preloadAllTextures();
        
        ShowFeedback("Animator loaded successfully: " + currentAnimatorName, 2.0f);
        
    } catch (const std::exception& e) {
        ShowFeedback("Failed to parse animator file: " + std::string(e.what()), 2.0f);
    }
}

void AnimatorEditor::CreateNewAnimator() {
    currentAnimator = std::make_shared<SpriteAnimator>();
    currentAnimatorName = "NewAnimator";
    strcpy_s(animatorNameBuffer, currentAnimatorName.c_str());
    
    // Add to list if not already present
    if (std::find(animatorNames.begin(), animatorNames.end(), currentAnimatorName) == animatorNames.end()) {
        animatorNames.push_back(currentAnimatorName);
    }
    
    ShowFeedback("New animator created", 2.0f);
}

void AnimatorEditor::DeleteAnimator(int index) {
    if (index >= 0 && index < animatorNames.size()) {
        std::string animatorName = animatorNames[index];
        std::string filePath = animatorsDirectory + animatorName + animatorExtension;
        
        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
        }
        
        animatorNames.erase(animatorNames.begin() + index);
        
        if (selectedAnimator >= animatorNames.size()) {
            selectedAnimator = std::max(0, (int)animatorNames.size() - 1);
        }
        
        ShowFeedback("Animator deleted: " + animatorName, 2.0f);
    }
}

void AnimatorEditor::ExportAnimatorToFile(const std::string& animatorName) {
    SaveAnimator(animatorName);
}

void AnimatorEditor::ImportAnimatorFromFile() {
    // This would typically open a file dialog
    // For now, we'll use a simple input
    static char importPath[256] = "";
    ImGui::InputText("Import Path", importPath, sizeof(importPath));
    if (ImGui::Button("Import")) {
        LoadAnimator(importPath);
        importPath[0] = '\0';
    }
}

void AnimatorEditor::RenderAnimatorList() {
    ImGui::Text("Animators");
    ImGui::Separator();
    
    LoadAnimators();
    
    if (ImGui::Button("Create New")) {
        CreateNewAnimator();
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Selected") && selectedAnimator >= 0 && selectedAnimator < animatorNames.size()) {
        DeleteAnimator(selectedAnimator);
    }
    
    ImGui::BeginChild("AnimatorList", ImVec2(0, 150), true);
    
    for (int i = 0; i < animatorNames.size(); i++) {
        bool isSelected = (i == selectedAnimator);
        if (ImGui::Selectable(animatorNames[i].c_str(), isSelected)) {
            SelectAnimator(i);
        }
    }
    
    ImGui::EndChild();
}

void AnimatorEditor::RenderAnimatorProperties() {
    ImGui::Text("Properties");
    ImGui::Separator();
    
    if (!currentAnimator) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No animator selected");
        return;
    }
    
    // Animator name
    if (ImGui::InputText("Name", animatorNameBuffer, sizeof(animatorNameBuffer))) {
        currentAnimatorName = animatorNameBuffer;
    }
    
    // Animation speed
    if (ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.1f, 5.0f)) {
        currentAnimator->setAnimationSpeed(animationSpeed);
    }
    
    // Current state
    if (ImGui::BeginCombo("Current State", currentAnimator->currentState.c_str())) {
        for (const auto& state : currentAnimator->SpriteStates) {
            bool isSelected = (state.state_name == currentAnimator->currentState);
            if (ImGui::Selectable(state.state_name.c_str(), isSelected)) {
                currentAnimator->setCurrentState(state.state_name);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    // Debug info
    if (ImGui::CollapsingHeader("Debug Info")) {
        ImGui::Text("States: %zu", currentAnimator->SpriteStates.size());
        ImGui::Text("Current Frame: %d", currentAnimator->getCurrentFrame());
        ImGui::Text("Is Playing: %s", currentAnimator->getIsPlaying() ? "Yes" : "No");
        ImGui::Text("Debug Info: %s", currentAnimator->getDebugInfo().c_str());
    }
}

void AnimatorEditor::RenderStateEditor() {
    ImGui::Text("States");
    ImGui::Separator();
    
    if (!currentAnimator) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No animator selected");
        return;
    }
    
    // Add new state
    ImGui::InputText("New State Name", stateNameBuffer, sizeof(stateNameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Add State")) {
        AddStateToCurrentAnimator();
    }
    
    // State list
    ImGui::BeginChild("StateList", ImVec2(0, 200), true);
    
    for (int i = 0; i < currentAnimator->SpriteStates.size(); i++) {
        const auto& state = currentAnimator->SpriteStates[i];
        bool isSelected = (i == selectedState);
        
        if (ImGui::Selectable(state.state_name.c_str(), isSelected)) {
            selectedState = i;
        }
        
        ImGui::SameLine();
        ImGui::Text("(%zu textures)", state.texturePaths.size());
        
        if (isSelected) {
            ImGui::SameLine();
            if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
                RemoveStateFromCurrentAnimator(i);
            }
        }
    }
    
    ImGui::EndChild();
}

void AnimatorEditor::RenderTextureList() {
    ImGui::Text("Textures");
    ImGui::Separator();
    
    if (!currentAnimator || selectedState < 0 || selectedState >= currentAnimator->SpriteStates.size()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No state selected");
        return;
    }
    
    const auto& state = currentAnimator->SpriteStates[selectedState];
    
    // Add texture with UIBuilder drag and drop
    ImGui::InputText("Texture Path", texturePathBuffer, sizeof(texturePathBuffer));
    
    // Add drag-and-drop functionality using UIBuilder
    static std::string lastDroppedTexture = "";
    auto textureResult = UIBuilder::Drag_Objetive("TextureClass");
    if (textureResult.has_value()) {
        std::string droppedTexturePath = textureResult.value();
        
        // Only process if this is a new texture (not the same as last frame)
        if (droppedTexturePath != lastDroppedTexture) {
            std::cout << "DEBUG: Texture dropped in AnimatorEditor! Path: " << droppedTexturePath << std::endl;

            // Clean up the path using FileSystem::GetPathAfterContent
            std::string cleanedPath = FileSystem::GetPathAfterContent(droppedTexturePath);
            if (cleanedPath.empty()) {
                // If GetPathAfterContent returns empty, use the original path
                cleanedPath = droppedTexturePath;
            }

            std::cout << "DEBUG: Cleaned path: " << cleanedPath << std::endl;

            // Update the input field with the dropped texture path
            strncpy_s(texturePathBuffer, sizeof(texturePathBuffer), cleanedPath.c_str(), _TRUNCATE);

            // Add texture to current state
            currentAnimator->addTextureToState(state.state_name, cleanedPath);
            currentAnimator->loadTexture(cleanedPath);
            
            std::cout << "DEBUG: Texture added to state successfully" << std::endl;
            ShowFeedback("Texture added: " + cleanedPath, 2.0f);
            
            // Store this texture as the last dropped one
            lastDroppedTexture = droppedTexturePath;
        }
    } else {
        // Reset the last dropped texture when no texture is being dragged
        lastDroppedTexture = "";
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Add Texture")) {
        AddTextureToState(state.state_name);
    }
    
    // Texture list
    ImGui::BeginChild("TextureList", ImVec2(0, 0), true);
    
    for (int i = 0; i < state.texturePaths.size(); i++) {
        const std::string& texturePath = state.texturePaths[i];
        bool isSelected = (i == selectedTexture);
        
        // Get texture preview
        ImTextureID preview = GetTexturePreview(texturePath.c_str());
        
        if (preview) {
            ImGui::Image(preview, ImVec2(32, 32));
            ImGui::SameLine();
        }
        
        if (ImGui::Selectable(texturePath.c_str(), isSelected)) {
            selectedTexture = i;
        }
        
        if (isSelected) {
            ImGui::SameLine();
            if (ImGui::Button(("Remove##tex" + std::to_string(i)).c_str())) {
                RemoveTextureFromState(state.state_name, i);
            }
        }
    }
    
    ImGui::EndChild();
}

void AnimatorEditor::RenderAnimationControls() {
    ImGui::Text("Animation Controls");
    ImGui::Separator();
    
    if (!currentAnimator) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No animator selected");
        return;
    }
    
    if (ImGui::Button(currentAnimator->getIsPlaying() ? "Pause" : "Play")) {
        if (currentAnimator->getIsPlaying()) {
            currentAnimator->pauseAnimation();
        } else {
            currentAnimator->playAnimation();
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        currentAnimator->stopAnimation();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        currentAnimator->setCurrentFrame(0);
    }
    
    // Frame slider
    int maxFrames = 0;
    if (!currentAnimator->SpriteStates.empty() && !currentAnimator->currentState.empty()) {
        for (const auto& state : currentAnimator->SpriteStates) {
            if (state.state_name == currentAnimator->currentState) {
                maxFrames = state.texturePaths.size();
                break;
            }
        }
    }
    
    if (maxFrames > 0) {
        int frame = currentAnimator->getCurrentFrame();
        if (ImGui::SliderInt("Frame", &frame, 0, maxFrames - 1)) {
            currentAnimator->setCurrentFrame(frame);
        }
    }
}

void AnimatorEditor::RenderExportImportSection() {
    ImGui::Text("Export/Import");
    ImGui::Separator();
    
    if (!currentAnimator) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No animator selected");
        return;
    }
    
    if (ImGui::Button("Save Animator")) {
        SaveAnimator(currentAnimatorName);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Export to File")) {
        ExportAnimatorToFile(currentAnimatorName);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Import from File")) {
        ImportAnimatorFromFile();
    }
}

void AnimatorEditor::SelectAnimator(int index) {
    if (index >= 0 && index < animatorNames.size()) {
        selectedAnimator = index;
        std::string animatorName = animatorNames[index];
        std::string filePath = animatorsDirectory + animatorName + animatorExtension;
        LoadAnimator(filePath);
    }
}

void AnimatorEditor::AddStateToCurrentAnimator() {
    if (!currentAnimator) return;
    
    std::string stateName = stateNameBuffer;
    if (!stateName.empty()) {
        currentAnimator->addSpriteState(stateName);
        stateNameBuffer[0] = '\0';
        ShowFeedback("State added: " + stateName, 2.0f);
    }
}

void AnimatorEditor::RemoveStateFromCurrentAnimator(int stateIndex) {
    if (!currentAnimator || stateIndex < 0 || stateIndex >= currentAnimator->SpriteStates.size()) return;
    
    std::string stateName = currentAnimator->SpriteStates[stateIndex].state_name;
    currentAnimator->SpriteStates.erase(currentAnimator->SpriteStates.begin() + stateIndex);
    
    if (selectedState >= currentAnimator->SpriteStates.size()) {
        selectedState = std::max(0, (int)currentAnimator->SpriteStates.size() - 1);
    }
    
    ShowFeedback("State removed: " + stateName, 2.0f);
}

void AnimatorEditor::AddTextureToState(const std::string& stateName) {
    if (!currentAnimator) return;
    
    std::string texturePath = texturePathBuffer;
    if (!texturePath.empty()) {
        currentAnimator->addTextureToState(stateName, texturePath);
        texturePathBuffer[0] = '\0';
        ShowFeedback("Texture added to state: " + stateName, 2.0f);
    }
}

void AnimatorEditor::RemoveTextureFromState(const std::string& stateName, int textureIndex) {
    if (!currentAnimator) return;
    
    for (auto& state : currentAnimator->SpriteStates) {
        if (state.state_name == stateName && textureIndex >= 0 && textureIndex < state.texturePaths.size()) {
            state.texturePaths.erase(state.texturePaths.begin() + textureIndex);
            ShowFeedback("Texture removed from state: " + stateName, 2.0f);
            break;
        }
    }
}

void AnimatorEditor::ShowFeedback(const std::string& message, float duration) {
    feedbackMsg = message;
    feedbackTime = duration;
}

void AnimatorEditor::RenderTextureDragDrop(const std::string& stateName) {
    // This function is no longer needed since we're using UIBuilder::Drag_Objetive
    // Keeping it empty to avoid breaking existing code
}

void AnimatorEditor::HandleTextureDrop(const std::string& stateName, const std::string& texturePath) {
    // This function is no longer needed since we're using UIBuilder::Drag_Objetive
    // Keeping it empty to avoid breaking existing code
} 