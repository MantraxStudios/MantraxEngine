#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "Frustum.h"
#include "Framebuffer.h"
#include "../core/CoreExporter.h"

enum class MANTRAXCORE_API ProjectionType {
    Perspective,
    Orthographic
};

class MANTRAXCORE_API Camera {
public:
    Camera(float fov, float aspect, float nearPlane, float farPlane);
    ~Camera();

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void setPosition(const glm::vec3& pos);
    glm::vec3 getPosition() const;
    void setTarget(const glm::vec3& target);
    void setAspectRatio(float aspect);

    // Camera movement methods
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float yaw, float pitch);
    void setRotation(float yaw, float pitch);
    
    // Getters for camera orientation
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;
    
    // Frustum culling support
    Frustum getFrustum() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // Projection type control with transition
    void setProjectionType(ProjectionType type, bool instant = false);
    ProjectionType getProjectionType() const { return projectionType; }
    void updateProjectionTransition(float deltaTime);
    bool isTransitioning() const { return transitionFactor < 1.0f; }
    
    // Orthographic parameters
    void setOrthographicSize(float size);
    float getOrthographicSize() const { return orthoSize; }
    
    // Getters para parámetros de la cámara
    float getFOV() const { return fov; }
    float getAspectRatio() const { return aspectRatio; }
    float getNearClip() const { return nearClip; }
    float getFarClip() const { return farClip; }
    
    // Framebuffer management
    void enableFramebuffer(bool enabled);
    bool isFramebufferEnabled() const { return framebufferEnabled; }
    void setFramebufferSize(int width, int height);
    Framebuffer* getFramebuffer() const { return framebuffer; }
    
    // Buffer size getters
    int getBufferWidth() const { return framebufferEnabled ? framebufferWidth : 0; }
    int getBufferHeight() const { return framebufferEnabled ? framebufferHeight : 0; }
    std::pair<int, int> getBufferSize() const { return { getBufferWidth(), getBufferHeight() }; }

    void update(float deltaTime);

    // Audio-related getters (uppercase for distinction)
    glm::vec3 GetVelocity() const { return m_velocity; }
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_lastPosition{0.0f};
    glm::vec3 m_velocity{0.0f};

    float fov;
    float aspectRatio;
    float nearClip;
    float farClip;
    
    // Camera rotation
    float yaw;
    float pitch;
    
    // Projection parameters
    ProjectionType projectionType;
    ProjectionType targetProjectionType;
    float orthoSize;
    
    // Transition parameters
    float transitionFactor;
    float transitionSpeed;
    
    // Framebuffer parameters
    Framebuffer* framebuffer;
    bool framebufferEnabled;
    int framebufferWidth;
    int framebufferHeight;
    
    void updateVectors();
    glm::mat4 calculatePerspectiveMatrix() const;
    glm::mat4 calculateOrthographicMatrix() const;
};
