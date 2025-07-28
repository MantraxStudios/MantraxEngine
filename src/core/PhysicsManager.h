#pragma once

#include "PhysicsEvents.h"
#include "PhysicsEventHandler.h"
#include "PhysicsEventCallback.h"
#include <physx/PxPhysicsAPI.h>
#include "../core/CoreExporter.h"
#include <vector>
#include <functional>
#include <iostream>

// Layer constants (similar to the example)
#define LAYER_PLAYER 1
#define LAYER_ENEMY 2
#define LAYER_ENVIRONMENT 4
#define LAYER_TRIGGER 8
#define LAYER_PROJECTILE 16

// Forward declarations
class PhysicalObject;
class GameObject;

// Collision layers and filters
enum class CollisionLayer : physx::PxU32 {
    STATIC = 1,      // 0001
    DYNAMIC = 2,     // 0010
    PLAYER = 4,      // 0100
    ENEMY = 8,       // 1000
    PROJECTILE = 16, // 0001 0000
    TRIGGER = 32,    // 0010 0000
    SENSOR = 64      // 0100 0000
};

enum class CollisionGroup : physx::PxU32 {
    STATIC_GROUP = 1,      // Static objects
    DYNAMIC_GROUP = 2,     // Dynamic objects
    PLAYER_GROUP = 4,      // Player objects
    ENEMY_GROUP = 8,       // Enemy objects
    PROJECTILE_GROUP = 16, // Projectile objects
    TRIGGER_GROUP = 32,    // Trigger objects
    SENSOR_GROUP = 64      // Sensor objects
};

enum class CollisionMask : physx::PxU32 {
    STATIC_MASK = 1,       // STATIC (1)
    DYNAMIC_MASK = 2,      // DYNAMIC (2)
    PLAYER_MASK = 4,       // PLAYER (4)
    ENEMY_MASK = 8,        // ENEMY (8)
    PROJECTILE_MASK = 16,  // PROJECTILE (16)
    TRIGGER_MASK = 32,     // TRIGGER (32)
    SENSOR_MASK = 64,      // SENSOR (64)
    
    // Predefined masks for common scenarios
    ALL_MASK = 127,        // All groups (1+2+4+8+16+32+64)
    STATIC_DYNAMIC_MASK = 3, // STATIC | DYNAMIC (1+2)
    TRIGGER_COLLISION_MASK = 58,     // DYNAMIC | PLAYER | ENEMY | PROJECTILE (2+8+16+32)
    PLAYER_COLLISION_MASK = 95,      // STATIC | DYNAMIC | ENEMY | TRIGGER | SENSOR (1+2+16+4+64)
    ENEMY_COLLISION_MASK = 63,       // STATIC | DYNAMIC | PLAYER | PROJECTILE | TRIGGER (1+2+8+32+4)
    PROJECTILE_COLLISION_MASK = 54,  // STATIC | DYNAMIC | ENEMY | TRIGGER (1+2+16+4)
    SENSOR_COLLISION_MASK = 24       // PLAYER | ENEMY (8+16)
};

// Operadores de bits para CollisionMask
inline CollisionMask operator|(CollisionMask a, CollisionMask b) {
    return static_cast<CollisionMask>(static_cast<physx::PxU32>(a) | static_cast<physx::PxU32>(b));
}

inline CollisionMask operator&(CollisionMask a, CollisionMask b) {
    return static_cast<CollisionMask>(static_cast<physx::PxU32>(a) & static_cast<physx::PxU32>(b));
}

// Global filter shader function (must be outside the class for PhysX)
physx::PxFilterFlags customFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
                                        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
                                        physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

class MANTRAXCORE_API PhysicsManager {
private:
    static PhysicsManager* instance;
    
    // PhysX objects
    physx::PxFoundation* foundation;
    physx::PxPhysics* physics;
    physx::PxScene* scene;
    physx::PxMaterial* defaultMaterial;
    physx::PxDefaultCpuDispatcher* cpuDispatcher;
    
    // Allocator and error callback instances
    physx::PxDefaultAllocator mDefaultAllocatorCallback;
    physx::PxDefaultErrorCallback mDefaultErrorCallback;
    
    // Event handler (separate from physics simulation)
    PhysicsEventHandler* eventHandler;
    
    // Event callback (separate from physics simulation)
    PhysicsEventCallback* eventCallback;
    
public:
    PhysicsManager();
    ~PhysicsManager();

public:
    static PhysicsManager& getInstance();
    
    bool initialize();
    void update(float deltaTime);
    void cleanup();
    
    // Getters
    physx::PxScene* getScene() const { return scene; }
    physx::PxPhysics* getPhysics() const { return physics; }
    PhysicsEventHandler* getEventHandler() const { return eventHandler; }
    PhysicsEventCallback* getEventCallback() const { return eventCallback; }
    
    // Material creation
    physx::PxMaterial* createMaterial(float staticFriction, float dynamicFriction, float restitution);
    
    // Get default material
    physx::PxMaterial* getDefaultMaterial() const { return defaultMaterial; }
    
    // Body creation
    physx::PxRigidDynamic* createDynamicBody(const physx::PxTransform& transform, float mass = 1.0f);
    physx::PxRigidStatic* createStaticBody(const physx::PxTransform& transform);
    
    // Shape creation
    physx::PxShape* createBoxShape(const physx::PxVec3& halfExtents, physx::PxMaterial* material = nullptr);
    physx::PxShape* createSphereShape(float radius, physx::PxMaterial* material = nullptr);
    physx::PxShape* createCapsuleShape(float radius, float halfHeight, physx::PxMaterial* material = nullptr);
    physx::PxShape* createPlaneShape(physx::PxMaterial* material = nullptr);
    
    // Scene management
    void addActor(physx::PxActor& actor);
    void removeActor(physx::PxActor& actor);
    
    // Physics properties
    void setGravity(const physx::PxVec3& gravity);
    physx::PxVec3 getGravity() const;
    
    // Raycast system
    RaycastHit raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    std::vector<RaycastHit> raycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    
    // Collision filter helpers
    void verifyTriggerConfiguration();
    
    static void setupCollisionFilter(physx::PxShape* shape, CollisionGroup group, CollisionMask mask, bool isTrigger = false);
    
    // Public method for inspector access
    void setupShapeCollisionFilter(physx::PxShape* shape, CollisionGroup group, CollisionMask mask, bool isTrigger = false);
    
    // Debug methods
    void debugTriggerSetup(physx::PxActor* actor, bool isTrigger);
}; 