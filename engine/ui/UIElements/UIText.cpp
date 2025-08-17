#include "UIText.h"
#include "../UIDragSystem.h"
#include <iostream>

// Constructor
// UIText::UIText() {} // If needed later

// Destructor 
// UIText::~UIText() {} // If needed later

// ==================== Drag System Implementation ====================

glm::vec4 UIText::getBounds() const {
    auto [textWidth, textHeight] = calculateTextSize(Text);
    return glm::vec4(Position.x, Position.y, static_cast<float>(textWidth), static_cast<float>(textHeight));
}

glm::vec2 UIText::getPosition() const {
    return Position;
}

void UIText::setPosition(const glm::vec2& position) {
    setPosition(position.x, position.y);
}

void UIText::onDragStarted(const UIDragEvent& event) {
    std::cout << "[UIText] Drag started on text: " << Text << std::endl;
}

void UIText::onDragUpdated(const UIDragEvent& event) {
    // Update position during drag
    setPosition(event.currentPosition);
    std::cout << "[UIText] Dragging text to: " << event.currentPosition.x << ", " << event.currentPosition.y << std::endl;
}

void UIText::onDragEnded(const UIDragEvent& event) {
    std::cout << "[UIText] Drag ended on text: " << Text << std::endl;
}

// Additional helper methods could go here if needed
