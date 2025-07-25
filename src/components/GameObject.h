#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>
#include <string>
#include "../render/NativeGeometry.h"
#include "../render/Material.h"
#include "../render/Frustum.h"
#include "Component.h"
#include "../core/CoreExporter.h"

// Forward declaration
class AssimpGeometry;

class MANTRAXCORE_API GameObject
{
public:
    GameObject(NativeGeometry* geometry);
    GameObject(NativeGeometry* geometry, std::shared_ptr<Material> material);
    GameObject(std::shared_ptr<NativeGeometry> geometry);
    GameObject(std::shared_ptr<NativeGeometry> geometry, std::shared_ptr<Material> material);
    
    // Constructor for AssimpGeometry specifically
    GameObject(std::shared_ptr<AssimpGeometry> geometry);
    GameObject(std::shared_ptr<AssimpGeometry> geometry, std::shared_ptr<Material> material);

    // Prohibir copia
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    // Permitir movimiento
    GameObject(GameObject&&) noexcept = default;
    GameObject& operator=(GameObject&&) noexcept = default;

    // Transform
    void setLocalPosition(const glm::vec3 &pos);
    void setLocalScale(const glm::vec3 &scl);
    void setLocalRotationEuler(const glm::vec3 &eulerDeg);
    void setLocalRotationQuat(const glm::quat &quat);

    glm::vec3 getLocalPosition() const;
    glm::vec3 getLocalScale() const;
    glm::quat getLocalRotationQuat() const;
    glm::vec3 getLocalRotationEuler() const;

    glm::mat4 getModelMatrix();
    NativeGeometry *getGeometry() const;
    
    // Material
    void setMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> getMaterial() const;
    
    // Bounding volumes para frustum culling (OPTIMIZADO)
    BoundingSphere getWorldBoundingSphere() const;
    BoundingBox getLocalBoundingBox() const;
    void setBoundingRadius(float radius);
    void calculateBoundingVolumes(); // Calcula bounding volumes de la geometría
    
    template <typename T, typename... Args>
    T *addComponent(Args &&...args)
    {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        comp->setOwner(this);
        T *rawPtr = comp.get();
        components.push_back(std::move(comp));
        return rawPtr;
    }

    template <typename T>
    T *getComponent()
    {
        for (auto &comp : components)
        {
            if (T *casted = dynamic_cast<T *>(comp.get()))
            {
                return casted;
            }
        }
        return nullptr;
    }

private:
    std::string Name = "New Object";
    std::string Tag = "Default";
    NativeGeometry *geometry;
    std::shared_ptr<NativeGeometry> sharedGeometry; // Para mantener referencia de modelos cargados
    std::shared_ptr<Material> material;
    glm::vec3 localPosition;
    glm::vec3 localScale;
    glm::quat localRotation;
    glm::mat4 modelMatrix;
    bool dirtyTransform;
    
    // Bounding volumes (OPTIMIZADO)
    BoundingBox localBoundingBox;
    float localBoundingRadius;
    mutable BoundingSphere cachedWorldBoundingSphere;
    mutable bool worldBoundingSphereDirty;
    
    void updateModelMatrix();

    std::vector<std::unique_ptr<Component>> components;
};
