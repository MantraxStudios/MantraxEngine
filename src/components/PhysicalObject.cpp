#include "PhysicalObject.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include <iostream>

PhysicalObject::PhysicalObject(GameObject* obj) : Component() {
    setOwner(obj);
    rigidActor = nullptr;
    dynamicActor = nullptr;
    staticActor = nullptr;
    shape = nullptr;
    material = nullptr;
    bodyType = BodyType::Dynamic;
    shapeType = ShapeType::Box;
    mass = 1.0f;
    friction = 0.5f;
    restitution = 0.1f;
    damping = 0.0f;
    gravityFactor = 1.0f;
    boxHalfExtents = glm::vec3(0.5f, 0.5f, 0.5f);
    sphereRadius = 0.5f;
    capsuleRadius = 0.5f;
    capsuleHalfHeight = 0.5f;
    initialized = false;
    
    // Don't auto-start - wait for explicit initialization
    // start() will be called later when PhysicsManager is ready
}

PhysicalObject::~PhysicalObject() {
    destroy();
}

void PhysicalObject::start() {
    // Don't auto-initialize physics - wait for explicit call
    // initializePhysics() should be called manually when ready
}

void PhysicalObject::initializePhysics() {
    if (initialized) return;
    
    // Check if PhysicsManager is initialized
    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.getPhysics()) {
        std::cerr << "PhysicalObject::initializePhysics() - PhysicsManager not initialized yet!" << std::endl;
        return;
    }
    
    createBody();
    createShape();
    
    if (rigidActor && shape) {
        rigidActor->attachShape(*shape);
        physicsManager.addActor(*rigidActor);
        initialized = true;
        std::cout << "PhysicalObject initialized successfully!" << std::endl;
    } else {
        std::cerr << "PhysicalObject::initializePhysics() - Failed to create body or shape!" << std::endl;
    }
}

void PhysicalObject::update() {
    auto& physicsManager = PhysicsManager::getInstance();
    if (rigidActor && isActive() && physicsManager.getPhysics()) {
        syncTransformFromPhysX();
    }
}

void PhysicalObject::destroy() {
    auto& physicsManager = PhysicsManager::getInstance();
    
    if (rigidActor && physicsManager.getPhysics()) {
        physicsManager.removeActor(*rigidActor);
        rigidActor->release();
        rigidActor = nullptr;
        dynamicActor = nullptr;
        staticActor = nullptr;
    }
    
    if (shape) {
        shape->release();
        shape = nullptr;
    }
    
    initialized = false;
}

void PhysicalObject::createBody() {
    auto& physicsManager = PhysicsManager::getInstance();
    
    // Check if PhysicsManager is initialized
    if (!physicsManager.getPhysics()) {
        std::cerr << "PhysicalObject::createBody() - PhysicsManager not initialized yet!" << std::endl;
        return;
    }
    
    // Get transform from GameObject
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();
    
    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
    );
    
    if (bodyType == BodyType::Dynamic) {
        dynamicActor = physicsManager.createDynamicBody(transform, mass);
        rigidActor = dynamicActor;
    } else {
        staticActor = physicsManager.createStaticBody(transform);
        rigidActor = staticActor;
    }
}

void PhysicalObject::createShape() {
    auto& physicsManager = PhysicsManager::getInstance();
    
    // Check if PhysicsManager is initialized
    if (!physicsManager.getPhysics()) {
        std::cerr << "PhysicalObject::createShape() - PhysicsManager not initialized yet!" << std::endl;
        return;
    }
    
    switch (shapeType) {
        case ShapeType::Box:
            shape = physicsManager.createBoxShape(
                physx::PxVec3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z),
                material
            );
            break;
        case ShapeType::Sphere:
            shape = physicsManager.createSphereShape(sphereRadius, material);
            break;
        case ShapeType::Capsule:
            shape = physicsManager.createCapsuleShape(capsuleRadius, capsuleHalfHeight, material);
            break;
        case ShapeType::Plane:
            shape = physicsManager.createPlaneShape(material);
            break;
    }
}

void PhysicalObject::syncTransformToPhysX() {
    if (!rigidActor) return;
    
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();
    
    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
    );
    
    rigidActor->setGlobalPose(transform);
}

void PhysicalObject::syncTransformFromPhysX() {
    if (!rigidActor) return;
    
    physx::PxTransform transform = rigidActor->getGlobalPose();
    
    glm::vec3 position(transform.p.x, transform.p.y, transform.p.z);
    glm::quat rotation(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
    
    owner->setWorldPosition(position);
    owner->setWorldRotationQuat(rotation);
}

void PhysicalObject::setMass(float newMass) {
    mass = newMass;
    if (dynamicActor) {
        physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, mass);
    }
}

void PhysicalObject::setVelocity(const glm::vec3& velocity) {
    if (dynamicActor) {
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
    }
}

glm::vec3 PhysicalObject::getVelocity() const {
    if (dynamicActor) {
        physx::PxVec3 velocity = dynamicActor->getLinearVelocity();
        return glm::vec3(velocity.x, velocity.y, velocity.z);
    }
    return glm::vec3(0.0f);
}

void PhysicalObject::setDamping(float newDamping) {
    damping = newDamping;
    if (dynamicActor) {
        dynamicActor->setLinearDamping(damping);
        dynamicActor->setAngularDamping(damping);
    }
}

void PhysicalObject::setFriction(float newFriction) {
    friction = newFriction;
    if (material) {
        material->setStaticFriction(friction);
        material->setDynamicFriction(friction);
    }
}

void PhysicalObject::setRestitution(float newRestitution) {
    restitution = newRestitution;
    if (material) {
        material->setRestitution(restitution);
    }
}

void PhysicalObject::setGravityFactor(float factor) {
    gravityFactor = factor;
    if (dynamicActor) {
        dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, factor == 0.0f);
    }
}

void PhysicalObject::setBodyType(BodyType type) {
    if (bodyType != type) {
        bodyType = type;
        if (initialized) {
            destroy();
            start();
        }
    }
}

void PhysicalObject::setShapeType(ShapeType type) {
    if (shapeType != type) {
        shapeType = type;
        if (initialized) {
            destroy();
            start();
        }
    }
}

void PhysicalObject::setBoxHalfExtents(const glm::vec3& extents) {
    boxHalfExtents = extents;
    if (initialized && shapeType == ShapeType::Box) {
        destroy();
        start();
    }
}

void PhysicalObject::setSphereRadius(float radius) {
    sphereRadius = radius;
    if (initialized && shapeType == ShapeType::Sphere) {
        destroy();
        start();
    }
}

void PhysicalObject::setCapsuleRadius(float radius) {
    capsuleRadius = radius;
    if (initialized && shapeType == ShapeType::Capsule) {
        destroy();
        start();
    }
}

void PhysicalObject::setCapsuleHalfHeight(float halfHeight) {
    capsuleHalfHeight = halfHeight;
    if (initialized && shapeType == ShapeType::Capsule) {
        destroy();
        start();
    }
}

void PhysicalObject::addForce(const glm::vec3& force, physx::PxForceMode::Enum mode) {
    if (dynamicActor) {
        dynamicActor->addForce(physx::PxVec3(force.x, force.y, force.z), mode);
    }
}

void PhysicalObject::addTorque(const glm::vec3& torque, physx::PxForceMode::Enum mode) {
    if (dynamicActor) {
        dynamicActor->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), mode);
    }
}

void PhysicalObject::addImpulse(const glm::vec3& impulse, physx::PxForceMode::Enum mode) {
    if (dynamicActor) {
        dynamicActor->addForce(physx::PxVec3(impulse.x, impulse.y, impulse.z), mode);
    }
}

void PhysicalObject::wakeUp() {
    if (dynamicActor) {
        dynamicActor->wakeUp();
    }
}

bool PhysicalObject::isAwake() const {
    if (dynamicActor) {
        return !dynamicActor->isSleeping();
    }
    return false;
} 