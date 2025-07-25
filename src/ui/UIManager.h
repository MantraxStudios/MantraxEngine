#pragma once

#include "UIDemo.h"
#include <SDL.h>
#include <memory>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Disable copy constructor and copy assignment
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    
    // Enable move constructor and move assignment
    UIManager(UIManager&&) = default;
    UIManager& operator=(UIManager&&) = default;

    // Initialization
    bool initialize(SDL_Renderer* renderer);
    void cleanup();

    // Core functionality
    void render();
    void update(float deltaTime);
    bool handleEvent(const SDL_Event& event);

    // Demo control
    void toggleDemo() { if (uiDemo) uiDemo->toggleDemo(); }
    bool isDemoVisible() const { return uiDemo ? uiDemo->isDemoVisible() : false; }

    // State
    bool isInitialized() const { return initialized; }

private:
    std::unique_ptr<UIDemo> uiDemo;
    bool initialized = false;
}; 