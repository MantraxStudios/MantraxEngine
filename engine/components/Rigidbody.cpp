#include "Rigidbody.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Rigidbody::Rigidbody(GameObject *obj) : Component()
{
    owner = obj;
    rigidActor = nullptr;
    dynamicActor = nullptr;
    staticActor = nullptr;

    bodyType = BodyType::Dynamic;
    mass = 1.0f;
    damping = 0.05f;
    gravityFactor = 1.0f;

    initialized = false;
}

Rigidbody::~Rigidbody()
{
    destroy();
}

void Rigidbody::defines()
{
    // Define variables for inspector
}

void Rigidbody::start()
{
    if (!initialized)
    {
        initializePhysics();
    }
}

void Rigidbody::update()
{
    if (initialized && rigidActor)
    {
        // Sync transform from PhysX to GameObject
        syncTransformFromPhysX();
    }
}

void Rigidbody::destroy()
{
    std::cout << "[Rigidbody] Starting cleanup for " << (owner ? owner->Name : "Unknown") << std::endl;

    if (rigidActor)
    {
        std::cout << "[Rigidbody] Removing actor from scene..." << std::endl;
        auto &physicsManager = PhysicsManager::getInstance();
        if (physicsManager.getScene())
        {
            physicsManager.getScene()->removeActor(*rigidActor);
        }

        std::cout << "[Rigidbody] Releasing rigid actor..." << std::endl;
        rigidActor->release();
        rigidActor = nullptr;
        dynamicActor = nullptr;
        staticActor = nullptr;
    }

    // Reset all state
    initialized = false;

    std::cout << "[Rigidbody] Cleanup completed for " << (owner ? owner->Name : "Unknown") << std::endl;
}

void Rigidbody::initializePhysics()
{
    if (initialized)
        return;

    createBody();

    if (rigidActor)
    {
        syncTransformToPhysX();
        initialized = true;
        std::cout << "[Rigidbody] Physics initialized for " << owner->Name << std::endl;
    }
}

void Rigidbody::createBody()
{
    auto &physicsManager = PhysicsManager::getInstance();
    physx::PxPhysics *physics = physicsManager.getPhysics();
    physx::PxScene *scene = physicsManager.getScene();

    if (!physics || !scene)
    {
        std::cerr << "[Rigidbody] PhysX not initialized" << std::endl;
        return;
    }

    // Get transform from GameObject
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();

    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));

    switch (bodyType)
    {
    case BodyType::Static:
    {
        staticActor = physics->createRigidStatic(transform);
        rigidActor = staticActor;
        break;
    }
    case BodyType::Dynamic:
    {
        dynamicActor = physics->createRigidDynamic(transform);
        physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, mass);
        dynamicActor->setLinearDamping(damping);
        dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
        rigidActor = dynamicActor;
        break;
    }
    case BodyType::Kinematic:
    {
        dynamicActor = physics->createRigidDynamic(transform);
        dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
        rigidActor = dynamicActor;
        break;
    }
    }

    if (rigidActor)
    {
        // Set user data to link back to this component
        rigidActor->userData = owner;

        // Add to scene
        scene->addActor(*rigidActor);

        std::cout << "[Rigidbody] Created "
                  << (bodyType == BodyType::Static ? "Static" : bodyType == BodyType::Dynamic ? "Dynamic"
                                                                                              : "Kinematic")
                  << " body for " << owner->Name << std::endl;
    }
}

void Rigidbody::syncTransformToPhysX()
{
    if (!rigidActor)
        return;

    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();

    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w));

    if (bodyType == BodyType::Kinematic && dynamicActor)
    {
        dynamicActor->setKinematicTarget(transform);
    }
    else
    {
        rigidActor->setGlobalPose(transform);
    }
}

void Rigidbody::syncTransformFromPhysX()
{
    if (!rigidActor || bodyType == BodyType::Static)
        return;

    physx::PxTransform transform = rigidActor->getGlobalPose();

    // Update GameObject transform
    owner->setWorldPosition(glm::vec3(transform.p.x, transform.p.y, transform.p.z));
    owner->setWorldRotationQuat(glm::quat(transform.q.w, transform.q.x, transform.q.y, transform.q.z));
}

// Physics properties
void Rigidbody::setMass(float newMass)
{
    mass = std::max(0.1f, newMass);
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, mass);
    }
}

void Rigidbody::setVelocity(const glm::vec3 &velocity)
{
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        dynamicActor->setLinearVelocity(physx::PxVec3(velocity.x, velocity.y, velocity.z));
    }
}

glm::vec3 Rigidbody::getVelocity() const
{
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        physx::PxVec3 vel = dynamicActor->getLinearVelocity();
        return glm::vec3(vel.x, vel.y, vel.z);
    }
    return glm::vec3(0.0f);
}

void Rigidbody::setDamping(float newDamping)
{
    damping = std::max(0.0f, newDamping);
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        dynamicActor->setLinearDamping(damping);
    }
}

void Rigidbody::setGravityFactor(float factor)
{
    gravityFactor = factor;
    if (dynamicActor)
    {
        dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
    }
}

void Rigidbody::setBodyType(BodyType type)
{
    if (bodyType == type)
        return;

    BodyType oldType = bodyType;
    bodyType = type;

    // If not initialized yet, just update the type
    if (!initialized)
        return;

    // Handle different conversion scenarios
    if (oldType == BodyType::Dynamic && type == BodyType::Kinematic)
    {
        // Dynamic to Kinematic: Just change the flag
        if (dynamicActor)
        {
            dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
            std::cout << "[Rigidbody] Changed Dynamic to Kinematic for " << owner->Name << std::endl;
        }
    }
    else if (oldType == BodyType::Kinematic && type == BodyType::Dynamic)
    {
        // Kinematic to Dynamic: Just change the flag
        if (dynamicActor)
        {
            dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
            // Reapply mass and damping settings
            physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, mass);
            dynamicActor->setLinearDamping(damping);
            dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, gravityFactor == 0.0f);
            std::cout << "[Rigidbody] Changed Kinematic to Dynamic for " << owner->Name << std::endl;
        }
    }
    else
    {
        // Static to Dynamic/Kinematic or Dynamic/Kinematic to Static: Need to recreate
        // This is because PhysX doesn't allow converting between static and dynamic at runtime
        std::cout << "[Rigidbody] Recreating body for type change from "
                  << (oldType == BodyType::Static ? "Static" : oldType == BodyType::Dynamic ? "Dynamic"
                                                                                            : "Kinematic")
                  << " to "
                  << (type == BodyType::Static ? "Static" : type == BodyType::Dynamic ? "Dynamic"
                                                                                      : "Kinematic")
                  << " for " << owner->Name << std::endl;

        destroy();
        initializePhysics();
    }
}

// Forces and impulses
void Rigidbody::addForce(const glm::vec3 &force, physx::PxForceMode::Enum mode)
{
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        dynamicActor->addForce(physx::PxVec3(force.x, force.y, force.z), mode);
    }
}

void Rigidbody::addTorque(const glm::vec3 &torque, physx::PxForceMode::Enum mode)
{
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        dynamicActor->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), mode);
    }
}

void Rigidbody::addImpulse(const glm::vec3 &impulse, physx::PxForceMode::Enum mode)
{
    if (dynamicActor && bodyType == BodyType::Dynamic)
    {
        dynamicActor->addForce(physx::PxVec3(impulse.x, impulse.y, impulse.z), mode);
    }
}

// State
void Rigidbody::wakeUp()
{
    if (dynamicActor)
    {
        dynamicActor->wakeUp();
    }
}

bool Rigidbody::isAwake() const
{
    if (dynamicActor)
    {
        return !dynamicActor->isSleeping();
    }
    return false;
}

void Rigidbody::enable()
{
    if (rigidActor)
    {
        rigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
    }
}

void Rigidbody::disable()
{
    if (rigidActor)
    {
        rigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
    }
}

bool Rigidbody::isActive() const
{
    if (rigidActor)
    {
        return !rigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION);
    }
    return false;
}

// Serialization
std::string Rigidbody::serializeComponent() const
{
    json j;
    j["bodyType"] = static_cast<int>(bodyType);
    j["mass"] = mass;
    j["damping"] = damping;
    j["gravityFactor"] = gravityFactor;
    return j.dump();
}

void Rigidbody::deserialize(const std::string &data)
{
    try
    {
        json j = json::parse(data);

        if (j.contains("bodyType"))
        {
            bodyType = static_cast<BodyType>(j["bodyType"]);
        }
        if (j.contains("mass"))
        {
            mass = j["mass"];
        }
        if (j.contains("damping"))
        {
            damping = j["damping"];
        }
        if (j.contains("gravityFactor"))
        {
            gravityFactor = j["gravityFactor"];
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Rigidbody] Deserialization error: " << e.what() << std::endl;
    }
}
