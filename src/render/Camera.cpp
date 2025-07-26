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

void Camera::update(float deltaTime) {
    // Protección contra deltaTime muy pequeño o cero
    const float MIN_DELTA_TIME = 0.001f; // 1ms
    if (deltaTime < MIN_DELTA_TIME) {
        deltaTime = MIN_DELTA_TIME;
    }

    // Calcular velocidad con suavizado
    const float VELOCITY_SMOOTHING = 0.3f; // Ajusta este valor entre 0 y 1
    glm::vec3 currentVelocity = (position - m_lastPosition) / deltaTime;
    m_velocity = glm::mix(m_velocity, currentVelocity, VELOCITY_SMOOTHING);
    
    // Limitar la magnitud de la velocidad para evitar valores extremos
    const float MAX_VELOCITY = 1000.0f; // Ajusta según las unidades de tu mundo
    float velocityLength = glm::length(m_velocity);
    if (velocityLength > MAX_VELOCITY) {
        m_velocity = (m_velocity / velocityLength) * MAX_VELOCITY;
    }

    m_lastPosition = position;
    
    updateProjectionTransition(deltaTime);
    updateVectors();
}

void Camera::updateVectors() {
    // Calcular la dirección forward basada en los ángulos de rotación
    m_forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    m_forward.y = sin(glm::radians(pitch));
    m_forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    m_forward = glm::normalize(m_forward);

    // Recalcular el vector right usando el up world como referencia
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    m_right = glm::normalize(glm::cross(m_forward, worldUp));

    // Recalcular el vector up para asegurar ortogonalidad
    m_up = glm::normalize(glm::cross(m_right, m_forward));
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

glm::vec3 Camera::getForward() const {
    return m_forward;
}

glm::vec3 Camera::getRight() const {
    return m_right;
}

glm::vec3 Camera::getUp() const {
    return m_up;
}

glm::vec3 Camera::GetForward() const {
    return m_forward;
}

glm::vec3 Camera::GetRight() const {
    return m_right;
}

glm::vec3 Camera::GetUp() const {
    return m_up;
}
