#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <physx/PxPhysicsAPI.h>

#include "../render/Material.h"
#include "../render/Frustum.h"
#include "../core/CoreExporter.h"
#include "../core/UIDGenerator.h"
#include "../render/AssimpGeometry.h"
#include "Component.h"

// Layer constants for physics
#define LAYER_0 (1 << 0)
#define LAYER_1 (1 << 1)
#define LAYER_2 (1 << 2)
#define LAYER_3 (1 << 3)
#define LAYER_4 (1 << 4)
#define LAYER_5 (1 << 5)
#define LAYER_6 (1 << 6)
#define LAYER_7 (1 << 7)
#define LAYER_8 (1 << 8)
#define LAYER_9 (1 << 9)
#define LAYER_10 (1 << 10)
#define LAYER_11 (1 << 11)
#define LAYER_12 (1 << 12)
#define LAYER_13 (1 << 13)
#define LAYER_14 (1 << 14)
#define LAYER_15 (1 << 15)
#define LAYER_16 (1 << 16)
#define LAYER_17 (1 << 17)
#define LAYER_18 (1 << 18)
#define LAYER_19 (1 << 19)
#define LAYER_TRIGGER (1 << 20)
#define LAYER_PLAYER (1 << 21)
#define LAYER_ENEMY (1 << 22)
#define LAYER_ENVIRONMENT (1 << 23)

// Forward declaration
class AssimpGeometry;

class MANTRAXCORE_API GameObject
{
public:
    // Constructor por defecto para objetos vacíos
    GameObject();
    
    // Constructor con path de modelo (carga automática)
    GameObject(const std::string& modelPath);
    GameObject(const std::string& modelPath, std::shared_ptr<Material> material);
    
    // Constructor for AssimpGeometry specifically
    GameObject(std::shared_ptr<AssimpGeometry> geometry);
    GameObject(std::shared_ptr<AssimpGeometry> geometry, std::shared_ptr<Material> material);

    // Prohibir copia
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    // Permitir movimiento
    GameObject(GameObject&&) noexcept = default;
    GameObject& operator=(GameObject&&) noexcept = default;

    // Destructor virtual para asegurar la correcta destrucción de clases derivadas
    virtual ~GameObject();

    // Método para destruir el objeto de forma segura
    void destroy();

    // Validación del objeto
    bool isValid() const { return !isDestroyed && ObjectID != ""; }

    // ===== TRANSFORM SYSTEM =====
    // Local Transform (relative to parent)
    void setLocalPosition(const glm::vec3 &pos);
    void setLocalScale(const glm::vec3 &scl);
    void setLocalRotationEuler(const glm::vec3 &eulerDeg);
    void setLocalRotationQuat(const glm::quat &quat);

    glm::vec3 getLocalPosition() const;
    glm::vec3 getLocalScale() const;
    glm::quat getLocalRotationQuat() const;
    glm::vec3 getLocalRotationEuler() const;

    // World Transform (absolute in world space)
    void setWorldPosition(const glm::vec3 &pos);
    void setWorldScale(const glm::vec3 &scl);
    void setWorldRotationEuler(const glm::vec3 &eulerDeg);
    void setWorldRotationQuat(const glm::quat &quat);

    glm::vec3 getWorldPosition() const;
    glm::vec3 getWorldScale() const;
    glm::quat getWorldRotationQuat() const;
    glm::vec3 getWorldRotationEuler() const;

    // Matrix operations
    glm::mat4 getLocalModelMatrix() const;
    glm::mat4 getWorldModelMatrix() const;
    glm::mat4 getWorldToLocalMatrix() const;

    // ===== HIERARCHY SYSTEM =====
    // Parent operations
    void setParent(GameObject* newParent);
    GameObject* getParent() const { return parent; }
    bool hasParent() const { return parent != nullptr; }
    
    // Child operations
    void addChild(GameObject* child);
    void removeChild(GameObject* child);
    const std::vector<GameObject*>& getChildren() const { return children; }
    int getChildCount() const { return static_cast<int>(children.size()); }
    GameObject* getChild(int index) const;
    
    // Hierarchy traversal
    GameObject* findChild(const std::string& name) const;
    GameObject* findChildRecursive(const std::string& name) const;
    std::vector<GameObject*> getAllChildren() const;
    
    // Hierarchy validation
    bool isChildOf(const GameObject* potentialParent) const;
    bool isParentOf(const GameObject* potentialChild) const;
    bool isInHierarchy(const GameObject* root) const;

    // ===== EXISTING FUNCTIONALITY =====
    AssimpGeometry *getGeometry() const;
    
    // Métodos para asignar geometría después de la creación
    void setGeometry(std::shared_ptr<AssimpGeometry> geometry);
    bool hasGeometry() const { return geometry != nullptr; }
    
    // Métodos para carga automática de modelos
    void setModelPath(const std::string& path);
    bool loadModelFromPath();
    bool loadModelFromPath(const std::string& path);
    const std::string& getModelPath() const { return ModelPath; }
    
    // Material
    void setMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> getMaterial() const;

    // Render control methods
    void setRenderEnabled(bool enable) { shouldRender = enable; }
    bool isRenderEnabled() const { return shouldRender; }
    void setTransformUpdateEnabled(bool enable) { shouldUpdateTransform = enable; }
    bool isTransformUpdateEnabled() const { return shouldUpdateTransform; }
    
    // Bounding volumes para frustum culling (OPTIMIZADO)
    BoundingSphere getWorldBoundingSphere() const;
    BoundingBox getLocalBoundingBox() const;
    BoundingBox getWorldBoundingBox() const;
    glm::vec3 getWorldBoundingBoxMin() const;
    glm::vec3 getWorldBoundingBoxMax() const;
    void setBoundingRadius(float radius);
    void calculateBoundingVolumes(); // Calcula bounding volumes de la geometría

    // Layer configuration for physics
    physx::PxU32 Layer = LAYER_0;
    physx::PxU32 LayerMask = LAYER_0 | LAYER_1 | LAYER_2 | LAYER_3 | LAYER_4 | LAYER_5 | LAYER_6 | LAYER_7 | LAYER_8 | LAYER_9 | LAYER_10 | LAYER_11 | LAYER_12 | LAYER_13 | LAYER_14 | LAYER_15 | LAYER_16 | LAYER_17 | LAYER_18 | LAYER_19 | LAYER_TRIGGER | LAYER_PLAYER | LAYER_ENEMY | LAYER_ENVIRONMENT;
    
    // Getters and setters for layer configuration
    physx::PxU32 getLayer() const { return Layer; }
    void setLayer(physx::PxU32 layer) { Layer = layer; }
    physx::PxU32 getLayerMask() const { return LayerMask; }
    void setLayerMask(physx::PxU32 layerMask) { LayerMask = layerMask; }

    // Sistema de componentes
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->setOwner(this);
        T* rawPtr = comp.get();
        components.push_back(std::move(comp));
        
        // Ejecutar start() automáticamente cuando se añade el componente
        if (rawPtr && rawPtr->isActive()) {
            rawPtr->start();
        }
        
        return rawPtr;
    }

    template<typename T>
    T* getComponent() {
        for (auto& comp : components) {
            if (T* casted = dynamic_cast<T*>(comp.get())) {
                return casted;
            }
        }
        return nullptr;
    }

    template<typename T>
    const T* getComponent() const {
        for (const auto& comp : components) {
            if (const T* casted = dynamic_cast<const T*>(comp.get())) {
                return casted;
            }
        }
        return nullptr;
    }

    template <typename T>
    bool removeComponent() {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto it = std::find_if(components.begin(), components.end(),
            [](const std::unique_ptr<Component>& comp) {
                return dynamic_cast<T*>(comp.get()) != nullptr;
            });
        
        if (it != components.end()) {
            components.erase(it);
            return true;
        }
        return false;
    }

    // Obtener todos los componentes
    std::vector<const Component*> getAllComponents() const {
        std::vector<const Component*> result;
        result.reserve(components.size());
        for (const auto& comp : components) {
            if (comp) {
                result.push_back(comp.get());
            }
        }
        return result;
    }

    // Update method
    void update(float deltaTime);

    std::string Name = "New Object";
    std::string Tag = "Default";
    std::string ObjectID = std::to_string(UIDGenerator::Generate());
    std::string ModelPath = ""; // Path del modelo a cargar automáticamente
    AssimpGeometry *geometry;
    std::shared_ptr<AssimpGeometry> sharedGeometry; // Para mantener referencia de modelos cargados
    std::shared_ptr<Material> material;
    
    // Transform data
    glm::vec3 localPosition;
    glm::vec3 localScale;
    glm::quat localRotation;
    mutable glm::mat4 localModelMatrix;
    mutable glm::mat4 worldModelMatrix;
    mutable bool dirtyLocalTransform;
    mutable bool dirtyWorldTransform;
    
    // Hierarchy data
    GameObject* parent;
    std::vector<GameObject*> children;
    
    // Bounding volumes (OPTIMIZADO)
    BoundingBox localBoundingBox;
    float localBoundingRadius;
    mutable BoundingSphere cachedWorldBoundingSphere;
    mutable bool worldBoundingSphereDirty;

private:
    void updateLocalModelMatrix() const;
    void updateWorldModelMatrix() const;
    void updateChildrenTransforms();
    void removeFromParent();
    void addToParent(GameObject* newParent);
    void invalidateWorldTransform();
    void cleanup(); // Método interno para limpieza

    std::vector<std::unique_ptr<Component>> components;
    bool shouldRender{true};
    bool shouldUpdateTransform{true};
    bool isDestroyed{false}; // Variable para tracking de estado
};
