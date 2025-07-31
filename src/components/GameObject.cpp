#define GLM_ENABLE_EXPERIMENTAL
#include "GameObject.h"
#include "../render/AssimpGeometry.h"
#include "../render/ModelLoader.h"
#include "../core/FileSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>
#include <iostream>
#include <limits>

// Constructor por defecto para objetos vacíos
GameObject::GameObject()
    : geometry(nullptr), sharedGeometry(nullptr), material(nullptr),
    localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
    localBoundingRadius(0.5f), worldBoundingSphereDirty(true), ModelPath(""),
    isDestroyed(false) {
    calculateBoundingVolumes();
}

// Constructor con path de modelo (carga automática)
GameObject::GameObject(const std::string& modelPath)
    : geometry(nullptr), sharedGeometry(nullptr), material(nullptr),
    localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
    localBoundingRadius(0.5f), worldBoundingSphereDirty(true), ModelPath(modelPath) {
    calculateBoundingVolumes();
    loadModelFromPath(); // Intentar cargar el modelo automáticamente
}

GameObject::GameObject(const std::string& modelPath, std::shared_ptr<Material> mat)
    : geometry(nullptr), sharedGeometry(nullptr), material(mat),
    localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
    localBoundingRadius(0.5f), worldBoundingSphereDirty(true), ModelPath(modelPath) {
    calculateBoundingVolumes();
    loadModelFromPath(); // Intentar cargar el modelo automáticamente
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry)
    : geometry(geometry.get()), sharedGeometry(geometry), material(nullptr),
    localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
    localBoundingRadius(0.5f), worldBoundingSphereDirty(true), ModelPath("") {
    calculateBoundingVolumes();
}

GameObject::GameObject(std::shared_ptr<AssimpGeometry> geometry, std::shared_ptr<Material> mat)
    : geometry(geometry.get()), sharedGeometry(geometry), material(mat),
    localPosition(0.0f), localScale(1.0f), localRotation(1.0f, 0.0f, 0.0f, 0.0f),
    dirtyLocalTransform(true), dirtyWorldTransform(true), parent(nullptr),
    localBoundingRadius(0.5f), worldBoundingSphereDirty(true), ModelPath("") {
    calculateBoundingVolumes();
}

GameObject::~GameObject() {
    cleanup();
}

void GameObject::destroy() {
    if (!isDestroyed) {
        isDestroyed = true;
        cleanup();
    }
}

void GameObject::cleanup() {
    // Primero destruir todos los hijos
    for (auto* child : children) {
        if (child) {
            child->destroy();
        }
    }

    children.clear();

    // Desvincularse del padre
    removeFromParent();

    // Limpiar componentes
    components.clear();

    // Limpiar referencias
    geometry = nullptr;
    sharedGeometry = nullptr;
    material = nullptr;
}

void GameObject::setLocalPosition(const glm::vec3& pos) {
    localPosition = pos;
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalScale(const glm::vec3& scl) {
    localScale = scl;
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalRotationEuler(const glm::vec3& eulerDeg) {
    localRotation = glm::quat(glm::radians(eulerDeg));
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

void GameObject::setLocalRotationQuat(const glm::quat& quat) {
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

void GameObject::setWorldPosition(const glm::vec3& pos) {
    if (parent) {
        glm::mat4 parentWorldMatrix = parent->getWorldModelMatrix();
        glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
        glm::vec4 localPos = parentWorldInverse * glm::vec4(pos, 1.0f);
        setLocalPosition(glm::vec3(localPos));
    }
    else {
        setLocalPosition(pos);
    }
}

void GameObject::setWorldScale(const glm::vec3& scl) {
    if (parent) {
        glm::vec3 parentWorldScale = parent->getWorldScale();
        glm::vec3 localScl = scl / parentWorldScale;
        setLocalScale(localScl);
    }
    else {
        setLocalScale(scl);
    }
}

void GameObject::setWorldRotationEuler(const glm::vec3& eulerDeg) {
    glm::quat worldRot = glm::quat(glm::radians(eulerDeg));
    setWorldRotationQuat(worldRot);
}

void GameObject::setWorldRotationQuat(const glm::quat& quat) {
    if (parent) {
        glm::quat parentWorldRot = parent->getWorldRotationQuat();
        glm::quat localRot = glm::inverse(parentWorldRot) * quat;
        setLocalRotationQuat(localRot);
    }
    else {
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
    }
    else {
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
    }
    else {
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

glm::mat4 GameObject::getLocalModelMatrix() const {
    if (dirtyLocalTransform) {
        updateLocalModelMatrix();
    }
    return localModelMatrix;
}

glm::mat4 GameObject::getWorldModelMatrix() const {
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
    if (isDestroyed || (newParent && newParent->isDestroyed)) return;
    if (newParent == this || isChildOf(newParent)) return;

    // 1. Guardar la transformación mundial actual ANTES de hacer cualquier cambio
    glm::vec3 worldPos = getWorldPosition();
    glm::quat worldRot = getWorldRotationQuat();
    glm::vec3 worldScale = getWorldScale();

    // 2. Remover del padre actual y agregar al nuevo padre
    removeFromParent();
    addToParent(newParent);

    // 3. Calcular la nueva transformación local para preservar la transformación mundial
    if (newParent) {
        // Obtener transformaciones del nuevo padre
        glm::vec3 parentWorldPos = newParent->getWorldPosition();
        glm::quat parentWorldRot = newParent->getWorldRotationQuat();
        glm::vec3 parentWorldScale = newParent->getWorldScale();

        // Calcular transformación local relativa al nuevo padre
        // Local = Parent^-1 * World

        // Posición: Local = InverseParentRotation * (WorldPos - ParentPos) / ParentScale
        glm::vec3 relativePos = worldPos - parentWorldPos;
        relativePos = glm::inverse(parentWorldRot) * relativePos;
        localPosition = relativePos / parentWorldScale;

        // Rotación: Local = InverseParentRotation * WorldRotation
        localRotation = glm::inverse(parentWorldRot) * worldRot;

        // Escala: Local = WorldScale / ParentScale
        localScale = worldScale / parentWorldScale;
    }
    else {
        // Si no hay padre, la transformación local es igual a la mundial
        localPosition = worldPos;
        localRotation = worldRot;
        localScale = worldScale;
    }

    // 4. Normalizar quaternion y marcar las transformaciones como dirty
    localRotation = glm::normalize(localRotation);
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

// Método que preserva completamente la matriz de transformación mundial
void GameObject::setParentPreserveWorldMatrix(GameObject* newParent) {
    if (isDestroyed || (newParent && newParent->isDestroyed)) return;
    if (newParent == this || isChildOf(newParent)) return;

    // 1. Guardar la matriz de transformación mundial completa ANTES del cambio
    glm::mat4 currentWorldMatrix = getWorldModelMatrix();

    // 2. Cambiar el padre sin preservar transformaciones
    removeFromParent();
    addToParent(newParent);

    // 3. Calcular la nueva transformación local para mantener la misma matriz mundial
    if (newParent) {
        // LocalMatrix = ParentWorldMatrix^-1 * DesiredWorldMatrix
        glm::mat4 parentWorldMatrix = newParent->getWorldModelMatrix();
        glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
        glm::mat4 newLocalMatrix = parentWorldInverse * currentWorldMatrix;

        // Descomponer la nueva matriz local en componentes
        glm::vec3 newLocalPos, newLocalScale;
        glm::quat newLocalRot;
        decomposeMatrixRobust(newLocalMatrix, newLocalPos, newLocalRot, newLocalScale);

        // Aplicar los nuevos valores locales
        localPosition = newLocalPos;
        localRotation = glm::normalize(newLocalRot);
        localScale = newLocalScale;
    }
    else {
        // Si no hay padre, descomponer la matriz mundial directamente
        decomposeMatrixRobust(currentWorldMatrix, localPosition, localRotation, localScale);
        localRotation = glm::normalize(localRotation);
    }

    // 4. Marcar las transformaciones como dirty
    dirtyLocalTransform = true;
    invalidateWorldTransform();
}

// Alternativa más simple que reutiliza el método setParent existente
void GameObject::setParentPreserveWorldMatrixSimple(GameObject* newParent) {
    // Tu método setParent() ya hace exactamente esto:
    // preserva la transformación mundial completa
    setParent(newParent);
}

void GameObject::setParentNoWorldPreserve(GameObject* newParent) {
    if (isDestroyed || (newParent && newParent->isDestroyed)) return;
    if (newParent == this || isChildOf(newParent)) return;

    removeFromParent();
    addToParent(newParent);
    invalidateWorldTransform();
}

void GameObject::setParentPreserveWorldPosition(GameObject* newParent) {
    if (isDestroyed || (newParent && newParent->isDestroyed)) return;
    if (newParent == this || isChildOf(newParent)) return;

    glm::vec3 worldPos = getWorldPosition();

    removeFromParent();
    addToParent(newParent);

    setWorldPosition(worldPos);
}

void GameObject::decomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale) {
    // Extraer posición (columna 3)
    position = glm::vec3(matrix[3]);

    // Extraer vectores de las columnas de rotación/escala
    glm::vec3 col0 = glm::vec3(matrix[0]);
    glm::vec3 col1 = glm::vec3(matrix[1]);
    glm::vec3 col2 = glm::vec3(matrix[2]);

    // Calcular escalas (longitud de cada columna)
    scale.x = glm::length(col0);
    scale.y = glm::length(col1);
    scale.z = glm::length(col2);

    // Verificar si hay escalado negativo (determinante negativo)
    float det = glm::determinant(glm::mat3(matrix));
    if (det < 0) {
        scale.x = -scale.x;
    }

    // Normalizar las columnas para obtener la matriz de rotación pura
    if (scale.x != 0.0f) col0 /= scale.x;
    if (scale.y != 0.0f) col1 /= scale.y;
    if (scale.z != 0.0f) col2 /= scale.z;

    // Construir matriz de rotación 3x3
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = col0;
    rotationMatrix[1] = col1;
    rotationMatrix[2] = col2;

    // Asegurar que la matriz sea ortogonal (corregir errores de precisión)
    // Método Gram-Schmidt simplificado
    rotationMatrix[0] = glm::normalize(rotationMatrix[0]);
    rotationMatrix[1] = glm::normalize(rotationMatrix[1] - glm::dot(rotationMatrix[1], rotationMatrix[0]) * rotationMatrix[0]);
    rotationMatrix[2] = glm::cross(rotationMatrix[0], rotationMatrix[1]);

    // Convertir matriz de rotación a quaternion
    rotation = glm::quat_cast(rotationMatrix);

    // Normalizar el quaternion para evitar errores de precisión
    rotation = glm::normalize(rotation);
}

void GameObject::decomposeMatrixRobust(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale) {
    glm::vec3 skew;
    glm::vec4 perspective;

    if (glm::decompose(matrix, scale, rotation, position, skew, perspective)) {
        // glm::decompose fue exitoso
        rotation = glm::normalize(rotation);
    }
    else {
        // Fallback al método manual si glm::decompose falla
        decomposeMatrix(matrix, position, rotation, scale);
    }
}

void GameObject::addChild(GameObject* child) {
    if (isDestroyed || !child || child->isDestroyed) return;
    if (child != this && !isParentOf(child)) {
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

void GameObject::updateLocalModelMatrix() const {
    if (!shouldUpdateTransform) return;

    // Orden correcto: Translate * Rotate * Scale (TRS)
    glm::mat4 T = glm::translate(glm::mat4(1.0f), localPosition);
    glm::mat4 R = glm::toMat4(localRotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), localScale);

    localModelMatrix = T * R * S;
    dirtyLocalTransform = false;
}

void GameObject::updateWorldModelMatrix() const {
    if (!shouldUpdateTransform) return;

    if (dirtyLocalTransform) {
        updateLocalModelMatrix();
    }

    if (parent) {
        // CRÍTICO: El orden correcto es ParentWorld * LocalModel
        // Esto aplica primero la transformación local, luego la del padre
        glm::mat4 parentWorldMatrix = parent->getWorldModelMatrix();
        worldModelMatrix = parentWorldMatrix * localModelMatrix;
    }
    else {
        worldModelMatrix = localModelMatrix;
    }

    dirtyWorldTransform = false;
}

void GameObject::update(float deltaTime) {
    if (isDestroyed) return;

    // Actualizar todos los componentes
    for (auto& comp : components) {
        if (comp) {
            comp->update();
        }
    }

    // Actualizar la transformación si es necesario
    if (shouldUpdateTransform && dirtyWorldTransform) {
        invalidateWorldTransform();
    }
}

AssimpGeometry* GameObject::getGeometry() const {
    return geometry;
}

void GameObject::setGeometry(std::shared_ptr<AssimpGeometry> geom) {
    geometry = geom.get();
    sharedGeometry = geom;
    calculateBoundingVolumes();
}

void GameObject::setModelPath(const std::string& path) {
    ModelPath = path;
}

bool GameObject::loadModelFromPath() {
    if (ModelPath.empty()) {
        std::cerr << "GameObject::loadModelFromPath: No model path set for object '" << Name << "'" << std::endl;
        return false;
    }
    return loadModelFromPath(ModelPath);
}

bool GameObject::loadModelFromPath(const std::string& path) {
    if (path.empty()) {
        std::cerr << "GameObject::loadModelFromPath: Empty path provided for object '" << Name << "'" << std::endl;
        return false;
    }

    std::cout << "Loading model for GameObject '" << Name << "' from path: " << path << std::endl;

    // Usar el ModelLoader singleton para cargar el modelo
    auto& modelLoader = ModelLoader::getInstance();
    auto loadedModel = modelLoader.loadModel(FileSystem::getProjectPath() + "\\" + path);

    std::cout << "Model Path: " << loadedModel << std::endl;

    if (loadedModel) {
        // Asignar la geometría cargada
        setGeometry(loadedModel);
        ModelPath = path; // Actualizar el path guardado
        std::cout << "Successfully loaded model for GameObject '" << Name << "'" << std::endl;
        return true;
    }
    else {
        std::cerr << "Failed to load model for GameObject '" << Name << "' from path: " << path << std::endl;
        return false;
    }
}

void GameObject::setMaterial(std::shared_ptr<Material> mat) {
    material = mat;
}

std::shared_ptr<Material> GameObject::getMaterial() const {
    return material;
}

void GameObject::calculateBoundingVolumes() {
    if (geometry) {
        // Si hay geometría, usar sus bounding volumes
        glm::vec3 min = geometry->getBoundingBoxMin();
        glm::vec3 max = geometry->getBoundingBoxMax();
        localBoundingBox = BoundingBox(min, max);

        // Calcular radio basado en el bounding box
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 halfSize = (max - min) * 0.5f;
        localBoundingRadius = glm::length(halfSize);
    }
    else {
        // Si no hay geometría, usar valores por defecto para un objeto vacío
        glm::vec3 halfSize = glm::vec3(0.1f); // Tamaño más pequeño para objetos vacíos
        localBoundingBox = BoundingBox(-halfSize, halfSize);
        localBoundingRadius = glm::length(halfSize);
    }

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

BoundingBox GameObject::getWorldBoundingBox() const {
    // Transformar el bounding box local al espacio de mundo
    glm::mat4 worldMatrix = getWorldModelMatrix();

    // Obtener los 8 vértices del bounding box local
    glm::vec3 min = localBoundingBox.min;
    glm::vec3 max = localBoundingBox.max;

    std::vector<glm::vec3> localVertices = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z)
    };

    // Transformar todos los vértices al espacio de mundo
    glm::vec3 worldMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 worldMax = glm::vec3(-std::numeric_limits<float>::max());

    for (const auto& vertex : localVertices) {
        glm::vec4 worldVertex = worldMatrix * glm::vec4(vertex, 1.0f);
        glm::vec3 worldPos = glm::vec3(worldVertex);

        worldMin = glm::min(worldMin, worldPos);
        worldMax = glm::max(worldMax, worldPos);
    }

    return BoundingBox(worldMin, worldMax);
}

glm::vec3 GameObject::getWorldBoundingBoxMin() const {
    return getWorldBoundingBox().min;
}

glm::vec3 GameObject::getWorldBoundingBoxMax() const {
    return getWorldBoundingBox().max;
}

GameObject* GameObject::getSelfObject() {
    return this;
}