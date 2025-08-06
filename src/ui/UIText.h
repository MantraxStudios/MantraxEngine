#pragma once

#include <string>
#include <glm/glm.hpp>
#include <functional>
#include "../ui/UIBehaviour.h" // For Anchor enum

class Canvas2D; // Forward declaration

class UIText {
public:
    UIText(const std::string& name, const std::string& text, float x, float y, Anchor anchor = Anchor::TopLeft);
    ~UIText() = default;

    // Drawing and updating
    void draw(Canvas2D* canvas);
    void update();

    // Text properties
    void setText(const std::string& text);
    void setColor(const glm::vec4& color);
    void setFontSize(int size);
    void setTextScale(float scale);
    void setPosition(float x, float y);
    void setVisible(bool visible);
    void setEnabled(bool enabled);

    // Getters
    const std::string& getText() const { return text; }
    const glm::vec4& getColor() const { return color; }
    int getFontSize() const { return fontSize; }
    float getTextScale() const { return textScale; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    bool isVisible() const { return visible; }
    bool isEnabled() const { return enabled; }
    Anchor getAnchor() const { return anchor; }
    const std::string& getName() const { return name; }

    // Event callbacks
    void setOnTextChanged(std::function<void(const std::string&)> callback);

private:
    std::string name;
    std::string text;
    float x, y;
    float width, height;
    glm::vec4 color;
    int fontSize;
    float textScale;
    bool visible;
    bool enabled;
    Anchor anchor;
    std::function<void(const std::string&)> onTextChanged;
}; 