#include "UIBehaviour.h"
#include "Canvas.h"
#include <iostream>

// ==================== UITransform Implementation ====================

void UITransform::setPosition(float x, float y) {
    position = glm::vec2(x, y);
}

void UITransform::setPosition(const glm::vec2& pos) {
    position = pos;
}

glm::vec2 UITransform::getPosition() const {
    return position;
}

void UITransform::setSize(float width, float height) {
    size = glm::vec2(width, height);
}

void UITransform::setSize(const glm::vec2& s) {
    size = s;
}

glm::vec2 UITransform::getSize() const {
    return size;
}

void UITransform::setScale(float x, float y) {
    scale = glm::vec2(x, y);
}

void UITransform::setScale(const glm::vec2& s) {
    scale = s;
}

glm::vec2 UITransform::getScale() const {
    return scale;
}

void UITransform::setAnchor(Anchor a) {
    anchor = a;
}

Anchor UITransform::getAnchor() const {
    return anchor;
}

glm::vec2 UITransform::getWorldPosition() const {
    // For now, just return the position
    // TODO: Implement anchor-based positioning
    return position;
}

glm::vec4 UITransform::getBounds() const {
    return glm::vec4(position.x, position.y, size.x, size.y);
}