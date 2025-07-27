#include "PhysicalObject.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include <glm/gtc/quaternion.hpp>

PhysicalObject::PhysicalObject(GameObject* owner) : Component() {
    setOwner(owner);
    mBody = nullptr;
    bodyType = BodyType::Dynamic;
    mass = 1.0f;
    linearDamping = 0.0f;
    angularDamping = 0.0f;
    friction = 0.5f;
    restitution = 0.0f;
    gravityFactor = 1.0f;
}

PhysicalObject::~PhysicalObject() {
    destroy();
}

void PhysicalObject::start() {
    createBody();
    syncTransformToMBody();
}

void PhysicalObject::destroy() {
    if (mBody) {
        // Remove from physics world
        auto& physicsWorld = PhysicsManager::getInstance();
        if (physicsWorld.getWorld()) {
            physicsWorld.getWorld()->RemoveBody(mBody);
        }
        mBody = nullptr;
    }
}

void PhysicalObject::update() {
    if (mBody && isEnabled) {
        syncTransformFromMBody();
    }
}

void PhysicalObject::createBody() {
    if (!mBody) {
        auto& physicsWorld = PhysicsManager::getInstance();
        mBody = physicsWorld.getWorld()->CreateBody();
        
        if (mBody) {
            // Set initial properties
            mBody->BodyType = (bodyType == BodyType::Static) ? TypeBody::bStatic : TypeBody::bDynamic;
            mBody->UseGravity = (gravityFactor > 0.0f);
            mBody->isSleeping = false;
        }
    }
}

void PhysicalObject::updateTransform() {
    if (mBody && owner) {
        syncTransformFromMBody();
    }
}

void PhysicalObject::syncTransformToMBody() {
    if (mBody && owner) {
        auto position = owner->getWorldPosition();
        auto rotation = owner->getWorldRotationQuat();
        
        // Convert glm to MantraxPhysics types
        mBody->Position = Vector3(position.x, position.y, position.z);
        mBody->Rotation = Quaternion(rotation.w, rotation.x, rotation.y, rotation.z);
    }
}

void PhysicalObject::syncTransformFromMBody() {
    if (mBody && owner) {
        // Convert MantraxPhysics to glm types
        glm::vec3 position(mBody->Position.x, mBody->Position.y, mBody->Position.z);
        glm::quat rotation(mBody->Rotation.w, mBody->Rotation.x, mBody->Rotation.y, mBody->Rotation.z);
        
        owner->setWorldPosition(position);
        owner->setWorldRotationQuat(rotation);
    }
}

void PhysicalObject::setBodyType(BodyType type) {
    bodyType = type;
    if (mBody) {
        mBody->BodyType = (type == BodyType::Static) ? TypeBody::bStatic : TypeBody::bDynamic;
    }
}

void PhysicalObject::setMass(float newMass) {
    mass = newMass;
    // Note: MantraxPhysics doesn't seem to have mass property, but we keep it for compatibility
}

void PhysicalObject::setLinearVelocity(const glm::vec3& velocity) {
    if (mBody) {
        mBody->Velocity = Vector3(velocity.x, velocity.y, velocity.z);
    }
}

void PhysicalObject::setAngularVelocity(const glm::vec3& velocity) {
    if (mBody) {
        mBody->AngularVelocity = Vector3(velocity.x, velocity.y, velocity.z);
    }
}

void PhysicalObject::setLinearDamping(float damping) {
    linearDamping = damping;
}

void PhysicalObject::setAngularDamping(float damping) {
    angularDamping = damping;
}

void PhysicalObject::setFriction(float newFriction) {
    friction = newFriction;
}

void PhysicalObject::setRestitution(float newRestitution) {
    restitution = newRestitution;
}

void PhysicalObject::setGravityFactor(float factor) {
    gravityFactor = factor;
    if (mBody) {
        mBody->UseGravity = (factor > 0.0f);
    }
}

glm::vec3 PhysicalObject::getLinearVelocity() const {
    if (mBody) {
        return glm::vec3(mBody->Velocity.x, mBody->Velocity.y, mBody->Velocity.z);
    }
    return glm::vec3(0.0f);
}

glm::vec3 PhysicalObject::getAngularVelocity() const {
    if (mBody) {
        return glm::vec3(mBody->AngularVelocity.x, mBody->AngularVelocity.y, mBody->AngularVelocity.z);
    }
    return glm::vec3(0.0f);
}

BodyType PhysicalObject::getBodyType() const {
    return bodyType;
}

float PhysicalObject::getMass() const {
    return mass;
}

float PhysicalObject::getLinearDamping() const {
    return linearDamping;
}

float PhysicalObject::getAngularDamping() const {
    return angularDamping;
}

float PhysicalObject::getFriction() const {
    return friction;
}

float PhysicalObject::getRestitution() const {
    return restitution;
}

float PhysicalObject::getGravityFactor() const {
    return gravityFactor;
}

void PhysicalObject::addForce(const glm::vec3& force) {
    if (mBody) {
        // Convert force to velocity change (F = ma, so a = F/m)
        Vector3 acceleration(force.x / mass, force.y / mass, force.z / mass);
        mBody->Velocity += acceleration;
    }
}

void PhysicalObject::addTorque(const glm::vec3& torque) {
    if (mBody) {
        // Convert torque to angular velocity change
        // Simplified: assuming uniform mass distribution
        Vector3 angularAcceleration(torque.x / mass, torque.y / mass, torque.z / mass);
        mBody->AngularVelocity += angularAcceleration;
    }
}

void PhysicalObject::addImpulse(const glm::vec3& impulse) {
    if (mBody) {
        // Impulse directly changes velocity (J = mΔv, so Δv = J/m)
        Vector3 velocityChange(impulse.x / mass, impulse.y / mass, impulse.z / mass);
        mBody->Velocity += velocityChange;
    }
}

void PhysicalObject::addAngularImpulse(const glm::vec3& impulse) {
    if (mBody) {
        // Angular impulse directly changes angular velocity
        Vector3 angularVelocityChange(impulse.x / mass, impulse.y / mass, impulse.z / mass);
        mBody->AngularVelocity += angularVelocityChange;
    }
}

void PhysicalObject::wakeUp() {
    if (mBody) {
        mBody->isSleeping = false;
    }
}

bool PhysicalObject::isAwake() const {
    if (mBody) {
        return !mBody->isSleeping;
    }
    return false;
}

void PhysicalObject::enable() {
    Component::enable();
    if (mBody) {
        mBody->isSleeping = false;
    }
}

void PhysicalObject::disable() {
    Component::disable();
    if (mBody) {
        mBody->isSleeping = true;
    }
} 