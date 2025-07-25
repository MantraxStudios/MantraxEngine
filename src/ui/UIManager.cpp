#include "UIManager.h"
#include <iostream>

UIManager::UIManager() {
}

UIManager::~UIManager() {
    cleanup();
}

bool UIManager::initialize(SDL_Renderer* renderer) {
    if (!renderer) {
        std::cerr << "UIManager::initialize() - Renderer is null!" << std::endl;
        return false;
    }

    // Create UI demo
    uiDemo = std::make_unique<UIDemo>();
    if (!uiDemo->initialize(renderer)) {
        std::cerr << "UIManager::initialize() - Failed to initialize UIDemo!" << std::endl;
        return false;
    }

    initialized = true;
    std::cout << "UIManager::initialize() - UI Manager initialized successfully" << std::endl;
    return true;
}

void UIManager::cleanup() {
    if (uiDemo) {
        uiDemo->cleanup();
        uiDemo.reset();
    }
    initialized = false;
}

void UIManager::render() {
    if (initialized && uiDemo) {
        uiDemo->render();
    }
}

void UIManager::update(float deltaTime) {
    if (initialized && uiDemo) {
        uiDemo->update(deltaTime);
    }
}

bool UIManager::handleEvent(const SDL_Event& event) {
    if (initialized && uiDemo) {
        return uiDemo->handleEvent(event);
    }
    return false;
} 