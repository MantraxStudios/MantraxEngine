#include <imgui/imgui.h>
#include "CanvasManager.h"
#include <iostream>
#include <filesystem>
#include <render/RenderPipeline.h>
#include <components/SceneManager.h>
#include <SDL.h>
#include <cstring>
#include <algorithm>
#include <memory>

// Constructor
CanvasManager::CanvasManager() {
    setupInputActions();
    std::cout << "[CanvasManager] Initialized with input controls" << std::endl;
}

// Destructor
CanvasManager::~CanvasManager() {
    // Clean up input actions
    auto& inputSystem = InputSystem::getInstance();
    if (moveLeftAction) inputSystem.removeAction("canvas_move_left");
    if (moveRightAction) inputSystem.removeAction("canvas_move_right");
    if (moveUpAction) inputSystem.removeAction("canvas_move_up");
    if (moveDownAction) inputSystem.removeAction("canvas_move_down");
    if (fastMoveAction) inputSystem.removeAction("canvas_fast_move");
}

void CanvasManager::OnRenderGUI() {
    ImGui::Begin("Canvas Editor", &isOpen);
    
    // Update selected element based on input
    updateSelectedElement();
    
    // ========== Canvas Creation Section ==========
    ImGui::Text("=== Canvas Creation ===");
    
    if (ImGui::Button("Make Canvas")) {
        Canvas2D* canvas = SceneManager::getInstance().getActiveScene()->getRenderPipeline()->addCanvas(800, 600);
        if (canvas) {
            std::cout << "Canvas created successfully!" << std::endl;
        } else {
            std::cerr << "Failed to create canvas!" << std::endl;
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Make New Text")) {
        Canvas2D* canvas = getCurrentCanvas();
        if (canvas) {
            UIText* newText = canvas->MakeNewText("New Text Element");
            newText->setAnchor(Anchor::TopLeft);
            newText->setPosition(100, 100);
            newText->enableDrag(true); // Enable drag by default
            std::cout << "New text element created!" << std::endl;
        } else {
            std::cerr << "No canvas available!" << std::endl;
        }
    }
    
    ImGui::Separator();
    
    // ========== Canvas Selection ==========
    ImGui::Text("=== Canvas Selection ===");
    
    RenderPipeline* renderPipeline = SceneManager::getInstance().getActiveScene()->getRenderPipeline();
    if (renderPipeline) {
        int canvasCount = renderPipeline->getCanvasCount();
        
        if (ImGui::SliderInt("Canvas Index", &selectedCanvasIndex, 0, canvasCount - 1)) {
            selectedElement = nullptr; // Clear selection when changing canvas
            selectedElementIndex = -1;
        }
        
        ImGui::Text("Total Canvases: %d", canvasCount);
    }
    
    ImGui::Separator();
    
    // ========== Element List ==========
    renderElementList();
    
    ImGui::Separator();
    
    // ========== Element Properties ==========
    renderElementProperties();
    
    ImGui::Separator();
    
    // ========== Movement Controls ==========
    renderMovementControls();
    
    ImGui::Separator();
    
    // ========== Text Editing ==========
    renderTextEditControls();
    
    ImGui::End();
}

const std::string& CanvasManager::getName() const {
    static const std::string name = "Canvas Manager";
    return name;
}

// ========== Helper Methods Implementation ==========

Canvas2D* CanvasManager::getCurrentCanvas() {
    RenderPipeline* renderPipeline = SceneManager::getInstance().getActiveScene()->getRenderPipeline();
    if (renderPipeline && selectedCanvasIndex >= 0 && selectedCanvasIndex < renderPipeline->getCanvasCount()) {
        return renderPipeline->getCanvas(selectedCanvasIndex);
    }
    return nullptr;
}

void CanvasManager::setupInputActions() {
    auto& inputSystem = InputSystem::getInstance();
    
    // Movement actions
    moveLeftAction = inputSystem.registerAction("canvas_move_left", InputType::Button);
    if (moveLeftAction) {
        moveLeftAction->addKeyBinding(SDLK_a);
        moveLeftAction->addKeyBinding(SDLK_LEFT);
    }
    
    moveRightAction = inputSystem.registerAction("canvas_move_right", InputType::Button);
    if (moveRightAction) {
        moveRightAction->addKeyBinding(SDLK_d);
        moveRightAction->addKeyBinding(SDLK_RIGHT);
    }
    
    moveUpAction = inputSystem.registerAction("canvas_move_up", InputType::Button);
    if (moveUpAction) {
        moveUpAction->addKeyBinding(SDLK_w);
        moveUpAction->addKeyBinding(SDLK_UP);
    }
    
    moveDownAction = inputSystem.registerAction("canvas_move_down", InputType::Button);
    if (moveDownAction) {
        moveDownAction->addKeyBinding(SDLK_s);
        moveDownAction->addKeyBinding(SDLK_DOWN);
    }
    
    fastMoveAction = inputSystem.registerAction("canvas_fast_move", InputType::Button);
    if (fastMoveAction) {
        fastMoveAction->addKeyBinding(SDLK_LSHIFT);
        fastMoveAction->addKeyBinding(SDLK_RSHIFT);
    }
    
    std::cout << "[CanvasManager] Input actions configured: WASD/Arrows for movement, Shift for fast move" << std::endl;
}

void CanvasManager::updateSelectedElement() {
    if (!selectedElement || !isRealTimeMovement) return;
    
    // Get current key states
    const Uint8* keyState = SDL_GetKeyboardState(nullptr);
    
    float currentMoveSpeed = moveSpeed;
    
    // Check for fast move modifier
    if (keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT]) {
        currentMoveSpeed *= 3.0f;
    }
    
    // Calculate movement
    float deltaX = 0.0f, deltaY = 0.0f;
    
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) {
        deltaX -= currentMoveSpeed;
    }
    if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) {
        deltaX += currentMoveSpeed;
    }
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
        deltaY -= currentMoveSpeed;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
        deltaY += currentMoveSpeed;
    }
    
    // Apply movement
    if (deltaX != 0.0f || deltaY != 0.0f) {
        moveSelectedElement(deltaX, deltaY);
    }
}

void CanvasManager::renderElementList() {
    ImGui::Text("=== UI Elements ===");
    
    Canvas2D* canvas = getCurrentCanvas();
    if (!canvas) {
        ImGui::Text("No canvas selected");
        return;
    }
    
    ImGui::Text("Canvas %d - Elements: %zu", selectedCanvasIndex, canvas->RenderElements.size());
    
    // List all elements with selection
    for (size_t i = 0; i < canvas->RenderElements.size(); ++i) {
        UIBehaviour* element = canvas->RenderElements[i];
        if (!element) continue;
        
        // Try to cast to UIText to get more info
        UIText* textElement = dynamic_cast<UIText*>(element);
        std::string elementName = textElement ? ("Text: \"" + textElement->Text + "\"") : ("Element " + std::to_string(i));
        
        if (ImGui::Button(elementName.c_str())) {
            selectElement(selectedCanvasIndex, static_cast<int>(i));
        }
        
        // Show element info on same line
        if (textElement) {
            ImGui::SameLine();
            ImGui::Text("Pos: (%.1f, %.1f)", textElement->Position.x, textElement->Position.y);
        }
    }
    
    if (canvas->RenderElements.empty()) {
        ImGui::Text("No elements in this canvas");
    }
}

void CanvasManager::renderElementProperties() {
    ImGui::Text("=== Element Properties ===");
    
    if (!selectedElement) {
        ImGui::Text("No element selected");
        return;
    }
    
    // Show selected element info
    UIText* textElement = dynamic_cast<UIText*>(selectedElement);
    if (textElement) {
        ImGui::Text("Selected: Text Element");
        ImGui::Text("Current Text: \"%s\"", textElement->Text.c_str());
        
        // Position controls
        float pos[2] = { textElement->Position.x, textElement->Position.y };
        if (ImGui::DragFloat2("Position", pos, 1.0f)) {
            textElement->setPosition(pos[0], pos[1]);
        }
        
        // Color controls
        float color[4] = { textElement->Color.r, textElement->Color.g, textElement->Color.b, textElement->Color.a };
        if (ImGui::ColorEdit4("Color", color)) {
            textElement->Color = glm::vec4(color[0], color[1], color[2], color[3]);
        }
        
        // Anchor selection
        static const char* anchorNames[] = {
            "TopLeft", "TopCenter", "TopRight",
            "CenterLeft", "Center", "CenterRight", 
            "BottomLeft", "BottomCenter", "BottomRight"
        };
        
        int currentAnchor = static_cast<int>(textElement->UIAnchor);
        if (ImGui::Combo("Anchor", &currentAnchor, anchorNames, 9)) {
            textElement->setAnchor(static_cast<Anchor>(currentAnchor));
        }
        
        // Drag settings
        bool isDragEnabled = textElement->isDragEnabled();
        if (ImGui::Checkbox("Enable Drag", &isDragEnabled)) {
            textElement->enableDrag(isDragEnabled);
        }
        
    } else {
        ImGui::Text("Selected: Unknown Element Type");
    }
}

void CanvasManager::renderMovementControls() {
    ImGui::Text("=== Movement Controls ===");
    
    if (!selectedElement) {
        ImGui::Text("Select an element to move it");
        return;
    }
    
    // Movement settings
    ImGui::SliderFloat("Move Speed", &moveSpeed, 1.0f, 50.0f);
    ImGui::Checkbox("Real-time Movement", &isRealTimeMovement);
    
    ImGui::Text("Controls:");
    ImGui::BulletText("WASD or Arrow Keys: Move element");
    ImGui::BulletText("Hold Shift: Fast movement (3x speed)");
    ImGui::BulletText("Real-time: Move while keys are held");
    
    // Manual movement buttons
    ImGui::Text("Manual Movement:");
    
    if (ImGui::Button("< Left")) {
        moveSelectedElement(-moveSpeed, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Right >")) {
        moveSelectedElement(moveSpeed, 0);
    }
    
    if (ImGui::Button("^ Up")) {
        moveSelectedElement(0, -moveSpeed);
    }
    ImGui::SameLine();
    if (ImGui::Button("Down v")) {
        moveSelectedElement(0, moveSpeed);
    }
    
    // Current position display
    UIText* textElement = dynamic_cast<UIText*>(selectedElement);
    if (textElement) {
        ImGui::Text("Current Position: (%.1f, %.1f)", textElement->Position.x, textElement->Position.y);
    }
}

void CanvasManager::renderTextEditControls() {
    ImGui::Text("=== Text Editing ===");
    
    UIText* textElement = dynamic_cast<UIText*>(selectedElement);
    if (!textElement) {
        ImGui::Text("Select a text element to edit");
        return;
    }
    
    // Text editing
    if (!isEditingText) {
        // Copy current text to buffer safely using modern C++
        size_t copyLen = std::min(textElement->Text.length(), sizeof(textEditBuffer) - 1);
        std::memcpy(textEditBuffer, textElement->Text.c_str(), copyLen);
        textEditBuffer[copyLen] = '\0';
    }
    
    ImGui::Text("Edit Text:");
    if (ImGui::InputText("##textedit", textEditBuffer, sizeof(textEditBuffer))) {
        isEditingText = true;
        textElement->Text = std::string(textEditBuffer);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Apply")) {
        textElement->Text = std::string(textEditBuffer);
        isEditingText = false;
        std::cout << "[CanvasManager] Text updated to: \"" << textElement->Text << "\"" << std::endl;
    }
    
    // Quick text presets
    ImGui::Text("Quick Presets:");
    if (ImGui::Button("Hello World")) {
        textElement->Text = "Hello World";
        strcpy_s(textEditBuffer, sizeof(textEditBuffer), "Hello World");
    }
    ImGui::SameLine();
    if (ImGui::Button("UI Element")) {
        textElement->Text = "UI Element";
        strcpy_s(textEditBuffer, sizeof(textEditBuffer), "UI Element");
    }
    ImGui::SameLine();
    if (ImGui::Button("Test Text")) {
        textElement->Text = "Test Text";
        strcpy_s(textEditBuffer, sizeof(textEditBuffer), "Test Text");
    }
}

void CanvasManager::selectElement(int canvasIndex, int elementIndex) {
    Canvas2D* canvas = getCurrentCanvas();
    if (!canvas || elementIndex < 0 || elementIndex >= static_cast<int>(canvas->RenderElements.size())) {
        selectedElement = nullptr;
        selectedElementIndex = -1;
        return;
    }
    
    selectedElement = canvas->RenderElements[elementIndex];
    selectedElementIndex = elementIndex;
    selectedCanvasIndex = canvasIndex;
    
    // Update text buffer if it's a text element
    UIText* textElement = dynamic_cast<UIText*>(selectedElement);
    if (textElement) {
        // Copy using modern C++ methods for better portability
        size_t copyLen = std::min(textElement->Text.length(), sizeof(textEditBuffer) - 1);
        std::memcpy(textEditBuffer, textElement->Text.c_str(), copyLen);
        textEditBuffer[copyLen] = '\0';
    }
    
    std::cout << "[CanvasManager] Selected element " << elementIndex << " in canvas " << canvasIndex << std::endl;
}

void CanvasManager::moveSelectedElement(float deltaX, float deltaY) {
    if (!selectedElement) return;
    
    UIText* textElement = dynamic_cast<UIText*>(selectedElement);
    if (textElement) {
        float newX = textElement->Position.x + deltaX;
        float newY = textElement->Position.y + deltaY;
        textElement->setPosition(newX, newY);
        
        if (deltaX != 0.0f || deltaY != 0.0f) {
            std::cout << "[CanvasManager] Moved element to (" << newX << ", " << newY << ")" << std::endl;
        }
    }
}

void CanvasManager::selectElementByClick(int canvasIndex, int elementIndex) {
    selectElement(canvasIndex, elementIndex);
}