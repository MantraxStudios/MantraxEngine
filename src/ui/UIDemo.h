#pragma once

#include "Canvas.h"
#include <SDL.h>
#include <memory>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API UIDemo {
public:
    UIDemo();
    ~UIDemo();
    
    // Disable copy constructor and copy assignment
    UIDemo(const UIDemo&) = delete;
    UIDemo& operator=(const UIDemo&) = delete;
    
    // Enable move constructor and move assignment
    UIDemo(UIDemo&&) = default;
    UIDemo& operator=(UIDemo&&) = default;

    // Initialization
    bool initialize(SDL_Renderer* renderer);
    void cleanup();

    // Core functionality
    void render();
    void update(float deltaTime);
    bool handleEvent(const SDL_Event& event);

    // Demo control
    void toggleDemo() { showDemo = !showDemo; }
    bool isDemoVisible() const { return showDemo; }

private:
    std::unique_ptr<Canvas> canvas;
    bool showDemo = true;
    
    // UI Elements (stored as pointers for easy access)
    Button* testButton1 = nullptr;
    Button* testButton2 = nullptr;
    Button* toggleButton = nullptr;
    Text* titleText = nullptr;
    Text* infoText = nullptr;
    Image* testImage = nullptr;
    
    // Demo state
    int clickCount = 0;
    
    // Setup methods
    void setupUI();
    void setupButtons();
    void setupTexts();
    void setupImages();
    
    // Event handlers
    void onButton1Click();
    void onButton2Click();
    void onToggleClick();
}; 