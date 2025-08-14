#pragma once
#include "WindowBehaviour.h"
#include "../../src/ui/UIBehaviour.h"
#include "../../src/ui/UIElements/UIText.h"
#include "../../src/render/RenderPipeline.h"
#include "../../src/input/InputSystem.h"
#include <vector>
#include <memory>
#include <string>

class CanvasManager : public WindowBehaviour {
private:
    // Element selection and editing
    UIBehaviour* selectedElement = nullptr;
    int selectedCanvasIndex = 0;
    int selectedElementIndex = -1;
    
    // Text editing buffer
    char textEditBuffer[256] = "";
    bool isEditingText = false;
    
    // Movement settings
    float moveSpeed = 5.0f;
    bool isRealTimeMovement = true;
    
    // Input actions for element movement
    std::shared_ptr<InputAction> moveLeftAction;
    std::shared_ptr<InputAction> moveRightAction;
    std::shared_ptr<InputAction> moveUpAction;
    std::shared_ptr<InputAction> moveDownAction;
    std::shared_ptr<InputAction> fastMoveAction;
    
    // Helper methods
    Canvas2D* getCurrentCanvas();
    void setupInputActions();
    void updateSelectedElement();
    void renderElementList();
    void renderElementProperties();
    void renderMovementControls();
    void renderTextEditControls();
    void selectElement(int canvasIndex, int elementIndex);
    void moveSelectedElement(float deltaX, float deltaY);
    
public:
    CanvasManager();
    ~CanvasManager();
    
    void OnRenderGUI() override;
    const std::string& getName() const override;
    
    // Public methods for external access
    void selectElementByClick(int canvasIndex, int elementIndex);
    UIBehaviour* getSelectedElement() const { return selectedElement; }
}; 