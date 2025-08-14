#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "GameObject.h"
#include "../core/CoreExporter.h"
#include "../core/PhysicsManager.h"
#include "PhysicalObject.h"

class MANTRAXCORE_API GameObject;

class MANTRAXCORE_API Rigidbody : public Component {
private:
    // PhysX objects
    physx::PxRigidActor* rigidActor;
    physx::PxRigidDynamic* dynamicActor;
    physx::PxRigidStatic* staticActor;
    
    // Physics properties
    BodyType bodyType;
    float mass;
    float damping;
    float gravityFactor;
    
    bool initialized;

public:
    Rigidbody(GameObject* obj);
    ~Rigidbody();

    std::string getComponentName() const override {
        return "Rigidbody";
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
    
    // Transform synchronization
    void syncTransformToPhysX();
    void syncTransformFromPhysX();
    
    // Getters
    physx::PxRigidActor* getRigidActor() const { return rigidActor; }
    physx::PxRigidDynamic* getRigidDynamic() const { return dynamicActor; }
    physx::PxRigidStatic* getRigidStatic() const { return staticActor; }
    
    // Physics properties
    void setMass(float newMass);
    float getMass() const { return mass; }
    
    void setVelocity(const glm::vec3& velocity);
    glm::vec3 getVelocity() const;
    
    void setDamping(float newDamping);
    float getDamping() const { return damping; }
    
    void setGravityFactor(float factor);
    float getGravityFactor() const { return gravityFactor; }
    
    // Body type
    void setBodyType(BodyType type);
    BodyType getBodyType() const { return bodyType; }
    
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
    
    // Enable/Disable
    void enable();
    void disable();
    bool isActive() const;
};
