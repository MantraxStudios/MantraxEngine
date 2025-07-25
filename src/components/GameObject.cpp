#define GLM_ENABLE_EXPERIMENTAL
#include "GameObject.h"
#include "../render/AssimpGeometry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>

GameObject::GameObject(NativeGeometry *geometry)
    : geometry(geometry), sharedGeometry(nullptr), material(nullptr), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(NativeGeometry *geometry, std::shared_ptr<Material> mat)
    : geometry(geometry), sharedGeometry(nullptr), material(mat), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<NativeGeometry> geometry)
    : geometry(geometry.get()), sharedGeometry(geometry), material(nullptr), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<NativeGeometry> geometry, std::shared_ptr<Material> mat)
    : geometry(geometry.get()), sharedGeometry(geometry), material(mat), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry)
    : geometry(geometry.get()), sharedGeometry(std::static_pointer_cast<NativeGeometry>(geometry)), material(nullptr), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry, std::shared_ptr<Material> mat)
    : geometry(geometry.get()), sharedGeometry(std::static_pointer_cast<NativeGeometry>(geometry)), material(mat), 
      localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f), 
      dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
      localBoundingRadius(0.5f), worldBoundingSphereDirty(true) {
    calculateBoundingVolumes();
}

void GameObject::setLocalPosition(const glm::vec3 &pos) {
    localPosition = pos;
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalScale(const glm::vec3 &scl) {
    localScale = scl;
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalRotationEuler(const glm::vec3 &eulerDeg) {
    localRotation = glm::quat(glm::radians(eulerDeg));
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalRotationQuat(const glm::quat &quat) {
    localRotation = quat;
    dirtyLocalTransform = true;
    invalidateWorldTransform();
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

void GameObject::setWorldPosition(const glm::vec3 &pos) {
    if (parent) {
        glm::mat4 parentWorldMatrix = parent->getWorldModelMatrix();
        glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
        glm::vec4 localPos = parentWorldInverse * glm::vec4(pos, 1.0f);
        setLocalPosition(glm::vec3(localPos));
    } else {
        setLocalPosition(pos);
    }
}

void GameObject::setWorldScale(const glm::vec3 &scl) {
    if (parent) {
        glm::vec3 parentWorldScale = parent->getWorldScale();
        glm::vec3 localScl = scl / parentWorldScale;
        setLocalScale(localScl);
    } else {
        setLocalScale(scl);
    }
}

void GameObject::setWorldRotationEuler(const glm::vec3 &eulerDeg) {
    glm::quat worldRot = glm::quat(glm::radians(eulerDeg));
    setWorldRotationQuat(worldRot);
}

void GameObject::setWorldRotationQuat(const glm::quat &quat) {
    if (parent) {
        glm::quat parentWorldRot = parent->getWorldRotationQuat();
        glm::quat localRot = glm::inverse(parentWorldRot) * quat;
        setLocalRotationQuat(localRot);
    } else {
        setLocalRotationQuat(quat);
    }
}

glm::vec3 GameObject::getWorldPosition() const {
    if (dirtyWorldTransform) {
        const_cast<GameObject*>(this)->updateWorldModelMatrix();
    }
    return glm::vec3(worldModelMatrix[3]);
}

glm::vec3 GameObject::getWorldScale() const {
    if (dirtyWorldTransform) {
        const_cast<GameObject*>(this)->updateWorldModelMatrix();
    }
    
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    
    if (glm::decompose(worldModelMatrix, scale, rotation, translation, skew, perspective)) {
        return scale;
    } else {
        scale.x = glm::length(glm::vec3(worldModelMatrix[0]));
        scale.y = glm::length(glm::vec3(worldModelMatrix[1]));
        scale.z = glm::length(glm::vec3(worldModelMatrix[2]));
        return scale;
    }
}

glm::quat GameObject::getWorldRotationQuat() const {
    if (dirtyWorldTransform) {
        const_cast<GameObject*>(this)->updateWorldModelMatrix();
    }
    
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    
    if (glm::decompose(worldModelMatrix, scale, rotation, translation, skew, perspective)) {
        return rotation;
    } else {
        glm::mat3 rotationMatrix = glm::mat3(worldModelMatrix);
        for (int i = 0; i < 3; ++i) {
            rotationMatrix[i] = glm::normalize(rotationMatrix[i]);
        }
        return glm::quat_cast(rotationMatrix);
    }
}

glm::vec3 GameObject::getWorldRotationEuler() const {
    return glm::degrees(glm::eulerAngles(getWorldRotationQuat()));
}

glm::mat4 GameObject::getLocalModelMatrix() {
    if (dirtyLocalTransform) {
        updateLocalModelMatrix();
    }
    return localModelMatrix;
}

glm::mat4 GameObject::getWorldModelMatrix() {
    if (dirtyWorldTransform) {
        updateWorldModelMatrix();
    }
    return worldModelMatrix;
}

glm::mat4 GameObject::getWorldToLocalMatrix() const {
    if (parent) {
        return glm::inverse(parent->getWorldModelMatrix());
    }
    return glm::mat4(1.0f);
}

void GameObject::setParent(GameObject* newParent) {
    if (newParent == this || isChildOf(newParent)) {
        return;
    }
    
    removeFromParent();
    
    addToParent(newParent);
    
    invalidateWorldTransform();
}

void GameObject::addChild(GameObject* child) {
    if (child && child != this && !isParentOf(child)) {
        child->setParent(this);
    }
}

void GameObject::removeChild(GameObject* child) {
    if (child && child->parent == this) {
        child->setParent(nullptr);
    }
}

GameObject* GameObject::getChild(int index) const {
    if (index >= 0 && index < static_cast<int>(children.size())) {
        return children[index];
    }
    return nullptr;
}

GameObject* GameObject::findChild(const std::string& name) const {
    for (GameObject* child : children) {
        if (child->Name == name) {
            return child;
        }
    }
    return nullptr;
}

GameObject* GameObject::findChildRecursive(const std::string& name) const {
    GameObject* result = findChild(name);
    if (result) {
        return result;
    }
    
    for (GameObject* child : children) {
        result = child->findChildRecursive(name);
        if (result) {
            return result;
        }
    }
    
    return nullptr;
}

std::vector<GameObject*> GameObject::getAllChildren() const {
    std::vector<GameObject*> allChildren;
    for (GameObject* child : children) {
        allChildren.push_back(child);
        std::vector<GameObject*> grandChildren = child->getAllChildren();
        allChildren.insert(allChildren.end(), grandChildren.begin(), grandChildren.end());
    }
    return allChildren;
}

bool GameObject::isChildOf(const GameObject* potentialParent) const {
    const GameObject* current = parent;
    while (current) {
        if (current == potentialParent) {
            return true;
        }
        current = current->parent;
    }
    return false;
}

bool GameObject::isParentOf(const GameObject* potentialChild) const {
    return potentialChild && potentialChild->isChildOf(this);
}

bool GameObject::isInHierarchy(const GameObject* root) const {
    if (this == root) {
        return true;
    }
    return isChildOf(root);
}

void GameObject::removeFromParent() {
    if (parent) {
        auto& parentChildren = parent->children;
        parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), this), parentChildren.end());
        parent = nullptr;
    }
}

void GameObject::addToParent(GameObject* newParent) {
    if (newParent) {
        parent = newParent;
        parent->children.push_back(this);
    }
}

void GameObject::invalidateWorldTransform() {
    dirtyWorldTransform = true;
    worldBoundingSphereDirty = true;
    updateChildrenTransforms();
}

void GameObject::updateChildrenTransforms() {
    for (GameObject* child : children) {
        child->invalidateWorldTransform();
    }
}

void GameObject::updateLocalModelMatrix() {
    localModelMatrix = glm::translate(glm::mat4(1.0f), localPosition) *
                      glm::toMat4(localRotation) *
                      glm::scale(glm::mat4(1.0f), localScale);
    dirtyLocalTransform = false;
}

void GameObject::updateWorldModelMatrix() {
    if (dirtyLocalTransform) {
        updateLocalModelMatrix();
    }
    
    if (parent) {
        worldModelMatrix = parent->getWorldModelMatrix() * localModelMatrix;
    } else {
        worldModelMatrix = localModelMatrix;
    }
    
    dirtyWorldTransform = false;
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

void GameObject::calculateBoundingVolumes() {
    glm::vec3 halfSize = glm::vec3(0.5f);
    localBoundingBox = BoundingBox(-halfSize, halfSize);
    
    localBoundingRadius = glm::length(halfSize);
    
    worldBoundingSphereDirty = true;
}

BoundingBox GameObject::getLocalBoundingBox() const {
    return localBoundingBox;
}

BoundingSphere GameObject::getWorldBoundingSphere() const {
    if (worldBoundingSphereDirty) {
        glm::vec3 worldCenter = getWorldPosition();
        
        float maxScale = glm::max(glm::max(localScale.x, localScale.y), localScale.z);
        float worldRadius = localBoundingRadius * maxScale;
        
        cachedWorldBoundingSphere = BoundingSphere(worldCenter, worldRadius);
        worldBoundingSphereDirty = false;
    }
    
    return cachedWorldBoundingSphere;
}

void GameObject::setBoundingRadius(float radius) {
    localBoundingRadius = radius;
    worldBoundingSphereDirty = true;
}
