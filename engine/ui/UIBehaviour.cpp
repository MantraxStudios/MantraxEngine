#include "UIBehaviour.h"
#include "Canvas.h"
#include "UIDragSystem.h"
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

// ==================== UIBehaviour Drag Implementation ====================

void UIBehaviour::enableDrag(bool enable) {
    if (dragEnabled != enable) {
        dragEnabled = enable;
        updateDragRegistration();
        std::cout << "[UIBehaviour] Drag " << (enable ? "enabled" : "disabled") << std::endl;
    }
}

void UIBehaviour::setOnDragStart(std::function<void(const UIDragEvent&)> callback) {
    dragStartCallback = callback;
}

void UIBehaviour::setOnDragUpdate(std::function<void(const UIDragEvent&)> callback) {
    dragUpdateCallback = callback;
}

void UIBehaviour::setOnDragEnd(std::function<void(const UIDragEvent&)> callback) {
    dragEndCallback = callback;
}

void UIBehaviour::setOnDragHover(std::function<void(const UIDragEvent&)> callback) {
    dragHoverCallback = callback;
}

void UIBehaviour::setDragConstraints(const glm::vec2& minPos, const glm::vec2& maxPos) {
    UIDragSystem::getInstance().setDragConstraints(this, minPos, maxPos);
}

void UIBehaviour::setConstrainToParent(bool constrain) {
    UIDragSystem::getInstance().setConstrainToParent(this, constrain);
}

glm::vec4 UIBehaviour::getBounds() const {
    // Implementación básica - debería ser sobrescrita por clases derivadas
    glm::vec2 pos = getPosition();
    return glm::vec4(pos.x, pos.y, width * 0.1f, 30.0f); // Estimación básica
}

glm::vec2 UIBehaviour::getPosition() const {
    // Implementación básica - debería ser sobrescrita por clases derivadas
    return glm::vec2(0.0f, 0.0f);
}

void UIBehaviour::setPosition(const glm::vec2& position) {
    // Implementación básica - debería ser sobrescrita por clases derivadas
    std::cout << "[UIBehaviour] Setting position to (" << position.x << ", " << position.y << ")" << std::endl;
}

void UIBehaviour::registerWithDragSystem() {
    if (!dragRegistered && dragEnabled) {
        DraggableInfo info;
        info.element = this;
        info.isDraggable = true;
        info.constrainToParent = true;
        
        // Configurar callbacks que llaman a los métodos virtuales y callbacks del usuario
        info.onDragStart = [this](const UIDragEvent& event) {
            onDragStarted(event);
            if (dragStartCallback) dragStartCallback(event);
        };
        
        info.onDragUpdate = [this](const UIDragEvent& event) {
            onDragUpdated(event);
            if (dragUpdateCallback) dragUpdateCallback(event);
        };
        
        info.onDragEnd = [this](const UIDragEvent& event) {
            onDragEnded(event);
            if (dragEndCallback) dragEndCallback(event);
        };
        
        info.onDragHover = [this](const UIDragEvent& event) {
            onDragHovered(event);
            if (dragHoverCallback) dragHoverCallback(event);
        };
        
        UIDragSystem::getInstance().registerDraggable(this, info);
        dragRegistered = true;
        
        std::cout << "[UIBehaviour] Registered with drag system" << std::endl;
    }
}

void UIBehaviour::unregisterFromDragSystem() {
    if (dragRegistered) {
        UIDragSystem::getInstance().unregisterDraggable(this);
        dragRegistered = false;
        
        std::cout << "[UIBehaviour] Unregistered from drag system" << std::endl;
    }
}

void UIBehaviour::updateDragRegistration() {
    if (dragEnabled && !dragRegistered) {
        registerWithDragSystem();
    } else if (!dragEnabled && dragRegistered) {
        unregisterFromDragSystem();
    }
}