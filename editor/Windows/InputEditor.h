#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <string>
#include <vector>
#include <memory>
#include "../../src/input/InputAction.h"
#include "../../src/input/InputSystem.h"

class InputEditor : public WindowBehaviour {
public:
    InputEditor();
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Input Editor";
        return name;
    }

private:
    void renderInputActionsList();
    void renderInputActionEditor();
    void renderAddNewInput();
    void renderBindingEditor(std::shared_ptr<InputAction> action);
    void renderKeyBindingEditor(std::shared_ptr<InputAction> action, int bindingIndex);
    void renderMouseBindingEditor(std::shared_ptr<InputAction> action, int bindingIndex);
    
    void saveInputConfig();
    void loadInputConfig();
    
    std::string keycodeToString(SDL_Keycode keycode);
    std::string mouseAxisToString(MouseAxisType axis);
    std::string mouseButtonToString(Uint8 button);
    std::string inputTypeToString(InputType type);
    
    // UI State
    std::shared_ptr<InputAction> selectedAction;
    int selectedActionIndex;
    bool showAddNewInput;
    char newInputNameBuffer[256];
    InputType newInputType;
    
    // Temporary editing state
    char editingNameBuffer[256];
    InputType editingType;
    std::vector<InputBinding> editingBindings;
    
    // Key binding editing
    bool isEditingKeyBinding;
    int editingBindingIndex;
    SDL_Keycode newKeyCode;
    bool newKeyPositive;
    int newKeyAxis;
    
    // Mouse binding editing
    bool isEditingMouseBinding;
    MouseAxisType newMouseAxis;
    Uint8 newMouseButton;
    
    // Input types for combo box
    const char* inputTypeNames[5] = {
        "Button",
        "Value", 
        "Vector2D",
        "MouseButton",
        "MouseAxis"
    };
    
    // Mouse axis types for combo box
    const char* mouseAxisNames[3] = {
        "X",
        "Y", 
        "ScrollWheel"
    };
    
    // Mouse button names
    const char* mouseButtonNames[3] = {
        "Left Button",
        "Right Button",
        "Middle Button"
    };
}; 