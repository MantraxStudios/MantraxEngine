#pragma once

#include <SDL.h>
#include <glm/glm.hpp>
#include <string>
#include <functional>
#include "../core/CoreExporter.h"

enum class UIElementType {
    BUTTON,
    TEXT,
    IMAGE
};

class MANTRAXCORE_API UIElement {
public:
    UIElement(UIElementType type, const glm::vec2& position, const glm::vec2& size);
    virtual ~UIElement() = default;

    // Core methods
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void update(float deltaTime) {}
    virtual bool handleEvent(const SDL_Event& event) { return false; }

    // Position and size
    virtual void setPosition(const glm::vec2& pos) { position = pos; }
    virtual void setSize(const glm::vec2& s) { size = s; }
    virtual glm::vec2 getPosition() const { return position; }
    virtual glm::vec2 getSize() const { return size; }
    
    // Utility methods
    virtual bool isPointInside(const glm::vec2& point) const;
    virtual SDL_Rect getRect() const;
    
    // Visibility and state
    virtual void setVisible(bool visible) { isVisible = visible; }
    virtual bool getVisible() const { return isVisible; }
    virtual void setEnabled(bool enabled) { isEnabled = enabled; }
    virtual bool getEnabled() const { return isEnabled; }

    // Type and ID
    UIElementType getType() const { return type; }
    void setID(const std::string& id) { elementID = id; }
    std::string getID() const { return elementID; }

protected:
    UIElementType type;
    glm::vec2 position;
    glm::vec2 size;
    bool isVisible = true;
    bool isEnabled = true;
    std::string elementID;
}; 