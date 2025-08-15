#include "InputEditor.h"
#include <imgui/imgui.h>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../../src/core/InputConfigLoader.h"

using json = nlohmann::json;

InputEditor::InputEditor() 
    : selectedActionIndex(-1)
    , showAddNewInput(false)
    , newInputType(InputType::Button)
    , isEditingKeyBinding(false)
    , isEditingMouseBinding(false)
    , editingBindingIndex(-1)
    , newKeyCode(SDLK_UNKNOWN)
    , newKeyPositive(true)
    , newKeyAxis(0)
    , newMouseAxis(MouseAxisType::X)
    , newMouseButton(SDL_BUTTON_LEFT)
{
    strcpy_s(newInputNameBuffer, "NewInput");
    strcpy_s(editingNameBuffer, "");
}

void InputEditor::OnRenderGUI() {
    if (!isOpen) return;
    
    ImGui::Begin("Input Editor", &isOpen);
    
    // Toolbar
    if (ImGui::Button("Save Config")) {
        saveInputConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Config")) {
        loadInputConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Add New Input")) {
        showAddNewInput = true;
    }
    ImGui::Separator();
    
    // Main content
    ImGui::Columns(2, "InputEditorColumns");
    
    // Left column - Input actions list
    renderInputActionsList();
    
    ImGui::NextColumn();
    
    // Right column - Input action editor
    renderInputActionEditor();
    
    ImGui::Columns(1);
    
    // Add new input dialog
    if (showAddNewInput) {
        renderAddNewInput();
    }
    
    // Render binding editors
    if (selectedAction) {
        renderBindingEditor(selectedAction);
    }
    
    ImGui::End();
}

void InputEditor::renderInputActionsList() {
    ImGui::Text("Input Actions");
    ImGui::Separator();
    
    auto& inputSystem = InputSystem::getInstance();
    
    // Display all registered actions
    for (size_t i = 0; i < inputSystem.getActionCount(); ++i) {
        std::string actionName = inputSystem.getActionNameByIndex(i);
        auto action = inputSystem.getActionByIndex(i);
        
        if (!action) continue;
        
        // Create a selectable item for each action
        bool isSelected = (selectedActionIndex == static_cast<int>(i));
        if (ImGui::Selectable(actionName.c_str(), isSelected)) {
            selectedActionIndex = static_cast<int>(i);
            selectedAction = action;
            
            // Initialize editing state
            strcpy_s(editingNameBuffer, actionName.c_str());
            editingType = action->getType();
            editingBindings = action->getBindings();
        }
        
        // Show action type as a small label
        ImGui::SameLine();
        ImGui::TextDisabled("(%s)", inputTypeToString(action->getType()).c_str());
    }
    
    if (ImGui::Button("Refresh List")) {
        // The list is already up to date since we're iterating directly
    }
}

void InputEditor::renderInputActionEditor() {
    if (!selectedAction) {
        ImGui::Text("Select an input action to edit");
        return;
    }
    
    ImGui::Text("Edit Input Action");
    ImGui::Separator();
    
    // Name editing
    if (ImGui::InputText("Name", editingNameBuffer, sizeof(editingNameBuffer))) {
        // Name changed
    }
    
    // Type editing
    int currentType = static_cast<int>(editingType);
    if (ImGui::Combo("Type", &currentType, inputTypeNames, 5)) {
        editingType = static_cast<InputType>(currentType);
    }
    
    ImGui::Separator();
    ImGui::Text("Bindings");
    
    // Show existing bindings
    const auto& bindings = selectedAction->getBindings();
    for (int i = 0; i < bindings.size(); ++i) {
        const auto& binding = bindings[i];
        
        ImGui::PushID(i);
        
        std::string bindingText;
        if (binding.isKeyboard) {
            std::string keyName = keycodeToString(binding.key);
            // Remove "SDLK_" prefix for display
            if (keyName.substr(0, 5) == "SDLK_") {
                keyName = keyName.substr(5);
            }
            bindingText = "Key: " + keyName;
            if (binding.axis >= 0) {
                bindingText += " (Axis: " + std::to_string(binding.axis) + ")";
            }
            if (editingType == InputType::Vector2D || editingType == InputType::Value) {
                bindingText += binding.isPositive ? " (+)" : " (-)";
            }
        } else {
            if (binding.mouseAxis != MouseAxisType::X) { // Assuming X is default
                bindingText = "Mouse Axis: " + mouseAxisToString(binding.mouseAxis);
            } else {
                std::string buttonName = mouseButtonToString(binding.mouseButton);
                // Remove "SDL_BUTTON_" prefix for display
                if (buttonName.substr(0, 11) == "SDL_BUTTON_") {
                    buttonName = buttonName.substr(11);
                }
                bindingText = "Mouse Button: " + buttonName;
            }
        }
        
        ImGui::Text("%s", bindingText.c_str());
        ImGui::SameLine();
        
        if (ImGui::Button("Edit")) {
            isEditingKeyBinding = binding.isKeyboard;
            isEditingMouseBinding = !binding.isKeyboard;
            editingBindingIndex = i;
            
            if (binding.isKeyboard) {
                newKeyCode = binding.key;
                newKeyPositive = binding.isPositive;
                newKeyAxis = binding.axis;
            } else {
                newMouseAxis = binding.mouseAxis;
                newMouseButton = binding.mouseButton;
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            selectedAction->removeBinding(i);
        }
        
        ImGui::PopID();
    }
    
    // Add new binding buttons
    if (ImGui::Button("Add Key Binding")) {
        isEditingKeyBinding = true;
        isEditingMouseBinding = false;
        editingBindingIndex = -1;
        newKeyCode = SDLK_UNKNOWN;
        newKeyPositive = true;
        newKeyAxis = 0;
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Add Mouse Binding")) {
        isEditingMouseBinding = true;
        isEditingKeyBinding = false;
        editingBindingIndex = -1;
        newMouseAxis = MouseAxisType::X;
        newMouseButton = SDL_BUTTON_LEFT;
    }
    
    // Apply changes
    if (ImGui::Button("Apply Changes")) {
        // For now, we'll just save the config
        // In a real implementation, you might want to update the action name/type
        saveInputConfig();
    }
    
    // Delete action
    ImGui::SameLine();
    if (ImGui::Button("Delete Action")) {
                    if (selectedAction) {
                auto& inputSystem = InputSystem::getInstance();
                inputSystem.removeAction(editingNameBuffer);
            selectedAction = nullptr;
            selectedActionIndex = -1;
        }
    }
}

void InputEditor::renderAddNewInput() {
    ImGui::OpenPopup("Add New Input");
    
    if (ImGui::BeginPopupModal("Add New Input", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Input Name", newInputNameBuffer, sizeof(newInputNameBuffer));
        
        int currentType = static_cast<int>(newInputType);
        if (ImGui::Combo("Input Type", &currentType, inputTypeNames, 5)) {
            newInputType = static_cast<InputType>(currentType);
        }
        
        if (ImGui::Button("Create")) {
            auto& inputSystem = InputSystem::getInstance();
            auto newAction = inputSystem.registerAction(newInputNameBuffer, newInputType);
            
            // Reset form
            strcpy_s(newInputNameBuffer, "NewInput");
            newInputType = InputType::Button;
            showAddNewInput = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            showAddNewInput = false;
        }
        
        ImGui::EndPopup();
    }
}

void InputEditor::renderBindingEditor(std::shared_ptr<InputAction> action) {
    if (isEditingKeyBinding) {
        renderKeyBindingEditor(action, editingBindingIndex);
    } else if (isEditingMouseBinding) {
        renderMouseBindingEditor(action, editingBindingIndex);
    }
}

void InputEditor::renderKeyBindingEditor(std::shared_ptr<InputAction> action, int bindingIndex) {
    ImGui::OpenPopup("Edit Key Binding");
    
    if (ImGui::BeginPopupModal("Edit Key Binding", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select Key:");
        
        // Common keys list with proper names
        SDL_Keycode keyCodes[] = {
            SDLK_W, SDLK_A, SDLK_S, SDLK_D, SDLK_Q, SDLK_E, SDLK_R, SDLK_F, SDLK_G, SDLK_H, SDLK_I, SDLK_J, SDLK_K, SDLK_L,
            SDLK_Z, SDLK_X, SDLK_C, SDLK_V, SDLK_B, SDLK_N, SDLK_M, SDLK_Y, SDLK_U, SDLK_O, SDLK_P,
            SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_0,
            SDLK_SPACE, SDLK_RETURN, SDLK_ESCAPE, SDLK_TAB, SDLK_LSHIFT, SDLK_LCTRL, SDLK_LALT
        };
        
        for (int i = 0; i < sizeof(keyCodes) / sizeof(keyCodes[0]); ++i) {
            std::string keyName = InputConfigLoader::keycodeToString(keyCodes[i]);
            // Remove "SDLK_" prefix for display
            if (keyName.substr(0, 5) == "SDLK_") {
                keyName = keyName.substr(5);
            }
            if (ImGui::Button(keyName.c_str())) {
                newKeyCode = keyCodes[i];
            }
            if ((i + 1) % 7 != 0) ImGui::SameLine();
        }
        
        ImGui::Separator();
        std::string currentKeyName = keycodeToString(newKeyCode);
        // Remove "SDLK_" prefix for display
        if (currentKeyName.substr(0, 5) == "SDLK_") {
            currentKeyName = currentKeyName.substr(5);
        }
        ImGui::Text("Current Key: %s", currentKeyName.c_str());
        
        if (action->getType() == InputType::Vector2D || action->getType() == InputType::Value) {
            ImGui::Checkbox("Positive", &newKeyPositive);
            ImGui::InputInt("Axis", &newKeyAxis);
        }
        
        if (ImGui::Button("Apply")) {
            if (bindingIndex >= 0) {
                // Update existing binding
                action->updateKeyBinding(bindingIndex, newKeyCode, newKeyPositive, newKeyAxis);
            } else {
                // Add new binding
                action->addKeyBinding(newKeyCode, newKeyPositive, newKeyAxis);
            }
            isEditingKeyBinding = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            isEditingKeyBinding = false;
        }
        
        ImGui::EndPopup();
    }
}

void InputEditor::renderMouseBindingEditor(std::shared_ptr<InputAction> action, int bindingIndex) {
    ImGui::OpenPopup("Edit Mouse Binding");
    
    if (ImGui::BeginPopupModal("Edit Mouse Binding", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Select Mouse Input:");
        
        if (action->getType() == InputType::MouseAxis) {
            ImGui::Text("Mouse Axis:");
            int currentAxis = static_cast<int>(newMouseAxis);
            if (ImGui::Combo("Axis", &currentAxis, mouseAxisNames, 3)) {
                newMouseAxis = static_cast<MouseAxisType>(currentAxis);
            }
            ImGui::Text("Current: %s", mouseAxisToString(newMouseAxis).c_str());
        } else if (action->getType() == InputType::MouseButton) {
            ImGui::Text("Mouse Button:");
            int currentButton = (newMouseButton == SDL_BUTTON_LEFT) ? 0 : 
                               (newMouseButton == SDL_BUTTON_RIGHT) ? 1 : 2;
            if (ImGui::Combo("Button", &currentButton, mouseButtonNames, 3)) {
                newMouseButton = (currentButton == 0) ? SDL_BUTTON_LEFT :
                                (currentButton == 1) ? SDL_BUTTON_RIGHT : SDL_BUTTON_MIDDLE;
            }
            ImGui::Text("Current: %s", mouseButtonToString(newMouseButton).c_str());
        }
        
        if (ImGui::Button("Apply")) {
            if (bindingIndex >= 0) {
                // Update existing binding
                if (action->getType() == InputType::MouseAxis) {
                    action->updateMouseAxisBinding(bindingIndex, newMouseAxis);
                } else {
                    action->updateMouseButtonBinding(bindingIndex, newMouseButton);
                }
            } else {
                // Add new binding
                if (action->getType() == InputType::MouseAxis) {
                    action->addMouseAxisBinding(newMouseAxis);
                } else {
                    action->addMouseButtonBinding(newMouseButton);
                }
            }
            isEditingMouseBinding = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            isEditingMouseBinding = false;
        }
        
        ImGui::EndPopup();
    }
}

void InputEditor::saveInputConfig() {
    InputConfigLoader::saveInputConfigToJSON("config/input_config.json");
}

void InputEditor::loadInputConfig() {
    InputConfigLoader::loadInputConfigFromJSON("config/input_config.json");
}

std::string InputEditor::keycodeToString(SDL_Keycode keycode) {
    return InputConfigLoader::keycodeToString(keycode);
}

std::string InputEditor::mouseAxisToString(MouseAxisType axis) {
    return InputConfigLoader::mouseAxisToString(axis);
}

std::string InputEditor::mouseButtonToString(Uint8 button) {
    return InputConfigLoader::mouseButtonToString(button);
}

std::string InputEditor::inputTypeToString(InputType type) {
    switch (type) {
        case InputType::Button: return "Button";
        case InputType::Value: return "Value";
        case InputType::Vector2D: return "Vector2D";
        case InputType::MouseButton: return "MouseButton";
        case InputType::MouseAxis: return "MouseAxis";
        default: return "Unknown";
    }
} 