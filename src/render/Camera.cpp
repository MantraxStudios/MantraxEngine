#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <algorithm>

Camera::Camera(float fov, float aspect, float nearPlane, float farPlane)
    : fov(fov), aspectRatio(aspect), nearClip(nearPlane), farClip(farPlane),
    position(glm::vec3(0.0f, 0.0f, 3.0f)),
    target(glm::vec3(0.0f, 0.0f, 0.0f)),
    up(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw(-90.0f), pitch(0.0f),
    projectionType(ProjectionType::Perspective),
    targetProjectionType(ProjectionType::Perspective),
    orthoSize(5.0f),
    transitionFactor(1.0f),
    transitionSpeed(2.0f), // Ajusta este valor para cambiar la velocidad de transición
    framebuffer(nullptr),
    framebufferEnabled(false),
    framebufferWidth(800),
    framebufferHeight(600)
{
    updateVectors();
}

Camera::~Camera() {
    if (framebuffer) {
        delete framebuffer;
        framebuffer = nullptr;
    }
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + getForward(), up);
}

glm::mat4 Camera::calculatePerspectiveMatrix() const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
}

glm::mat4 Camera::calculateOrthographicMatrix() const {
    float halfWidth = orthoSize * aspectRatio;
    return glm::ortho(-halfWidth, halfWidth, -orthoSize, orthoSize, nearClip, farClip);
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (transitionFactor >= 1.0f) {
        // Si no hay transición, devolver la matriz correspondiente
        return (projectionType == ProjectionType::Perspective) ? 
            calculatePerspectiveMatrix() : calculateOrthographicMatrix();
    }

    // Durante la transición, interpolar entre las dos matrices
    glm::mat4 fromMatrix = (targetProjectionType == ProjectionType::Perspective) ? 
        calculateOrthographicMatrix() : calculatePerspectiveMatrix();
    glm::mat4 toMatrix = (targetProjectionType == ProjectionType::Perspective) ? 
        calculatePerspectiveMatrix() : calculateOrthographicMatrix();

    // Interpolar cada elemento de la matriz
    glm::mat4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] = glm::mix(fromMatrix[i][j], toMatrix[i][j], transitionFactor);
        }
    }
    return result;
}

void Camera::setPosition(const glm::vec3& pos) {
    position = pos;
}

glm::vec3 Camera::getPosition() const {
    return position;
}

void Camera::setTarget(const glm::vec3& tgt) {
    target = tgt;
}

void Camera::setAspectRatio(float aspect) {
    aspectRatio = aspect;
}

void Camera::moveForward(float distance) {
    position += getForward() * distance;
}

void Camera::moveRight(float distance) {
    position += getRight() * distance;
}

void Camera::moveUp(float distance) {
    position += getUp() * distance;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw += yawDelta;
    pitch += pitchDelta;
    
    // Clamp pitch to prevent gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    updateVectors();
}

void Camera::setRotation(float newYaw, float newPitch) {
    yaw = newYaw;
    pitch = newPitch;
    
    // Clamp pitch to prevent gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    
    updateVectors();
}

glm::vec3 Camera::getForward() const {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    return glm::normalize(direction);
}

glm::vec3 Camera::getRight() const {
    return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 Camera::getUp() const {
    return glm::normalize(glm::cross(getRight(), getForward()));
}

void Camera::updateVectors() {
    // Vectors are calculated on-demand in getForward(), getRight(), getUp()
    // This method is called when rotation changes
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

Frustum Camera::getFrustum() const {
    Frustum frustum;
    frustum.extractFromMatrix(getViewProjectionMatrix());
    return frustum;
}

void Camera::setProjectionType(ProjectionType type, bool instant) {
    if (type != projectionType) {
        targetProjectionType = type;
        if (instant) {
            projectionType = type;
            transitionFactor = 1.0f;
        } else {
            transitionFactor = 0.0f;
        }
    }
}

void Camera::updateProjectionTransition(float deltaTime) {
    if (transitionFactor < 1.0f) {
        transitionFactor = std::min(transitionFactor + deltaTime * transitionSpeed, 1.0f);
        if (transitionFactor >= 1.0f) {
            projectionType = targetProjectionType;
        }
    }
}

void Camera::setOrthographicSize(float size) {
    orthoSize = size;
}

void Camera::enableFramebuffer(bool enabled) {
    if (enabled && !framebuffer) {
        // Create framebuffer if it doesn't exist
        framebuffer = new Framebuffer(framebufferWidth, framebufferHeight);
    } else if (!enabled && framebuffer) {
        // Delete framebuffer if disabling
        delete framebuffer;
        framebuffer = nullptr;
    }
    framebufferEnabled = enabled;
}

void Camera::setFramebufferSize(int width, int height) {
    framebufferWidth = width;
    framebufferHeight = height;
    
    // Resize existing framebuffer if it exists
    if (framebuffer && framebufferEnabled) {
        framebuffer->resize(width, height);
    }
}
