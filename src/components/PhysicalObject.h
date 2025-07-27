#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "../core/CoreExporter.h"

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

public:
    PhysicalObject(GameObject* obj);
    ~PhysicalObject();
    
    // Component overrides
    void start() override;
    void update() override;
    void destroy() override;
    
    // Manual initialization
    void initializePhysics();
    
    // Body creation
    void createBody();
    void createShape();
    
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
}; 