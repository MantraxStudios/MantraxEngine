#pragma once
#include <physx/PxPhysicsAPI.h>
#include <glm/glm.hpp>

// Forward declaration
class PhysicalObject;

// Trigger event structure
struct TriggerEvent {
    enum Type {
        ENTER,
        STAY,
        EXIT
    };
    
    Type type;
    physx::PxActor* triggerActor;
    physx::PxActor* otherActor;
};



// Contact event structure
struct ContactEvent {
    enum Type {
        BEGIN,
        END
    };
    
    Type type;
    physx::PxActor* actor1;
    physx::PxActor* actor2;
    glm::vec3 contactPoint;
    glm::vec3 contactNormal;
    float contactForce;
};

// Raycast hit structure
struct RaycastHit {
    bool hit = false;
    glm::vec3 position;
    glm::vec3 normal;
    float distance;
    physx::PxActor* actor = nullptr;
    physx::PxShape* shape = nullptr;
}; 