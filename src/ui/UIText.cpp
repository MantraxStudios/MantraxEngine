#include "UIText.h"
#include "../ui/Canvas.h" // To access Canvas2D's drawing methods

UIText::UIText(const std::string& name, const std::string& text, float x, float y, Anchor anchor)
    : name(name), text(text), x(x), y(y), width(0), height(0),
      color(1.0f, 1.0f, 1.0f, 1.0f), fontSize(32), textScale(1.0f),
      visible(true), enabled(true), anchor(anchor) {
}

void UIText::draw(Canvas2D* canvas) {
    if (!canvas || !visible || !enabled) return;
    
    // Draw the text and get its dimensions
    auto [textWidth, textHeight] = canvas->drawText(text, x, y, color, anchor);
    width = static_cast<float>(textWidth);
    height = static_cast<float>(textHeight);
}

void UIText::update() {
    // No specific update logic for static text
    // This method can be overridden for animated text or other dynamic behavior
}

void UIText::setText(const std::string& newText) {
    if (text != newText) {
        text = newText;
        if (onTextChanged) {
            onTextChanged(text);
        }
    }
}

void UIText::setColor(const glm::vec4& newColor) {
    color = newColor;
}

void UIText::setFontSize(int size) {
    fontSize = size;
}

void UIText::setTextScale(float scale) {
    textScale = scale;
}

void UIText::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void UIText::setVisible(bool isVisible) {
    visible = isVisible;
}

void UIText::setEnabled(bool isEnabled) {
    enabled = isEnabled;
}

void UIText::setOnTextChanged(std::function<void(const std::string&)> callback) {
    onTextChanged = callback;
} 