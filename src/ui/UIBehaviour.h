#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "../core/CoreExporter.h"

// Forward declarations
struct UIDragEvent;
class UIDragSystem;

// Anchor enum for UI positioning
enum MANTRAXCORE_API Anchor {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

// UI Transform class for handling UI positioning and scaling
class MANTRAXCORE_API UITransform {
public:
    UITransform();
    UITransform(float x, float y, float width, float height);
    
    // Position
    void setPosition(float x, float y);
    void setPosition(const glm::vec2& position);
    glm::vec2 getPosition() const;
    
    // Size
    void setSize(float width, float height);
    void setSize(const glm::vec2& size);
    glm::vec2 getSize() const;
    
    // Scale
    void setScale(float x, float y);
    void setScale(const glm::vec2& scale);
    glm::vec2 getScale() const;
    
    // Anchor
    void setAnchor(Anchor anchor);
    Anchor getAnchor() const;
    
    // Get world position based on anchor
    glm::vec2 getWorldPosition() const;
    
    // Get bounds
    glm::vec4 getBounds() const; // x, y, width, height
    
private:
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 scale;
    Anchor anchor;
};


class MANTRAXCORE_API UIBehaviour {
public:
    GLuint textShaderProgram;
    GLint textOrthoLoc, textModelLoc, textSamplerLoc;
    GLint orthoLoc, modelLoc, colorLoc;
    GLuint quadVAO, quadVBO;

    float width;
    float height;

    Anchor UIAnchor;
    
    // Lifecycle methods
    virtual void start() { }
    virtual void update() { }
    
    void setTransform(UITransform* transform);
    UITransform* getTransform() const;
    
    // Input handling
    void handleMouseInput(float mouseX, float mouseY, bool mousePressed);
    
    // Drag & Drop functionality
    void enableDrag(bool enable = true);
    void disableDrag() { enableDrag(false); }
    bool isDragEnabled() const { return dragEnabled; }
    
    // Drag callbacks
    void setOnDragStart(std::function<void(const UIDragEvent&)> callback);
    void setOnDragUpdate(std::function<void(const UIDragEvent&)> callback);
    void setOnDragEnd(std::function<void(const UIDragEvent&)> callback);
    void setOnDragHover(std::function<void(const UIDragEvent&)> callback);
    
    // Drag constraints
    void setDragConstraints(const glm::vec2& minPos, const glm::vec2& maxPos);
    void setConstrainToParent(bool constrain);
    
    // Get element bounds for drag detection
    virtual glm::vec4 getBounds() const;
    virtual glm::vec2 getPosition() const;
    virtual void setPosition(const glm::vec2& position);
    
protected:
    // Called when drag events occur (can be overridden)
    virtual void onDragStarted(const UIDragEvent& event) {}
    virtual void onDragUpdated(const UIDragEvent& event) {}
    virtual void onDragEnded(const UIDragEvent& event) {}
    virtual void onDragHovered(const UIDragEvent& event) {}
    
private:
    UITransform* transform;
    
    // Input state
    float lastMouseX, lastMouseY;
    bool lastMousePressed;
    
    // Drag state
    bool dragEnabled = false;
    bool dragRegistered = false;
    
    // Drag callbacks
    std::function<void(const UIDragEvent&)> dragStartCallback;
    std::function<void(const UIDragEvent&)> dragUpdateCallback;
    std::function<void(const UIDragEvent&)> dragEndCallback;
    std::function<void(const UIDragEvent&)> dragHoverCallback;
    
    // Internal drag methods
    void registerWithDragSystem();
    void unregisterFromDragSystem();
    void updateDragRegistration();
    
    friend class UIDragSystem;
}; 