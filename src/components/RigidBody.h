#pragma once

#include "Component.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <glm/glm.hpp>
#include "../core/CoreExporter.h"

enum class BodyType {
    Static,
    Dynamic,
    Kinematic
};

class MANTRAXCORE_API RigidBody : public Component {
public:
    RigidBody(GameObject* owner);
    virtual ~RigidBody();

    void start() override;
    void update() override;
    void destroy() override;

    // Configuration methods
    void setBodyType(BodyType type);
    void setMass(float mass);
    void setLinearVelocity(const glm::vec3& velocity);
    void setAngularVelocity(const glm::vec3& velocity);
    void setLinearDamping(float damping);
    void setAngularDamping(float damping);
    void setFriction(float friction);
    void setRestitution(float restitution);
    void setGravityFactor(float factor);

    // Force application
    void addForce(const glm::vec3& force);
    void addTorque(const glm::vec3& torque);
    void addImpulse(const glm::vec3& impulse);
    void addAngularImpulse(const glm::vec3& impulse);

    // Getters
    BodyType getBodyType() const;
    float getMass() const;
    glm::vec3 getLinearVelocity() const;
    glm::vec3 getAngularVelocity() const;
    float getLinearDamping() const;
    float getAngularDamping() const;
    float getFriction() const;
    float getRestitution() const;
    float getGravityFactor() const;

    // Utility methods
    void wakeUp();
    bool isAwake() const;
    void enable() override;
    void disable() override;

private:
    void createBody();
    void updateTransform();

    JPH::Body* body;
    JPH::BodyID bodyId;
    BodyType bodyType;
    float mass;
    float linearDamping;
    float angularDamping;
    float friction;
    float restitution;
    float gravityFactor;
}; 