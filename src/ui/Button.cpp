#include "Button.h"
#include <iostream>

Button::Button(const glm::vec2& position, const glm::vec2& size, const std::string& text)
    : UIElement(UIElementType::BUTTON, position, size), buttonText(text) {
}

void Button::render(SDL_Renderer* renderer) {
    if (!isVisible) return;

    // Get current background color based on state
    SDL_Color currentColor = getCurrentBackgroundColor();
    
    // Set render color
    SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    
    // Draw button background
    SDL_Rect rect = getRect();
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Render text (simple implementation without SDL_ttf)
    renderText(renderer);
}

void Button::update(float deltaTime) {
    // Update any animations or state changes here
}

bool Button::handleEvent(const SDL_Event& event) {
    if (!isVisible || !isEnabled) return false;

    switch (event.type) {
        case SDL_MOUSEMOTION: {
            glm::vec2 mousePos(event.motion.x, event.motion.y);
            bool wasHovered = hovered;
            hovered = isPointInside(mousePos);
            
            if (!wasHovered && hovered && onHoverCallback) {
                onHoverCallback();
            }
            return hovered;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                glm::vec2 mousePos(event.button.x, event.button.y);
                if (isPointInside(mousePos)) {
                    pressed = true;
                    wasPressed = true;
                    if (onPressCallback) {
                        onPressCallback();
                    }
                    return true;
                }
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (pressed && wasPressed) {
                    glm::vec2 mousePos(event.button.x, event.button.y);
                    if (isPointInside(mousePos) && onClickCallback) {
                        onClickCallback();
                    }
                }
                pressed = false;
                wasPressed = false;
                return true;
            }
            break;
        }
    }
    
    return false;
}

void Button::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    backgroundColor = {r, g, b, a};
}

void Button::setHoverColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    hoverColor = {r, g, b, a};
}

void Button::setPressedColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    pressedColor = {r, g, b, a};
}

void Button::setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    textColor = {r, g, b, a};
}

SDL_Color Button::getCurrentBackgroundColor() const {
    if (pressed) return pressedColor;
    if (hovered) return hoverColor;
    return backgroundColor;
}

void Button::renderText(SDL_Renderer* renderer) {
    // Simple text rendering placeholder
    // For now, we'll just render a small rectangle in the center to represent text
    // In a full implementation, you'd use SDL_ttf here
    
    if (buttonText.empty()) return;
    
    SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, textColor.a);
    
    // Calculate text area (centered)
    int textWidth = static_cast<int>(buttonText.length() * 8); // Rough estimate: 8px per character
    int textHeight = 16; // Fixed height
    
    int textX = static_cast<int>(position.x + (size.x - textWidth) / 2);
    int textY = static_cast<int>(position.y + (size.y - textHeight) / 2);
    
    // Draw simple text placeholder (series of small rectangles)
    for (size_t i = 0; i < buttonText.length(); ++i) {
        SDL_Rect charRect = {
            textX + static_cast<int>(i * 8),
            textY,
            6, // char width
            textHeight
        };
        SDL_RenderFillRect(renderer, &charRect);
    }
} 