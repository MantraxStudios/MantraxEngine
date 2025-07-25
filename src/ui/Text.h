#pragma once

#include "UIElement.h"
#include <string>

enum class TextAlignment {
    LEFT,
    CENTER,
    RIGHT
};

class MANTRAXCORE_API Text : public UIElement {
public:
    Text(const glm::vec2& position, const std::string& text = "Text");
    virtual ~Text() = default;

    // Core UIElement overrides
    void render(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;

    // Text-specific methods
    void setText(const std::string& text);
    std::string getText() const { return textContent; }
    
    // Appearance
    void setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setFontSize(int size) { fontSize = size; updateSize(); }
    void setAlignment(TextAlignment align) { alignment = align; }
    
    // Auto-sizing
    void setAutoSize(bool autoSize) { useAutoSize = autoSize; updateSize(); }
    bool getAutoSize() const { return useAutoSize; }

private:
    std::string textContent;
    SDL_Color textColor = {255, 255, 255, 255};
    int fontSize = 16;
    TextAlignment alignment = TextAlignment::LEFT;
    bool useAutoSize = true;
    
    // Helper methods
    void updateSize();
    void renderSimpleText(SDL_Renderer* renderer);
    void renderCharacter(SDL_Renderer* renderer, char c, float x, float y, float w, float h);
    glm::vec2 calculateTextPosition() const;
}; 