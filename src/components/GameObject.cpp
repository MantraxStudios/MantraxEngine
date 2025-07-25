#define GLM_ENABLE_EXPERIMENTAL
#include "GameObject.h"
#include "../render/AssimpGeometry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

GameObject::GameObject(NativeGeometry *geometry)
    : geometry(geometry), sharedGeometry(nullptr), material(nullptr), localPosition(0.0f), localScale(1.0f), 
      localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true), 
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(NativeGeometry *geometry, std::shared_ptr<Material> mat)
    : geometry(geometry), sharedGeometry(nullptr), material(mat), localPosition(0.0f), localScale(1.0f), 
      localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<NativeGeometry> geometry)
    : geometry(geometry.get()), sharedGeometry(geometry), material(nullptr), localPosition(0.0f), localScale(1.0f), 
      localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<NativeGeometry> geometry, std::shared_ptr<Material> mat)
    : geometry(geometry.get()), sharedGeometry(geometry), material(mat), localPosition(0.0f), localScale(1.0f), 
      localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry)
    : geometry(geometry.get()), sharedGeometry(std::static_pointer_cast<NativeGeometry>(geometry)), material(nullptr), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry, std::shared_ptr<Material> mat)
    : geometry(geometry.get()), sharedGeometry(std::static_pointer_cast<NativeGeometry>(geometry)), material(mat), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), dirtyTransform(true),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

void GameObject::setLocalPosition(const glm::vec3 &pos) {
    localPosition = pos;
    dirtyTransform = true;
    worldBoundingSphereDirty = true;
}

void GameObject::setLocalScale(const glm::vec3 &scl) {
    localScale = scl;
    dirtyTransform = true;
    worldBoundingSphereDirty = true;
}

void GameObject::setLocalRotationEuler(const glm::vec3 &eulerDeg) {
    localRotation = glm::quat(glm::radians(eulerDeg));
    dirtyTransform = true;
    worldBoundingSphereDirty = true;
}

void GameObject::setLocalRotationQuat(const glm::quat &quat) {
    localRotation = quat;
    dirtyTransform = true;
    worldBoundingSphereDirty = true;
}

glm::vec3 GameObject::getLocalPosition() const {
    return localPosition;
}

glm::vec3 GameObject::getLocalScale() const {
    return localScale;
}

glm::quat GameObject::getLocalRotationQuat() const {
    return localRotation;
}

glm::vec3 GameObject::getLocalRotationEuler() const {
    return glm::degrees(glm::eulerAngles(localRotation));
}

glm::mat4 GameObject::getModelMatrix() {
    if (dirtyTransform) {
        updateModelMatrix();
    }
    return modelMatrix;
}

NativeGeometry *GameObject::getGeometry() const {
    return geometry;
}

void GameObject::setMaterial(std::shared_ptr<Material> mat) {
    material = mat;
}

std::shared_ptr<Material> GameObject::getMaterial() const {
    return material;
}

void GameObject::updateModelMatrix() {
    modelMatrix = glm::translate(glm::mat4(1.0f), localPosition) *
                  glm::toMat4(localRotation) *
                  glm::scale(glm::mat4(1.0f), localScale);
    dirtyTransform = false;
}

// Implementaciones OPTIMIZADAS de bounding volumes
void GameObject::calculateBoundingVolumes() {
    // Calcular bounding box local simple
    glm::vec3 halfSize = glm::vec3(0.5f); // Tamaño base
    localBoundingBox = BoundingBox(-halfSize, halfSize);
    
    // Calcular radio de bounding sphere local (diagonal de la caja)
    localBoundingRadius = glm::length(halfSize);
    
    // Marcar world bounding sphere como dirty
    worldBoundingSphereDirty = true;
}

BoundingBox GameObject::getLocalBoundingBox() const {
    return localBoundingBox;
}

BoundingSphere GameObject::getWorldBoundingSphere() const {
    // CACHE OPTIMIZADO - Solo recalcular si es necesario
    if (worldBoundingSphereDirty) {
        // Transformar solo el centro (mucho más rápido que 8 esquinas)
        glm::vec3 worldCenter = localPosition;
        
        // Calcular el radio mundial basado en la escala máxima
        float maxScale = glm::max(glm::max(localScale.x, localScale.y), localScale.z);
        float worldRadius = localBoundingRadius * maxScale;
        
        // Cachear resultado
        cachedWorldBoundingSphere = BoundingSphere(worldCenter, worldRadius);
        worldBoundingSphereDirty = false;
    }
    
    return cachedWorldBoundingSphere;
}

void GameObject::setBoundingRadius(float radius) {
    localBoundingRadius = radius;
    worldBoundingSphereDirty = true;
}
