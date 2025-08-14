#include "ComponentSerializer.h"

// Static member definition
std::unordered_map<GameObject*, std::unique_ptr<ComponentUIState>> ComponentSerializer::objectStates;

// Helper function implementations
ComponentUIState* ComponentSerializer::getOrCreateState(GameObject* go) {
    auto it = objectStates.find(go);
    if (it == objectStates.end()) {
        objectStates[go] = std::make_unique<ComponentUIState>();
    }
    return objectStates[go].get();
}

void ComponentSerializer::cleanupObjectState(GameObject* go) {
    objectStates.erase(go);
}

void ComponentSerializer::clearAllStates() {
    objectStates.clear();
}

void ComponentSerializer::RenderAudioSourceComponent(GameObject* go) {
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
                // Sound path input using UIBuilder
                audioSource->setSound(UIBuilder::InputText("Sound", audioSource->getSoundPath()), audioSource->is3DEnabled());
                
                // Drag and drop target for audio files
                auto result = UIBuilder::Drag_Objetive("AudioClass");
                if (result.has_value()) {
                    audioSource->setSound(result.value(), audioSource->is3DEnabled());
                }

                // Volume slider using UIBuilder
                audioSource->setVolume(UIBuilder::Slider("Volumen", audioSource->getVolume(), 0.0f, 1.0f));

                // 3D sound toggle using UIBuilder
                audioSource->set3DAttributes(UIBuilder::Toggle("Spatial Sound", audioSource->is3DEnabled()));

                if (audioSource->is3DEnabled()) {
                    ImGui::Separator();
                    ImGui::Text("3D Settings+");

                    // Min distance using UIBuilder Float method
                    audioSource->setMinDistance(UIBuilder::Float("Min Distance", audioSource->getMinDistance()));
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Minimum distance before sound starts to attenuate");
                    }

                    // Max distance using UIBuilder Float method
                    audioSource->setMaxDistance(UIBuilder::Float("Max Distance", audioSource->getMaxDistance()));
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Maximum distance at which the sound can be heard");
                    }
                }

                // Controles de reproducción
                if (ImGui::Button(audioSource->isPlaying() ? "Stop" : "Play")) {
                    if (audioSource->isPlaying()) {
                        audioSource->stop();
                    }
                    else {
                        audioSource->play();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button(audioSource->isPaused() ? "Resume" : "Pause")) {
                    if (audioSource->isPaused()) {
                        audioSource->resume();
                    }
                    else {
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
}

void ComponentSerializer::RenderLightComponent(GameObject* go) {
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

                // Enabled/Disabled using UIBuilder
                lightComp->setEnabled(UIBuilder::Toggle("Enabled", lightComp->isEnabled()));

                // Color using UIBuilder Vector3
                lightComp->setColor(UIBuilder::Vector3("Color", lightComp->getColor()));

                // Intensidad using UIBuilder Float
                lightComp->setIntensity(UIBuilder::Float("Intensity", lightComp->getIntensity()));

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
}

void ComponentSerializer::RenderSpriteAnimatorComponent(GameObject* go) {
    if (auto spriteAnimator = go->getComponent<SpriteAnimator>()) {
        // Get state for this specific GameObject
        ComponentUIState* state = getOrCreateState(go);
        
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNode("[Sprite Animator]");

        if (treeNodeOpen) {
            // Botón de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##SpriteAnimator", ImVec2(30, 0))) {
                ImGui::OpenPopup("SpriteAnimatorOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("SpriteAnimatorOptions")) {
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

            if (!removeComponent && spriteAnimator != nullptr) {
                // Material section
                ImGui::Separator();
                ImGui::Text("Material Settings");

                // Material name input
                if (ImGui::InputText("Material Name", state->materialName, sizeof(state->materialName))) {
                    // Update material name if needed
                }

                if (ImGui::Button("Create Material")) {
                    spriteAnimator->createMaterial(state->materialName);
                }

                // Material properties
                if (spriteAnimator->getMaterial()) {
                    ImGui::Separator();
                    ImGui::Text("Material Properties");

                    // Albedo color
                    if (ImGui::ColorEdit3("Albedo", glm::value_ptr(state->albedoColor))) {
                        spriteAnimator->setSpriteAlbedo(state->albedoColor);
                    }

                    // Metallic
                    if (ImGui::SliderFloat("Metallic", &state->metallic, 0.0f, 1.0f)) {
                        spriteAnimator->setSpriteMetallic(state->metallic);
                    }

                    // Roughness
                    if (ImGui::SliderFloat("Roughness", &state->roughness, 0.0f, 1.0f)) {
                        spriteAnimator->setSpriteRoughness(state->roughness);
                    }

                    // Emissive
                    if (ImGui::ColorEdit3("Emissive", glm::value_ptr(state->emissiveColor))) {
                        spriteAnimator->setSpriteEmissive(state->emissiveColor);
                    }

                    // Tiling
                    if (ImGui::DragFloat2("Tiling", glm::value_ptr(state->tiling), 0.1f)) {
                        spriteAnimator->setSpriteTiling(state->tiling);
                    }
                }

                // Animator Configuration File
                ImGui::Separator();
                ImGui::Text("Animator Configuration");

                // Display current animator file path
                std::string currentAnimatorPath = spriteAnimator->getAnimatorFilePath();
                if (!currentAnimatorPath.empty()) {
                    ImGui::Text("Current File: %s", currentAnimatorPath.c_str());
                }
                else {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No animator file assigned");
                }

                // Load animator file button
                if (ImGui::Button("Load Animator File")) {
                    state->showFileDialog = true;
                }

                // File dialog for .animator files
                if (state->showFileDialog) {
                    std::string projectPath = FileSystem::getProjectPath();
                    std::string contentPath = FileSystem::combinePath(projectPath, "Content");
                    std::string animatorsPath = FileSystem::combinePath(contentPath, "Animators");

                    // Ensure the Animators directory exists
                    if (!std::filesystem::exists(animatorsPath)) {
                        std::filesystem::create_directories(animatorsPath);
                    }

                    std::string selectedFile;
                    if (FileExplorer::ShowPopup(animatorsPath, selectedFile, ".animator")) {
                        // Convert relative path to absolute path
                        std::string absolutePath = FileSystem::combinePath(FileSystem::getProjectPath(), selectedFile);

                        if (spriteAnimator->loadFromAnimatorFile(absolutePath)) {
                            ImGui::OpenPopup("AnimatorLoaded");
                        }
                        else {
                            ImGui::OpenPopup("AnimatorLoadError");
                        }
                        state->showFileDialog = false;
                    }
                }

                // Load from specific path input
                ImGui::InputText("Animator File Path", state->animatorFilePath, sizeof(state->animatorFilePath));

                // Add drag-and-drop functionality for .animator files
                auto animatorResult = UIBuilder::Drag_Objetive("AnimatorClass");
                if (animatorResult.has_value()) {
                    std::string droppedAnimatorPath = animatorResult.value();
                    // Only process if this is a new file (not the same as last frame)
                    if (droppedAnimatorPath != state->lastDroppedAnimatorFile) {
                        std::cout << "DEBUG: Animator file dropped! Path: " << droppedAnimatorPath << std::endl;

                        // Clean up the path using FileSystem::GetPathAfterContent
                        std::string cleanedPath = FileSystem::GetPathAfterContent(droppedAnimatorPath);
                        if (cleanedPath.empty()) {
                            // If GetPathAfterContent returns empty, use the original path
                            cleanedPath = droppedAnimatorPath;
                        }

                        std::cout << "DEBUG: Cleaned animator path: " << cleanedPath << std::endl;

                        // Update the input field with the dropped file path
                        strncpy_s(state->animatorFilePath, sizeof(state->animatorFilePath), cleanedPath.c_str(), _TRUNCATE);

                        // Load the animator file
                        // Construct the full path using getProjectPath + Content
                        std::string fullAnimatorPath = FileSystem::getProjectPath() + "\\Content\\" + cleanedPath;
                        std::cout << "DEBUG: Full animator path: " << fullAnimatorPath << std::endl;

                        if (spriteAnimator->loadFromAnimatorFile(fullAnimatorPath)) {
                            ImGui::OpenPopup("AnimatorLoaded");
                            std::cout << "DEBUG: Animator file loaded successfully" << std::endl;
                        }
                        else {
                            ImGui::OpenPopup("AnimatorLoadError");
                            std::cout << "DEBUG: Failed to load animator file" << std::endl;
                        }

                        state->lastDroppedAnimatorFile = droppedAnimatorPath;
                    }
                }
                else {
                    // Reset the last dropped file when no file is being dragged
                    state->lastDroppedAnimatorFile = "";
                }

                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                    if (strlen(state->animatorFilePath) > 0) {
                        std::string filePath = state->animatorFilePath;
                        // If it's a relative path, make it absolute
                        if (!std::filesystem::path(filePath).is_absolute()) {
                            filePath = FileSystem::combinePath(FileSystem::getProjectPath(), filePath);
                        }

                        if (spriteAnimator->loadFromAnimatorFile(filePath)) {
                            ImGui::OpenPopup("AnimatorLoaded");
                        }
                        else {
                            ImGui::OpenPopup("AnimatorLoadError");
                        }
                    }
                }

                // Save current configuration
                ImGui::SameLine();
                if (ImGui::Button("Save Current")) {
                    // Get the GameObject name for the default filename
                    std::string objectName = go->Name;
                    std::string defaultFileName = objectName + ".animator";

                    // Create the animators directory if it doesn't exist
                    std::string projectPath = FileSystem::getProjectPath();
                    std::string animatorsPath = projectPath + "\\Content\\Animators";
                    if (!std::filesystem::exists(animatorsPath)) {
                        std::filesystem::create_directories(animatorsPath);
                    }

                    // Save to the default location
                    std::string savePath = animatorsPath + "\\" + defaultFileName;

                    // Serialize the current animator state
                    std::string serializedData = spriteAnimator->serializeComponent();
                    std::ofstream file(savePath);
                    if (file.is_open()) {
                        file << serializedData;
                        file.close();
                        ImGui::OpenPopup("AnimatorSaved");
                    }
                    else {
                        ImGui::OpenPopup("AnimatorSaveError");
                    }
                }

                // Popup messages
                if (ImGui::BeginPopupModal("AnimatorLoaded", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Animator configuration loaded successfully!");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("AnimatorLoadError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Error loading animator configuration!");
                    ImGui::Text("Make sure the file exists and is a valid .animator file.");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("AnimatorFileNotFound", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("No animator file found at default location!");
                    ImGui::Text("Use the Animator Editor to create and save animator files.");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("AnimatorSaved", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Animator configuration saved successfully!");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("AnimatorSaveError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Error saving animator configuration!");
                    ImGui::Text("Make sure you have write permissions to the Content/Animators directory.");
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                // Preview section
                if (spriteAnimator->getMaterial()) {
                    ImGui::Separator();
                    ImGui::Text("Material Preview");

                    // Show material info
                    auto material = spriteAnimator->getMaterial();
                    ImGui::Text("Material: %s", material->getName().c_str());

                    // Show current texture info if available
                    if (!spriteAnimator->SpriteStates.empty()) {
                        auto currentStateIt = std::find_if(spriteAnimator->SpriteStates.begin(),
                            spriteAnimator->SpriteStates.end(),
                            [&](const SpriteArray& state) {
                                return state.state_name == spriteAnimator->currentState;
                            });

                        if (currentStateIt != spriteAnimator->SpriteStates.end() && !currentStateIt->texturePaths.empty()) {
                            // Get the texture from the persistent cache
                            auto texture = spriteAnimator->getTexture(currentStateIt->texturePaths[0]);
                            if (texture && texture->getID() != 0) {
                                ImGui::Text("Current Texture: %s", currentStateIt->texturePaths[0].c_str());
                                ImGui::Text("Size: %dx%d", texture->getWidth(), texture->getHeight());
                            }
                            else {
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Texture not loaded: %s", currentStateIt->texturePaths[0].c_str());
                            }
                        }
                    }
                }

                // Sprite States section
                ImGui::Separator();
                ImGui::Text("Sprite States");

                // Current state selector
                if (!spriteAnimator->SpriteStates.empty()) {
                    // Check if current state exists, if not, set to first state
                    bool currentStateExists = false;
                    for (const auto& state : spriteAnimator->SpriteStates) {
                        if (state.state_name == spriteAnimator->currentState) {
                            currentStateExists = true;
                            break;
                        }
                    }
                    if (!currentStateExists) {
                        spriteAnimator->currentState = spriteAnimator->SpriteStates[0].state_name;
                    }

                    const char* currentState = spriteAnimator->currentState.c_str();
                    if (ImGui::BeginCombo("Current State", currentState)) {
                        for (const auto& state : spriteAnimator->SpriteStates) {
                            bool isSelected = (spriteAnimator->currentState == state.state_name);
                            if (ImGui::Selectable(state.state_name.c_str(), isSelected)) {
                                spriteAnimator->currentState = state.state_name;
                            }
                            if (isSelected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                else {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No states available. Add a state first.");
                }

                // Add new state
                ImGui::InputText("New State Name", state->newStateName, sizeof(state->newStateName));
                if (ImGui::Button("Add State")) {
                    SpriteArray newState;
                    newState.state_name = state->newStateName;
                    spriteAnimator->SpriteStates.push_back(newState);
                    // Set as current state if it's the first one or if no current state is selected
                    if (spriteAnimator->currentState == "None" || spriteAnimator->currentState.empty()) {
                        spriteAnimator->currentState = state->newStateName;
                    }
                    strcpy_s(state->newStateName, "New State");
                }

                // Animation controls
                if (!spriteAnimator->SpriteStates.empty()) {
                    ImGui::Separator();
                    ImGui::Text("Animation Controls");

                    // Animation playback state selector
                    if (ImGui::InputText("Animation State", state->playbackStateName, sizeof(state->playbackStateName))) {
                        // State name input updated
                    }

                    // Play button and controls
                    ImGui::SameLine();
                    if (ImGui::Button(spriteAnimator->getIsPlaying() ? "Stop" : "Play")) {
                        if (spriteAnimator->getIsPlaying()) {
                            spriteAnimator->stopAnimation();
                        }
                        else {
                            std::string stateToPlay = strlen(state->playbackStateName) > 0 ? state->playbackStateName : spriteAnimator->currentState;
                            spriteAnimator->playAnimation(stateToPlay);
                        }
                    }

                    // Animation speed control
                    float animationSpeed = spriteAnimator->animationSpeed;
                    if (ImGui::SliderFloat("Animation Speed", &animationSpeed, 0.1f, 5.0f)) {
                        spriteAnimator->setAnimationSpeed(animationSpeed);
                    }

                    // Current frame display and control
                    int currentFrame = spriteAnimator->getCurrentFrame();
                    if (ImGui::DragInt("Current Frame", &currentFrame, 1, 0, 100)) {
                        spriteAnimator->setCurrentFrame(currentFrame);
                    }

                    // Animation status display
                    if (spriteAnimator->getIsPlaying()) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Playing: %s", spriteAnimator->getPlaybackState().c_str());
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Stopped");
                    }
                }

                // Texture drag and drop for current state
                if (!spriteAnimator->SpriteStates.empty()) {
                    ImGui::Separator();
                    ImGui::Text("Add Textures to Current State");

                    // Find current state
                    auto currentStateIt = std::find_if(spriteAnimator->SpriteStates.begin(),
                        spriteAnimator->SpriteStates.end(),
                        [&](const SpriteArray& state) {
                            return state.state_name == spriteAnimator->currentState;
                        });

                    if (currentStateIt != spriteAnimator->SpriteStates.end()) {
                        ImGui::Text("Current State: %s", spriteAnimator->currentState.c_str());

                        // Texture input field with drag-and-drop support
                        // Create the InputText field
                        if (ImGui::InputText("Texture Path", state->texturePath, sizeof(state->texturePath))) {
                            // Path input updated
                        }

                        // Add drag-and-drop functionality directly to the InputText field
                        auto textureResult = UIBuilder::Drag_Objetive("TextureClass");
                        if (textureResult.has_value()) {
                            std::string droppedTexturePath = textureResult.value();
                            std::cout << "DEBUG: Texture dropped! Path: " << droppedTexturePath << std::endl;

                            // Clean up the path using FileSystem::GetPathAfterContent
                            std::string cleanedPath = FileSystem::GetPathAfterContent(droppedTexturePath);
                            if (cleanedPath.empty()) {
                                // If GetPathAfterContent returns empty, use the original path
                                cleanedPath = droppedTexturePath;
                            }

                            std::cout << "DEBUG: Cleaned path: " << cleanedPath << std::endl;

                            // Update the input field with the dropped texture path
                            strncpy_s(state->texturePath, sizeof(state->texturePath), cleanedPath.c_str(), _TRUNCATE);

                            // Load texture and add to current state
                            currentStateIt->texturePaths.push_back(cleanedPath);
                            spriteAnimator->loadTexture(cleanedPath);
                            // Update material texture
                            spriteAnimator->updateMaterialTexture();

                            std::cout << "DEBUG: Texture added to state successfully" << std::endl;
                        }
                        else {
                            // Visual feedback: Show when drag target is active
                            ImGui::SameLine();
                            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.5f), "Drop zone");
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Add Texture")) {
                            if (strlen(state->texturePath) > 0) {
                                std::cout << "DEBUG: Manual texture addition - Path: " << state->texturePath << std::endl;

                                // Clean up the path using FileSystem::GetPathAfterContent
                                std::string cleanedPath = FileSystem::GetPathAfterContent(state->texturePath);
                                if (cleanedPath.empty()) {
                                    // If GetPathAfterContent returns empty, use the original path
                                    cleanedPath = state->texturePath;
                                }

                                std::cout << "DEBUG: Manual texture - Cleaned path: " << cleanedPath << std::endl;

                                currentStateIt->texturePaths.push_back(cleanedPath);
                                spriteAnimator->loadTexture(cleanedPath);
                                spriteAnimator->updateMaterialTexture();
                                // Clear the input field after successful addition
                                memset(state->texturePath, 0, sizeof(state->texturePath));

                                std::cout << "DEBUG: Manual texture added successfully" << std::endl;
                            }
                        }

                        // Quick texture path examples
                        ImGui::Text("Quick paths (click to copy):");
                        if (ImGui::Button("Sprites/Characters/player.png")) {
                            strncpy_s(state->texturePath, sizeof(state->texturePath), "Sprites/Characters/player.png", _TRUNCATE);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Sprites/Backgrounds/background.png")) {
                            strncpy_s(state->texturePath, sizeof(state->texturePath), "Sprites/Backgrounds/background.png", _TRUNCATE);
                        }

                        // Error popup for failed texture loading
                        if (ImGui::BeginPopupModal("TextureLoadError", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::Text("Error: Could not load texture from path:");
                            ImGui::Text("%s", state->texturePath);
                            ImGui::Separator();
                            if (ImGui::Button("OK", ImVec2(120, 0))) {
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }

                        // Display current textures in this state
                        if (!currentStateIt->texturePaths.empty()) {
                            ImGui::Text("Textures in %s:", currentStateIt->state_name.c_str());
                            for (size_t i = 0; i < currentStateIt->texturePaths.size(); ++i) {
                                const std::string& texturePath = currentStateIt->texturePaths[i];
                                auto texture = spriteAnimator->getTexture(texturePath);
                                std::string textureInfo = std::to_string(i + 1) + ": " + texturePath;
                                if (texture && texture->getID() != 0) {
                                    textureInfo += " (" + std::to_string(texture->getWidth()) +
                                        "x" + std::to_string(texture->getHeight()) + ")";
                                    ImGui::Text("  %s", textureInfo.c_str());
                                }
                                else {
                                    textureInfo += " (NOT LOADED)";
                                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "  %s", textureInfo.c_str());
                                }

                                // Remove texture button
                                ImGui::SameLine();
                                if (ImGui::Button(("X##" + std::to_string(i)).c_str())) {
                                    currentStateIt->texturePaths.erase(currentStateIt->texturePaths.begin() + i);
                                    spriteAnimator->updateMaterialTexture();
                                    break;
                                }
                            }
                        }
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No valid current state found. Please select a state.");
                    }
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<SpriteAnimator>();
        }
    }
}

void ComponentSerializer::RenderPhysicalObjectComponent(GameObject* go) {
    if (auto physicalObject = go->getComponent<PhysicalObject>()) {
        // Get state for this specific GameObject
        ComponentUIState* state = getOrCreateState(go);
        
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Physical Object]", ImGuiTreeNodeFlags_DefaultOpen);

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

            if (!removeComponent && physicalObject != nullptr) {
                // Enabled/Disabled
                bool enabled = physicalObject->isActive();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    if (enabled) {
                        physicalObject->enable();
                    }
                    else {
                        physicalObject->disable();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Body Properties");

                // Body Type
                BodyType currentType = physicalObject->getBodyType();
                const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
                int currentItem = static_cast<int>(currentType);
                if (ImGui::Combo("Body Type", &currentItem, bodyTypes, 3)) {
                    physicalObject->setBodyType(static_cast<BodyType>(currentItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Static: Immovable, Dynamic: Affected by physics, Kinematic: Movable but not affected by forces");
                }

                // Mass
                float mass = physicalObject->getMass();
                if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 1000.0f)) {
                    physicalObject->setMass(mass);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mass of the object (affects physics behavior)");
                }

                // Gravity Factor
                float gravityFactor = physicalObject->getGravityFactor();
                if (ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f, 0.0f, 10.0f)) {
                    physicalObject->setGravityFactor(gravityFactor);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Multiplier for gravity effect (0 = no gravity, 1 = normal gravity)");
                }

                ImGui::Separator();
                ImGui::Text("Physics Properties");

                // Linear Velocity
                glm::vec3 velocity = physicalObject->getVelocity();
                if (ImGui::DragFloat3("Linear Velocity", glm::value_ptr(velocity), 0.1f)) {
                    physicalObject->setVelocity(velocity);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current linear velocity of the object");
                }

                // Damping
                float damping = physicalObject->getDamping();
                if (ImGui::DragFloat("Damping", &damping, 0.01f, 0.0f, 1.0f)) {
                    physicalObject->setDamping(damping);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Damping factor for velocity (0 = no damping, 1 = full damping)");
                }

                ImGui::Separator();
                ImGui::Text("Material Properties");

                // Friction
                float friction = physicalObject->getFriction();
                if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 2.0f)) {
                    physicalObject->setFriction(friction);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Friction coefficient for contact with other objects");
                }

                // Restitution
                float restitution = physicalObject->getRestitution();
                if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
                    physicalObject->setRestitution(restitution);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Bounciness factor (0 = no bounce, 1 = perfect bounce)");
                }

                ImGui::Separator();
                ImGui::Text("Shape Properties");

                // Shape Type
                ShapeType currentShapeType = physicalObject->getShapeType();
                const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Plane" };
                int currentShapeItem = static_cast<int>(currentShapeType);
                if (ImGui::Combo("Shape Type", &currentShapeItem, shapeTypes, 4)) {
                    physicalObject->setShapeType(static_cast<ShapeType>(currentShapeItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Shape of the collision volume");
                }

                // Shape-specific properties
                switch (currentShapeType) {
                case ShapeType::Box: {
                    glm::vec3 boxExtents = physicalObject->getBoxHalfExtents();
                    if (ImGui::DragFloat3("Box Half Extents", glm::value_ptr(boxExtents), 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setBoxHalfExtents(boxExtents);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half extents of the box shape (X, Y, Z)");
                    }
                    break;
                }
                case ShapeType::Sphere: {
                    float radius = physicalObject->getSphereRadius();
                    if (ImGui::DragFloat("Sphere Radius", &radius, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setSphereRadius(radius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the sphere shape");
                    }
                    break;
                }
                case ShapeType::Capsule: {
                    float capsuleRadius = physicalObject->getCapsuleRadius();
                    float capsuleHalfHeight = physicalObject->getCapsuleHalfHeight();

                    if (ImGui::DragFloat("Capsule Radius", &capsuleRadius, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setCapsuleRadius(capsuleRadius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the capsule shape");
                    }

                    if (ImGui::DragFloat("Capsule Half Height", &capsuleHalfHeight, 0.1f, 0.1f, 10.0f)) {
                        physicalObject->setCapsuleHalfHeight(capsuleHalfHeight);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half height of the capsule shape");
                    }
                    break;
                }
                case ShapeType::Plane: {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Plane shape - no additional properties");
                    break;
                }
                }

                ImGui::Separator();
                ImGui::Text("Trigger Properties");

                // Is Trigger checkbox
                bool isTrigger = physicalObject->isTrigger();

                bool checkboxChanged = ImGui::Checkbox("Is Trigger", &isTrigger);

                if (checkboxChanged) {
                    physicalObject->setTrigger(isTrigger);
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Trigger shapes don't cause physical collisions but can detect overlaps");
                }

                // Show trigger status
                if (isTrigger) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Trigger Active");
                }

                ImGui::Separator();
                ImGui::Text("Collision Filters");

                // Collision Group (Word0)
                const char* collisionGroups[] = {
                    "Static", "Dynamic", "Trigger", "Player", "Enemy", "Projectile", "Sensor"
                };
                if (ImGui::Combo("Collision Group (Word0)", &state->currentCollisionGroup, collisionGroups, 7)) {
                    CollisionGroup group = static_cast<CollisionGroup>(1 << state->currentCollisionGroup);
                    physicalObject->setCollisionGroup(group);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word0: What type of object this is for collision detection");
                }

                // Collision Mask (Word1 - what this object can collide with)
                ImGui::Text("Collision Mask (Word1 - What this object can collide with):");

                if (ImGui::Checkbox("Static Objects", &state->collisionMaskStatic)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with static objects (walls, floors)");
                }

                if (ImGui::Checkbox("Dynamic Objects", &state->collisionMaskDynamic)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with dynamic objects (moving objects)");
                }

                if (ImGui::Checkbox("Triggers", &state->collisionMaskTrigger)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with trigger zones");
                }

                if (ImGui::Checkbox("Player", &state->collisionMaskPlayer)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with player objects");
                }

                if (ImGui::Checkbox("Enemy", &state->collisionMaskEnemy)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with enemy objects");
                }

                if (ImGui::Checkbox("Projectiles", &state->collisionMaskProjectile)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with projectile objects");
                }

                if (ImGui::Checkbox("Sensors", &state->collisionMaskSensor)) {
                    CollisionMask mask = CalculateCollisionMask(state->collisionMaskStatic, state->collisionMaskDynamic, state->collisionMaskTrigger,
                        state->collisionMaskPlayer, state->collisionMaskEnemy, state->collisionMaskProjectile, state->collisionMaskSensor);
                    physicalObject->setCollisionMask(mask);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Word1: Can collide with sensor objects");
                }

                // Collision visualization
                ImGui::Separator();
                ImGui::Text("Collision Info (Word0/Word1):");

                // Show current collision group (Word0)
                const char* currentGroupName = collisionGroups[state->currentCollisionGroup];
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Word0 (Group): %s", currentGroupName);

                // Show what this object can collide with (Word1)
                ImGui::Text("Word1 (Can collide with):");
                if (state->collisionMaskStatic) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Static Objects");
                if (state->collisionMaskDynamic) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Dynamic Objects");
                if (state->collisionMaskTrigger) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Triggers");
                if (state->collisionMaskPlayer) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Player");
                if (state->collisionMaskEnemy) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Enemy");
                if (state->collisionMaskProjectile) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Projectiles");
                if (state->collisionMaskSensor) ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), " Sensors");

                // Physics State
                ImGui::Separator();
                ImGui::Text("Physics State");

                // Is Awake
                bool isAwake = physicalObject->isAwake();
                ImGui::Text("Is Awake: %s", isAwake ? "Yes" : "No");

                if (ImGui::Button("Wake Up")) {
                    physicalObject->wakeUp();
                }
                ImGui::SameLine();

                if (ImGui::Button("Add Force")) {
                    state->showForceDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply a force to the object");
                }

                // Force application dialog
                if (state->showForceDialog) {
                    ImGui::OpenPopup("Add Force");
                }

                if (ImGui::BeginPopupModal("Add Force", &state->showForceDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Force to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Force Vector", glm::value_ptr(state->forceVector), 0.1f)) {
                        // Force vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Force", ImVec2(120, 0))) {
                        physicalObject->addForce(state->forceVector);
                        state->forceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        state->forceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // Torque application
                if (ImGui::Button("Add Torque")) {
                    state->showTorqueDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply a torque to the object");
                }

                // Torque application dialog
                if (state->showTorqueDialog) {
                    ImGui::OpenPopup("Add Torque");
                }

                if (ImGui::BeginPopupModal("Add Torque", &state->showTorqueDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Torque to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Torque Vector", glm::value_ptr(state->torqueVector), 0.1f)) {
                        // Torque vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Torque", ImVec2(120, 0))) {
                        physicalObject->addTorque(state->torqueVector);
                        state->torqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showTorqueDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        state->torqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showTorqueDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // Impulse application
                if (ImGui::Button("Add Impulse")) {
                    state->showImpulseDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply an impulse to the object");
                }

                ImGui::Separator();
                ImGui::Text("Raycast Testing");

                if (ImGui::DragFloat3("Raycast Origin", glm::value_ptr(state->raycastOrigin), 0.1f)) {
                    // Origin updated
                }
                if (ImGui::DragFloat3("Raycast Direction", glm::value_ptr(state->raycastDirection), 0.1f)) {
                    // Direction updated
                }
                if (ImGui::DragFloat("Raycast Distance", &state->raycastDistance, 1.0f, 1.0f, 1000.0f)) {
                    // Distance updated
                }

                if (ImGui::Button("Perform Raycast")) {
                    state->lastRaycastHit = PhysicalObject::raycast(state->raycastOrigin, state->raycastDirection, state->raycastDistance);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Perform a raycast from the specified origin in the specified direction");
                }

                ImGui::SameLine();
                if (ImGui::Button("Camera Raycast")) {
                    // Get camera from active scene
                    auto& sceneManager = SceneManager::getInstance();
                    if (auto* activeScene = sceneManager.getActiveScene()) {
                        if (auto* camera = activeScene->getCamera()) {
                            glm::vec3 cameraPos = camera->getPosition();
                            glm::vec3 cameraDir = camera->getForward();
                            state->lastRaycastHit = PhysicalObject::raycast(cameraPos, cameraDir, state->raycastDistance);
                        }
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Perform a raycast from the camera position in the camera direction");
                }

                if (state->lastRaycastHit.hit) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Raycast Hit!");
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", state->lastRaycastHit.position.x, state->lastRaycastHit.position.y, state->lastRaycastHit.position.z);
                    ImGui::Text("Normal: (%.2f, %.2f, %.2f)", state->lastRaycastHit.normal.x, state->lastRaycastHit.normal.y, state->lastRaycastHit.normal.z);
                    ImGui::Text("Distance: %.2f", state->lastRaycastHit.distance);
                }
                else {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "No raycast hit");
                }

                // Impulse application dialog
                if (state->showImpulseDialog) {
                    ImGui::OpenPopup("Add Impulse");
                }

                if (ImGui::BeginPopupModal("Add Impulse", &state->showImpulseDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Impulse to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Impulse Vector", glm::value_ptr(state->impulseVector), 0.1f)) {
                        // Impulse vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Impulse", ImVec2(120, 0))) {
                        physicalObject->addImpulse(state->impulseVector);
                        state->impulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showImpulseDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        state->impulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->showImpulseDialog = false;
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
}

void ComponentSerializer::RenderScriptExecutorComponent(GameObject* go) {
    if (auto scriptExecutor = go->getComponent<ScriptExecutor>()) {
        // Get state for this specific GameObject
        ComponentUIState* state = getOrCreateState(go);
        
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Script Executor]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            // Botón de opciones
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##ScriptExecutor", ImVec2(30, 0))) {
                ImGui::OpenPopup("ScriptExecutorOptions");
            }

            if (ImGui::BeginPopup("ScriptExecutorOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                ImGui::EndPopup();
            }

            if (!removeComponent && scriptExecutor != nullptr) {
                // Campo de edición de ruta del script
                strncpy_s(state->scriptPathBuffer, sizeof(state->scriptPathBuffer), scriptExecutor->luaPath.c_str(), _TRUNCATE);
                if (ImGui::InputText("Script Path", state->scriptPathBuffer, sizeof(state->scriptPathBuffer))) {
                    scriptExecutor->luaPath = std::string(state->scriptPathBuffer);
                }

                // Botón para recargar script
                if (ImGui::Button("Reload Script")) {
                    scriptExecutor->reloadScript();
                }

                // Mostrar estado del script
                ImGui::Text("Script Loaded: %s", scriptExecutor->isScriptLoaded() ? "Yes" : "No");

                if (!scriptExecutor->isScriptLoaded()) {
                    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Error: %s", scriptExecutor->getLastError().c_str());
                }
            }

            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<ScriptExecutor>();
        }
    }
}

void ComponentSerializer::RenderCharacterControllerComponent(GameObject* go) {
    if (auto cc = go->getComponent<CharacterController>()) {
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Character Controller]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##CharacterController", ImVec2(30, 0))) {
                ImGui::OpenPopup("CharacterControllerOptions");
            }

            // Popup
            if (ImGui::BeginPopup("CharacterControllerOptions")) {
                if (ImGui::MenuItem("Remove Component")) {
                    removeComponent = true;
                }
                if (ImGui::MenuItem("Reset")) {
                    // Reset de propiedades si quieres
                }
                ImGui::EndPopup();
            }

            if (!removeComponent) {
                // Tipo de controller
                const char* types[] = { "Capsule", "Box" };
                int typeIndex = (cc->getController() && cc->getController()->getActor()->is<physx::PxRigidDynamic>())
                    ? (cc->getController()->getActor()->is<physx::PxRigidDynamic>() ? 0 : 1)
                    : 0;
                if (ImGui::Combo("Controller Type", &typeIndex, types, 2)) {
                    // Tendrías que recrear el controller si cambias tipo
                }

                // Propiedades básicas
                float height = cc->getHeight();
                if (ImGui::DragFloat("Height", &height, 0.05f, 0.5f, 3.0f)) {
                    cc->setHeight(height);
                }

                float radius = cc->getRadius();
                if (ImGui::DragFloat("Radius", &radius, 0.05f, 0.1f, 2.0f)) {
                    cc->setRadius(radius);
                }

                float step = cc->getStepOffset();
                if (ImGui::DragFloat("Step Offset", &step, 0.01f, 0.0f, 1.0f)) {
                    cc->setStepOffset(step);
                }

                float slope = cc->getSlopeLimit();
                if (ImGui::DragFloat("Slope Limit", &slope, 1.0f, 0.0f, 90.0f)) {
                    cc->setSlopeLimit(slope);
                }

                // Propiedades de movimiento
                float walk = cc->getWalkSpeed();
                if (ImGui::DragFloat("Walk Speed", &walk, 0.1f, 0.0f, 20.0f)) cc->setWalkSpeed(walk);

                float run = cc->getRunSpeed();
                if (ImGui::DragFloat("Run Speed", &run, 0.1f, 0.0f, 20.0f)) cc->setRunSpeed(run);

                float crouch = cc->getCrouchSpeed();
                if (ImGui::DragFloat("Crouch Speed", &crouch, 0.1f, 0.0f, 20.0f)) cc->setCrouchSpeed(crouch);

                float jump = cc->getJumpForce();
                if (ImGui::DragFloat("Jump Force", &jump, 0.1f, 0.0f, 20.0f)) cc->setJumpForce(jump);

                float grav = cc->getGravity();
                if (ImGui::DragFloat("Gravity", &grav, 0.1f, -30.0f, 30.0f)) cc->setGravity(grav);

                float air = cc->getAirControl();
                if (ImGui::DragFloat("Air Control", &air, 0.01f, 0.0f, 1.0f)) cc->setAirControl(air);

                ImGui::Separator();
                ImGui::Text("State");
                ImGui::Text("Grounded: %s", cc->isGroundedState() ? "Yes" : "No");
                ImGui::Text("Jumping: %s", cc->isJumpingState() ? "Yes" : "No");

                glm::vec3 vel = cc->getVelocity();
                ImGui::Text("Velocity: %.2f %.2f %.2f", vel.x, vel.y, vel.z);

                // Botones de control
                if (ImGui::Button("Force Jump")) {
                    cc->jump();
                }
                ImGui::SameLine();
                if (ImGui::Button("Teleport to Origin")) {
                    cc->teleport(glm::vec3(0, 2, 0));
                }
            }

            ImGui::TreePop();
        }

        if (removeComponent) {
            go->removeComponent<CharacterController>();
        }
    }
}

void ComponentSerializer::RenderRigidbodyComponent(GameObject* go) {
    if (auto rigidbody = go->getComponent<Rigidbody>()) {
        // Get state for this specific GameObject
        ComponentUIState* state = getOrCreateState(go);
        
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Rigidbody]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            // Botón de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##Rigidbody", ImVec2(30, 0))) {
                ImGui::OpenPopup("RigidbodyOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("RigidbodyOptions")) {
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

            if (!removeComponent && rigidbody != nullptr) {
                // Enabled/Disabled
                bool enabled = rigidbody->isActive();
                if (ImGui::Checkbox("Enabled", &enabled)) {
                    if (enabled) {
                        rigidbody->enable();
                    } else {
                        rigidbody->disable();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Body Properties");

                // Body Type
                BodyType currentType = rigidbody->getBodyType();
                const char* bodyTypes[] = { "Static", "Dynamic", "Kinematic" };
                int currentItem = static_cast<int>(currentType);
                if (ImGui::Combo("Body Type", &currentItem, bodyTypes, 3)) {
                    rigidbody->setBodyType(static_cast<BodyType>(currentItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Static: Immovable, Dynamic: Affected by physics, Kinematic: Movable but not affected by forces");
                }

                // Mass
                float mass = rigidbody->getMass();
                if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.1f, 1000.0f)) {
                    rigidbody->setMass(mass);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mass of the object (affects physics behavior)");
                }

                // Gravity Factor
                float gravityFactor = rigidbody->getGravityFactor();
                if (ImGui::DragFloat("Gravity Factor", &gravityFactor, 0.1f, 0.0f, 10.0f)) {
                    rigidbody->setGravityFactor(gravityFactor);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Multiplier for gravity effect (0 = no gravity, 1 = normal gravity)");
                }

                ImGui::Separator();
                ImGui::Text("Physics Properties");

                // Linear Velocity
                glm::vec3 velocity = rigidbody->getVelocity();
                if (ImGui::DragFloat3("Linear Velocity", glm::value_ptr(velocity), 0.1f)) {
                    rigidbody->setVelocity(velocity);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Current linear velocity of the object");
                }

                // Damping
                float damping = rigidbody->getDamping();
                if (ImGui::DragFloat("Damping", &damping, 0.01f, 0.0f, 1.0f)) {
                    rigidbody->setDamping(damping);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Damping factor for velocity (0 = no damping, 1 = full damping)");
                }

                // Physics State
                ImGui::Separator();
                ImGui::Text("Physics State");

                // Is Awake
                bool isAwake = rigidbody->isAwake();
                ImGui::Text("Is Awake: %s", isAwake ? "Yes" : "No");

                if (ImGui::Button("Wake Up")) {
                    rigidbody->wakeUp();
                }

                ImGui::SameLine();
                if (ImGui::Button("Add Force")) {
                    state->rigidbodyShowForceDialog = true;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Apply a force to the object");
                }

                // Force application dialog
                if (state->rigidbodyShowForceDialog) {
                    ImGui::OpenPopup("Add Force##Rigidbody");
                }

                if (ImGui::BeginPopupModal("Add Force##Rigidbody", &state->rigidbodyShowForceDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Apply Force to Object");
                    ImGui::Separator();

                    if (ImGui::DragFloat3("Force Vector", glm::value_ptr(state->rigidbodyForceVector), 0.1f)) {
                        // Force vector is updated in real-time
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Apply Force", ImVec2(120, 0))) {
                        rigidbody->addForce(state->rigidbodyForceVector);
                        state->rigidbodyForceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->rigidbodyShowForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        state->rigidbodyForceVector = glm::vec3(0.0f, 0.0f, 0.0f);
                        state->rigidbodyShowForceDialog = false;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // Torque and impulse buttons (simplified for space)
                if (ImGui::Button("Add Torque")) {
                    state->rigidbodyShowTorqueDialog = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Add Impulse")) {
                    state->rigidbodyShowImpulseDialog = true;
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<Rigidbody>();
        }
    }
}

void ComponentSerializer::RenderColliderComponent(GameObject* go) {
    if (auto collider = go->getComponent<Collider>()) {
        // Get state for this specific GameObject
        ComponentUIState* state = getOrCreateState(go);
        
        bool removeComponent = false;
        bool treeNodeOpen = ImGui::TreeNodeEx("[Collider]", ImGuiTreeNodeFlags_DefaultOpen);

        if (treeNodeOpen) {
            // Botón de opciones alineado a la derecha pero dentro de la ventana
            float windowWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(windowWidth - 35);
            if (ImGui::Button(" ... ##Collider", ImVec2(30, 0))) {
                ImGui::OpenPopup("ColliderOptions");
            }

            // Popup de opciones
            if (ImGui::BeginPopup("ColliderOptions")) {
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

            if (!removeComponent && collider != nullptr) {
                ImGui::Separator();
                ImGui::Text("Shape Properties");

                // Shape Type
                ShapeType currentShapeType = collider->getShapeType();
                const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Plane" };
                int currentShapeItem = static_cast<int>(currentShapeType);
                if (ImGui::Combo("Shape Type", &currentShapeItem, shapeTypes, 4)) {
                    collider->setShapeType(static_cast<ShapeType>(currentShapeItem));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Shape of the collision volume");
                }

                // Shape-specific properties
                switch (currentShapeType) {
                case ShapeType::Box: {
                    glm::vec3 boxExtents = collider->getBoxHalfExtents();
                    if (ImGui::DragFloat3("Box Half Extents", glm::value_ptr(boxExtents), 0.1f, 0.1f, 10.0f)) {
                        collider->setBoxHalfExtents(boxExtents);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half extents of the box shape (X, Y, Z)");
                    }
                    break;
                }
                case ShapeType::Sphere: {
                    float radius = collider->getSphereRadius();
                    if (ImGui::DragFloat("Sphere Radius", &radius, 0.1f, 0.1f, 10.0f)) {
                        collider->setSphereRadius(radius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the sphere shape");
                    }
                    break;
                }
                case ShapeType::Capsule: {
                    float capsuleRadius = collider->getCapsuleRadius();
                    float capsuleHalfHeight = collider->getCapsuleHalfHeight();

                    if (ImGui::DragFloat("Capsule Radius", &capsuleRadius, 0.1f, 0.1f, 10.0f)) {
                        collider->setCapsuleRadius(capsuleRadius);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Radius of the capsule shape");
                    }

                    if (ImGui::DragFloat("Capsule Half Height", &capsuleHalfHeight, 0.1f, 0.1f, 10.0f)) {
                        collider->setCapsuleHalfHeight(capsuleHalfHeight);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Half height of the capsule shape");
                    }
                    break;
                }
                case ShapeType::Plane: {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Plane shape - no additional properties");
                    break;
                }
                }

                ImGui::Separator();
                ImGui::Text("Material Properties");

                // Friction
                float friction = collider->getFriction();
                if (ImGui::DragFloat("Friction", &friction, 0.01f, 0.0f, 2.0f)) {
                    collider->setFriction(friction);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Friction coefficient for contact with other objects");
                }

                // Restitution
                float restitution = collider->getRestitution();
                if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f)) {
                    collider->setRestitution(restitution);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Bounciness factor (0 = no bounce, 1 = perfect bounce)");
                }

                ImGui::Separator();
                ImGui::Text("Trigger Properties");

                // Is Trigger checkbox
                bool isTrigger = collider->isTrigger();
                if (ImGui::Checkbox("Is Trigger", &isTrigger)) {
                    collider->setTrigger(isTrigger);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Trigger shapes don't cause physical collisions but can detect overlaps");
                }

                // Show trigger status
                if (isTrigger) {
                    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Trigger Active");
                    ImGui::Text("Objects should pass through this collider");
                } else {
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "⚫ Solid Collider");
                    ImGui::Text("Objects will collide and stop");
                }

                ImGui::Separator();
                ImGui::Text("Collision Filters");

                // Layer configuration
                physx::PxU32 currentLayer = collider->getLayer();
                physx::PxU32 currentLayerMask = collider->getLayerMask();

                ImGui::Text("Layer (Word0): %u", currentLayer);
                ImGui::Text("Layer Mask (Word1): %u", currentLayerMask);

                // Debug collision filters button
                if (ImGui::Button("Debug Collision Filters")) {
                    collider->debugCollisionFilters();
                }

                // Setup as trigger button
                if (ImGui::Button("Setup as Trigger")) {
                    collider->setupAsTrigger();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Quick setup as a trigger with default player collision");
                }

                // Force transform update button (for debugging)
                ImGui::SameLine();
                if (ImGui::Button("Update Transform")) {
                    collider->forceTransformUpdate();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Force update of collider position to match GameObject");
                }
                
                // Mark transform dirty button (for when GameObject moves)
                ImGui::SameLine();
                if (ImGui::Button("Mark Dirty")) {
                    collider->markTransformDirty();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mark transform as needing update (will update on next frame)");
                }

                // Debug trigger configuration
                if (ImGui::Button("Test Trigger Config")) {
                    std::cout << "[Inspector] Testing trigger configuration for " << go->Name << std::endl;
                    collider->forceUpdateCollisionFilters();
                    collider->verifyTriggerSetup();
                    
                    // Also suggest checking the other object
                    std::cout << "[Inspector] Remember to check that the other object can collide with this trigger!" << std::endl;
                    std::cout << "[Inspector] The other object should have layer mask that includes this trigger's layer." << std::endl;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Debug trigger configuration and print status to console");
                }
                
                // Force recreation button for problematic triggers
                ImGui::SameLine();
                if (ImGui::Button("Force Recreate")) {
                    std::cout << "[Inspector] Forcing shape recreation for " << go->Name << std::endl;
                    collider->recreateShapeSafely();
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Force recreation of the collision shape (fixes shared shape issues)");
                }
            }
            ImGui::TreePop();
        }
        if (removeComponent) {
            go->removeComponent<Collider>();
        }
    }
}
