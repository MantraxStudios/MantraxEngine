#pragma once

#include "UIElement.h"
#include <string>
#include <functional>

class MANTRAXCORE_API Button : public UIElement {
public:
    Button(const glm::vec2& position, const glm::vec2& size, const std::string& text = "Button");
    virtual ~Button() = default;

    // Core UIElement overrides
    void render(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    bool handleEvent(const SDL_Event& event) override;

    // Button-specific methods
    void setText(const std::string& text) { buttonText = text; }
    std::string getText() const { return buttonText; }
    
    // Colors
    void setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setHoverColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setPressedColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setTextColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    
    // Events
    void setOnClick(std::function<void()> callback) { onClickCallback = callback; }
    void setOnHover(std::function<void()> callback) { onHoverCallback = callback; }
    void setOnPress(std::function<void()> callback) { onPressCallback = callback; }

    // State
    bool isHovered() const { return hovered; }
    bool isPressed() const { return pressed; }

private:
    std::string buttonText;
    
    // Colors (RGBA)
    SDL_Color backgroundColor = {100, 100, 100, 255};
    SDL_Color hoverColor = {130, 130, 130, 255};
    SDL_Color pressedColor = {70, 70, 70, 255};
    SDL_Color textColor = {255, 255, 255, 255};
    
    // State
    bool hovered = false;
    bool pressed = false;
    bool wasPressed = false; // For click detection
    
    // Callbacks
    std::function<void()> onClickCallback;
    std::function<void()> onHoverCallback;
    std::function<void()> onPressCallback;
    
    // Helper methods
    SDL_Color getCurrentBackgroundColor() const;
    void renderText(SDL_Renderer* renderer);
}; 