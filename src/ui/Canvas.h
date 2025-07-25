#pragma once

#include "UIElement.h"
#include "Button.h"
#include "Text.h"
#include "Image.h"
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <utility>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API Canvas {
public:
    Canvas(SDL_Renderer* renderer);
    ~Canvas();
    
    // Disable copy constructor and copy assignment
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    
    // Enable move constructor and move assignment
    Canvas(Canvas&&) = default;
    Canvas& operator=(Canvas&&) = default;

    // Core functionality
    void render();
    void update(float deltaTime);
    bool handleEvent(const SDL_Event& event);
    
    // Element management
    Button* addButton(const glm::vec2& position, const glm::vec2& size, const std::string& text = "Button");
    Text* addText(const glm::vec2& position, const std::string& text = "Text");
    Image* addImage(const glm::vec2& position, const glm::vec2& size, const std::string& imagePath = "");
    
    // Element retrieval
    UIElement* getElementById(const std::string& id);
    Button* getButtonById(const std::string& id);
    Text* getTextById(const std::string& id);
    Image* getImageById(const std::string& id);
    
    // Element removal
    void removeElement(const std::string& id);
    void removeElement(UIElement* element);
    void clearAllElements();
    
    // Canvas properties
    void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void clearBackground(bool clear) { shouldClearBackground = clear; }
    
    // Utility
    size_t getElementCount() const { return elements.size(); }
    bool isEmpty() const { return elements.empty(); }

private:
    SDL_Renderer* renderer;
    std::vector<std::unique_ptr<UIElement>> elements;
    
    SDL_Color backgroundColor = {0, 0, 0, 0}; // Transparent by default
    bool shouldClearBackground = false;
    
    // ID management
    int nextElementId = 1;
    std::string generateElementId();
    
    // Helper methods
    template<typename T>
    T* findElementByType(const std::string& id) {
        for (const auto& element : elements) {
            if (element && element->getID() == id) {
                T* typed = dynamic_cast<T*>(element.get());
                if (typed) {
                    return typed;
                }
            }
        }
        return nullptr;
    }
}; 