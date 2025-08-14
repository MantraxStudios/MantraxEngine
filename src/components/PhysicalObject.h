#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "GameObject.h"
#include "../core/CoreExporter.h"
#include "../core/PhysicsManager.h"
#include "../core/PhysicsEvents.h"
#include <functional>

enum class MANTRAXCORE_API BodyType {
    Static,
    Dynamic,
    Kinematic
};

enum class MANTRAXCORE_API ShapeType {
    Box,
    Sphere,
    Capsule,
    Plane
};

class MANTRAXCORE_API GameObject;

class MANTRAXCORE_API PhysicalObject : public Component {
private:
    // PhysX objects
    physx::PxRigidActor* rigidActor;
    physx::PxRigidDynamic* dynamicActor;
    physx::PxRigidStatic* staticActor;
    physx::PxShape* shape;
    physx::PxMaterial* material;
    
    // Collider reference for inspector modification
    physx::PxShape* colliderReference;
    
    // Physics properties
    BodyType bodyType;
    ShapeType shapeType;
    float mass;
    float friction;
    float restitution;
    float damping;
    float gravityFactor;
    
    // Shape properties
    glm::vec3 boxHalfExtents;
    float sphereRadius;
    float capsuleRadius;
    float capsuleHalfHeight;
    
    bool initialized;
    bool isTriggerShape;
    
    // Event callbacks
    std::function<void(PhysicalObject*, PhysicalObject*)> triggerCallback;
    std::function<void(PhysicalObject*, PhysicalObject*, const glm::vec3&, const glm::vec3&, float)> contactCallback;
    
    // Layer configuration (Word0/Word1)
    physx::PxU32 currentLayer;      // Word0: What type of object this is
    physx::PxU32 currentLayerMask;  // Word1: What this object can collide with

    // C# Bridge methods for physics events
    bool csharpBridgeEnabled = false;
    std::string csharpObjectName = "Unknown";

public:
    PhysicalObject(GameObject* obj);
    ~PhysicalObject();

    std::string getComponentName() const override {
        return "Physical Object";
    }
    
    // Component overrides
    void defines() override;
    void start() override;
    void update() override;
    void destroy() override;
    void deserialize(const std::string& data) override;
    std::string serializeComponent() const override;

    // Manual initialization
    void initializePhysics();
    
    // Body creation
    void createBody();
    void createShape();
    void verifyTriggerSetup();
    void configureShapeFlags();
    void configureTriggerFlags(); // Nueva función para configurar triggers correctamente
    
    // Transform synchronization
    void syncTransformToPhysX();
    void syncTransformFromPhysX();
    
    // Getters
    physx::PxRigidActor* getRigidActor() const { return rigidActor; }
    physx::PxRigidDynamic* getRigidDynamic() const { return dynamicActor; }
    physx::PxRigidStatic* getRigidStatic() const { return staticActor; }
    physx::PxShape* getShape() const { return shape; }
    physx::PxMaterial* getMaterial() const { return material; }
    
    // Physics properties
    void setMass(float newMass);
    float getMass() const { return mass; }
    
    void setVelocity(const glm::vec3& velocity);
    glm::vec3 getVelocity() const;
    
    void setDamping(float newDamping);
    float getDamping() const { return damping; }
    
    void setFriction(float newFriction);
    float getFriction() const { return friction; }
    
    void setRestitution(float newRestitution);
    float getRestitution() const { return restitution; }
    
    void setGravityFactor(float factor);
    float getGravityFactor() const { return gravityFactor; }
    
    // Body type
    void setBodyType(BodyType type);
    BodyType getBodyType() const { return bodyType; }
    
    // Shape properties
    void setShapeType(ShapeType type);
    ShapeType getShapeType() const { return shapeType; }
    
    // Trigger properties
    // IMPORTANTE: Para que los objetos puedan atravesar triggers:
    // - El shape debe tener eTRIGGER_SHAPE = true y eSIMULATION_SHAPE = false
    // - El actor NO debe tener eDISABLE_SIMULATION = true (debe estar habilitado para detectar eventos)
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
    
    // Forces and impulses
    void addForce(const glm::vec3& force, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE);
    void addTorque(const glm::vec3& torque, physx::PxForceMode::Enum mode = physx::PxForceMode::eFORCE);
    void addImpulse(const glm::vec3& impulse, physx::PxForceMode::Enum mode = physx::PxForceMode::eIMPULSE);
    
    // State
    void wakeUp();
    bool isAwake() const;
    
    // Utility
    bool isInitialized() const { return initialized; }
    GameObject* getOwner() const { return owner; }
    
    // Trigger and contact events
    void setTriggerCallback(std::function<void(PhysicalObject*, PhysicalObject*)> callback);
    void setContactCallback(std::function<void(PhysicalObject*, PhysicalObject*, const glm::vec3&, const glm::vec3&, float)> callback);
    
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
    
    // Force update collision filters with aggressive recreation
    void forceUpdateCollisionFiltersAggressive();
    
    // Helper function for safe collision filter setting
    void safeSetCollisionFilters(CollisionGroup group, CollisionMask mask);
    
    // Helper function to recreate shape safely when shared shape issues occur
    void recreateShapeSafely();
    
    // Helper function to get shape type as string
    std::string getShapeTypeString() const;
    
    // Debug method to print collision filter configuration
    void debugCollisionFilters();
    
    // Example method to set up a trigger (similar to the example)
    void setupAsTrigger(physx::PxU32 triggerLayer = LAYER_TRIGGER, physx::PxU32 collisionMask = LAYER_PLAYER | LAYER_ENEMY);
    
    // Method to set up a trigger that changes to a specific scene
    void setupAsSceneChangeTrigger(const std::string& targetSceneName, physx::PxU32 triggerLayer = LAYER_TRIGGER, physx::PxU32 collisionMask = LAYER_PLAYER | LAYER_ENEMY);
    
    // Raycast helper methods
    static struct RaycastHit raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    static std::vector<struct RaycastHit> raycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    
    // Collider reference management
    void setColliderReference(physx::PxShape* collider);
    physx::PxShape* getColliderReference() const { return colliderReference; }
    void updateColliderFromReference();
    void updateReferenceFromCollider();
    
    // C# Bridge methods for physics events
    void enableCSharpBridge(bool enable) { csharpBridgeEnabled = enable; }
    bool isCSharpBridgeEnabled() const { return csharpBridgeEnabled; }
    void setCSharpObjectName(const std::string& name) { csharpObjectName = name; }
    std::string getCSharpObjectName() const { return csharpObjectName; }
}; 