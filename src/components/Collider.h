#pragma once
#include <physx/PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include "Component.h"
#include "GameObject.h"
#include "../core/CoreExporter.h"
#include "../core/PhysicsManager.h"
#include "../core/PhysicsEvents.h"
#include "PhysicalObject.h"
#include <functional>

class MANTRAXCORE_API GameObject;
class MANTRAXCORE_API Rigidbody;

class MANTRAXCORE_API Collider : public Component {
private:
    // PhysX objects
    physx::PxShape* shape;
    physx::PxMaterial* material;
    physx::PxRigidStatic* staticActor; // For independent colliders
    
    bool transformNeedsUpdate; // Flag to track if transform is dirty
    
    // Shape properties
    ShapeType shapeType;
    glm::vec3 boxHalfExtents;
    float sphereRadius;
    float capsuleRadius;
    float capsuleHalfHeight;
    
    // Material properties
    float friction;
    float restitution;
    
    // Trigger properties
    bool isTriggerShape;
    
    // Event callbacks
    std::function<void(Collider*, Collider*)> triggerCallback;
    std::function<void(Collider*, Collider*, const glm::vec3&, const glm::vec3&, float)> contactCallback;
    
    // Layer configuration (Word0/Word1)
    physx::PxU32 currentLayer;      // Word0: What type of object this is
    physx::PxU32 currentLayerMask;  // Word1: What this object can collide with
    
    bool initialized;

public:
    Collider(GameObject* obj);
    ~Collider();

    std::string getComponentName() const override {
        return "Collider";
    }
    
    // Component overrides
    void defines() override;
    void start() override;
    void update() override;
    void destroy() override;
    void deserialize(const std::string& data) override;
    std::string serializeComponent() const override;

    // Manual initialization
    void initializeCollider();
    
    // Shape creation
    void createShape();
    void verifyTriggerSetup();
    void configureShapeFlags();
    void createIndependentActorIfNeeded();
    
    // Transform synchronization for independent actors
    void syncTransformToPhysX();
    void updateActorTransform(); // For checking and updating only when needed
    
    // Shape recreation for shared shape issues (like PhysicalObject)
    void recreateShapeSafely();
    
    // Getters
    physx::PxShape* getShape() const { return shape; }
    physx::PxMaterial* getMaterial() const { return material; }
    
    // Material properties
    void setFriction(float newFriction);
    float getFriction() const { return friction; }
    
    void setRestitution(float newRestitution);
    float getRestitution() const { return restitution; }
    
    // Shape properties
    void setShapeType(ShapeType type);
    ShapeType getShapeType() const { return shapeType; }
    
    // Trigger properties
    void setTrigger(bool isTrigger);
    bool isTrigger() const { return isTriggerShape; }
    
    void setBoxHalfExtents(const glm::vec3& extents);
    glm::vec3 getBoxHalfExtents() const { return boxHalfExtents; }
    
    void setSphereRadius(float radius);
    float getSphereRadius() const { return sphereRadius; }
    
    void setCapsuleRadius(float radius);
    float getCapsuleRadius() const { return capsuleRadius; }
    
    void setCapsuleHalfHeight(float halfHeight);
    float getCapsuleHalfHeight() const { return capsuleHalfHeight; }
    
    // Trigger and contact events
    void setTriggerCallback(std::function<void(Collider*, Collider*)> callback);
    void setContactCallback(std::function<void(Collider*, Collider*, const glm::vec3&, const glm::vec3&, float)> callback);
    
    // Collision filters
    void setCollisionGroup(CollisionGroup group);
    void setCollisionMask(CollisionMask mask);
    void setupCollisionFilters(CollisionGroup group, CollisionMask mask);
    
    // Dynamic layer configuration (Word0/Word1)
    void setLayer(physx::PxU32 layer);           // Word0: Set what type of object this is
    void setLayerMask(physx::PxU32 layerMask);   // Word1: Set what this object can collide with
    physx::PxU32 getLayer() const { return currentLayer; }      // Word0: Get what type of object this is
    physx::PxU32 getLayerMask() const { return currentLayerMask; } // Word1: Get what this object can collide with
    
    // Force update collision filters
    void forceUpdateCollisionFilters();
    
    // Helper function to get shape type as string
    std::string getShapeTypeString() const;
    
    // Debug method to print collision filter configuration
    void debugCollisionFilters();
    
    // Example method to set up a trigger
    void setupAsTrigger(physx::PxU32 triggerLayer = LAYER_TRIGGER, physx::PxU32 collisionMask = LAYER_PLAYER | LAYER_ENEMY);
    
    // Utility
    bool isInitialized() const { return initialized; }
    GameObject* getOwner() const { return owner; }
    
    // Helper to attach to rigidbody
    void attachToRigidbody(Rigidbody* rigidbody);
    void detachFromRigidbody();
    
    // Public method to force transform update (call when GameObject position changes)
    void forceTransformUpdate();
    
    // Mark that transform needs update on next opportunity
    void markTransformDirty();
};
