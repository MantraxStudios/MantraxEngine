#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxDefaultCpuDispatcher.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <vector>

class PhysicsManager {
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
    
    // Material creation
    physx::PxMaterial* createMaterial(float staticFriction, float dynamicFriction, float restitution);
    
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
}; 