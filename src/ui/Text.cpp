#include "Text.h"
#include <algorithm>

Text::Text(const glm::vec2& position, const std::string& text)
    : UIElement(UIElementType::TEXT, position, glm::vec2(0, 0)), textContent(text) {
    updateSize();
}

void Text::render(SDL_Renderer* renderer) {
    if (!isVisible || textContent.empty()) return;
    
    renderSimpleText(renderer);
}

void Text::update(float deltaTime) {
    // Update any text animations here
}

void Text::setText(const std::string& text) {
    textContent = text;
    updateSize();
}

void Text::setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    textColor = {r, g, b, a};
}

void Text::updateSize() {
    if (useAutoSize) {
        // Calculate size based on text content and font size
        float charWidth = fontSize * 0.6f; // Approximate character width
        float lineHeight = static_cast<float>(fontSize);
        
        size.x = textContent.length() * charWidth;
        size.y = lineHeight;
    }
}

void Text::renderSimpleText(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, textColor.a);
    
    glm::vec2 textPos = calculateTextPosition();
    
    // Simple character-by-character rendering
    float charWidth = fontSize * 0.6f;
    float charHeight = static_cast<float>(fontSize);
    
    for (size_t i = 0; i < textContent.length(); ++i) {
        char c = textContent[i];
        
        if (c == ' ') {
            // Skip spaces
            continue;
        }
        
        // Calculate character position
        float charX = textPos.x + i * charWidth;
        float charY = textPos.y;
        
        // Render character as a simple pattern based on the character
        // This is a very basic implementation - in real use you'd use SDL_ttf
        renderCharacter(renderer, c, charX, charY, charWidth, charHeight);
    }
}

glm::vec2 Text::calculateTextPosition() const {
    glm::vec2 textPos = position;
    
    switch (alignment) {
        case TextAlignment::CENTER:
            textPos.x = position.x + (size.x - textContent.length() * fontSize * 0.6f) * 0.5f;
            break;
        case TextAlignment::RIGHT:
            textPos.x = position.x + size.x - textContent.length() * fontSize * 0.6f;
            break;
        case TextAlignment::LEFT:
        default:
            // Already set to position.x
            break;
    }
    
    return textPos;
}

void Text::renderCharacter(SDL_Renderer* renderer, char c, float x, float y, float w, float h) {
    // Very simple character rendering - just different patterns for different characters
    // In a real implementation, you'd use proper font rendering
    
    SDL_Rect charRect = {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(w),
        static_cast<int>(h)
    };
    
    // Different patterns for different character types
    if (c >= 'A' && c <= 'Z') {
        // Capital letters - full rectangle
        SDL_RenderFillRect(renderer, &charRect);
    } else if (c >= 'a' && c <= 'z') {
        // Lowercase letters - smaller rectangle
        charRect.y += static_cast<int>(h * 0.2f);
        charRect.h = static_cast<int>(h * 0.6f);
        SDL_RenderFillRect(renderer, &charRect);
    } else if (c >= '0' && c <= '9') {
        // Numbers - outline rectangle
        SDL_RenderDrawRect(renderer, &charRect);
    } else {
        // Other characters - small dot
        charRect.x += static_cast<int>(w * 0.4f);
        charRect.y += static_cast<int>(h * 0.4f);
        charRect.w = static_cast<int>(w * 0.2f);
        charRect.h = static_cast<int>(h * 0.2f);
        SDL_RenderFillRect(renderer, &charRect);
    }
} 