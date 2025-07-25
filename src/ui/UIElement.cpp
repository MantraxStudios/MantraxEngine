#include "UIElement.h"

UIElement::UIElement(UIElementType type, const glm::vec2& position, const glm::vec2& size)
    : type(type), position(position), size(size) {
}

bool UIElement::isPointInside(const glm::vec2& point) const {
    return point.x >= position.x && 
           point.x <= position.x + size.x &&
           point.y >= position.y && 
           point.y <= position.y + size.y;
}

SDL_Rect UIElement::getRect() const {
    return SDL_Rect{
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        static_cast<int>(size.x),
        static_cast<int>(size.y)
    };
} 