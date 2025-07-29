#include "PhysicalObject.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include "../core/PhysicsEventHandler.h"
#include "../components/SceneManager.h"
#include <iostream>
#include <functional>
#include <algorithm>

// Helper function to get shape type as string
std::string PhysicalObject::getShapeTypeString() const {
    switch (shapeType) {
        case ShapeType::Box: return "Box";
        case ShapeType::Sphere: return "Sphere";
        case ShapeType::Capsule: return "Capsule";
        case ShapeType::Plane: return "Plane";
        default: return "Unknown";
    }
}

// Helper function to safely set collision filters, recreating shape if necessary
void PhysicalObject::safeSetCollisionFilters(CollisionGroup group, CollisionMask mask) {
    if (!shape) return;
    
    // Check if the shape is shared (attached to multiple actors)
    bool needsRecreation = false;
    
    // Try to set the filter data for custom filter shader
    try {
        // Configure shape flags for custom filter shader
        if (isTriggerShape) {
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        } else {
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        }
        
        // Set filter data for custom filter shader
        physx::PxFilterData filterData;
        filterData.word0 = static_cast<physx::PxU32>(group);  // Collision group
        filterData.word1 = static_cast<physx::PxU32>(mask);   // Collision mask
        filterData.word2 = isTriggerShape ? 0x1 : 0x0;        // Trigger flag
        filterData.word3 = 0;
        
        shape->setSimulationFilterData(filterData);
        shape->setQueryFilterData(filterData);
        
    } catch (const std::exception& e) {
        // If setting filter data fails, we need to recreate the shape
        needsRecreation = true;
    }
    
    if (needsRecreation) {
        // Store current shape properties
        ShapeType currentShapeType = shapeType;
        glm::vec3 currentBoxExtents = boxHalfExtents;
        float currentSphereRadius = sphereRadius;
        float currentCapsuleRadius = capsuleRadius;
        float currentCapsuleHalfHeight = capsuleHalfHeight;
        bool currentIsTrigger = isTriggerShape;
        
        // Detach shape from actor first
        if (rigidActor && shape) {
            rigidActor->detachShape(*shape);
        }
        
        // Destroy current shape and recreate it
        if (shape) {
            shape->release();
            shape = nullptr;
        }
        
        // Recreate the shape with the same properties
        createShape();
        
        // Reattach to actor
        if (rigidActor && shape) {
            rigidActor->attachShape(*shape);
            
            // Configure shape flags correctly for custom filter shader
            if (isTriggerShape) {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            } else {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            }
            
        }
    }
}

// Helper function to recreate shape safely when shared shape issues occur
void PhysicalObject::recreateShapeSafely() {
    // Store current properties
    ShapeType currentShapeType = shapeType;
    glm::vec3 currentBoxExtents = boxHalfExtents;
    float currentSphereRadius = sphereRadius;
    float currentCapsuleRadius = capsuleRadius;
    float currentCapsuleHalfHeight = capsuleHalfHeight;
    bool currentIsTrigger = isTriggerShape;
    
    // Detach shape from actor first
    if (rigidActor && shape) {
        rigidActor->detachShape(*shape);
    }
    
    // Destroy current shape
    if (shape) {
        shape->release();
        shape = nullptr;
    }
    
    // Recreate the shape with the same properties
    createShape();
    
    // Reattach to actor
    if (rigidActor && shape) {
        rigidActor->attachShape(*shape);
        
        // Configure shape flags correctly
        if (isTriggerShape) {
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        } else {
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        }
        
    }
}

PhysicalObject::PhysicalObject(GameObject* obj) : Component() {
    setOwner(obj);
    rigidActor = nullptr;
    dynamicActor = nullptr;
    staticActor = nullptr;
    shape = nullptr;
    material = nullptr;
    colliderReference = nullptr;
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
    isTriggerShape = false;
    
    // Initialize layer configuration
    currentLayer = LAYER_0;
    currentLayerMask = LAYER_0 | LAYER_1 | LAYER_2 | LAYER_3 | LAYER_4 | LAYER_5 | LAYER_6 | LAYER_7 | LAYER_8 | LAYER_9 | LAYER_10 | LAYER_11 | LAYER_12 | LAYER_13 | LAYER_14 | LAYER_15 | LAYER_16 | LAYER_17 | LAYER_18 | LAYER_19 | LAYER_TRIGGER | LAYER_PLAYER | LAYER_ENEMY | LAYER_ENVIRONMENT;
    
    // Don't auto-start - wait for explicit initialization
    // start() will be called later when PhysicsManager is ready
    
    // Set up default contact callback
    setContactCallback([](PhysicalObject* obj1, PhysicalObject* obj2, const glm::vec3& contactPoint, const glm::vec3& contactNormal, float contactForce) {
        if (obj1 && obj2) {
            auto* owner1 = obj1->getOwner();
            auto* owner2 = obj2->getOwner();
        }
    });
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
    
    // Initialize layer configuration from GameObject
    if (owner) {
        currentLayer = owner->getLayer();
        currentLayerMask = owner->getLayerMask();
    }
    
    createBody();
    createShape();
    
    if (rigidActor && shape) {
        rigidActor->attachShape(*shape);
        physicsManager.addActor(*rigidActor);
        
        // Register this PhysicalObject with the PhysicsEventHandler
        PhysicsEventHandler* eventHandler = physicsManager.getEventHandler();
        if (eventHandler) {
            eventHandler->registerPhysicalObject(rigidActor, this);
        }
        
        initialized = true;
        
        // Debug collision filters
        debugCollisionFilters();
        
        // If this is a trigger, automatically set up the trigger callback
        if (isTriggerShape && !triggerCallback) {
            setTriggerCallback([](PhysicalObject* trigger, PhysicalObject* other) {
                if (trigger) {
                    if (other) {
                    }
                    else {
                    }
                }
            });
        }
    } else {
        std::cerr << "PhysicalObject::initializePhysics() - Failed to create body or shape!" << std::endl;
    }
}

void PhysicalObject::update() {
    auto& physicsManager = PhysicsManager::getInstance();
    if (rigidActor && isActive() && physicsManager.getPhysics()) {
        syncTransformFromPhysX();
        
        // Update collision filters only when necessary
        // Check if layer configuration has changed
        if (owner) {
            physx::PxU32 gameObjectLayer = owner->getLayer();
            physx::PxU32 gameObjectLayerMask = owner->getLayerMask();
            
            // Only update if the layer configuration has actually changed
            if (gameObjectLayer != currentLayer || gameObjectLayerMask != currentLayerMask) {
                currentLayer = gameObjectLayer;
                currentLayerMask = gameObjectLayerMask;
                
                // Update filters safely for custom filter shader
                if (shape) {
                    try {
                        // Configure shape flags for custom filter shader
                        if (isTriggerShape) {
                            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
                            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
                            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                        } else {
                            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
                            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                        }
                        
                        // Set filter data for custom filter shader
                        physx::PxFilterData filterData;
                        filterData.word0 = currentLayer;  // Collision group
                        filterData.word1 = currentLayerMask;  // Collision mask
                        filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
                        filterData.word3 = 0;
                        
                        shape->setSimulationFilterData(filterData);
                        shape->setQueryFilterData(filterData);
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Error updating filters, recreating shape: " << e.what() << std::endl;
                        recreateShapeSafely();
                    }
                }
            }
        }
    }
}

void PhysicalObject::destroy() {
    std::cout << "PhysicalObject: Starting cleanup..." << std::endl;
    
    auto& physicsManager = PhysicsManager::getInstance();
    
    // Unregister from event handler first
    if (rigidActor) {
        PhysicsEventHandler* eventHandler = physicsManager.getEventHandler();
        if (eventHandler) {
            eventHandler->unregisterTriggerCallback(rigidActor);
            eventHandler->unregisterContactCallback(rigidActor);
            eventHandler->unregisterPhysicalObject(rigidActor);
        }
    }
    
    // Remove from scene and release actor
    if (rigidActor && physicsManager.getPhysics()) {
        std::cout << "PhysicalObject: Removing actor from scene..." << std::endl;
        physicsManager.removeActor(*rigidActor);
        
        std::cout << "PhysicalObject: Releasing rigid actor..." << std::endl;
        rigidActor->release();
        rigidActor = nullptr;
        dynamicActor = nullptr;
        staticActor = nullptr;
    }
    
    // Release shape
    if (shape) {
        std::cout << "PhysicalObject: Releasing shape..." << std::endl;
        shape->release();
        shape = nullptr;
    }
    
    // Release material if we own it
    if (material) {
        // Note: We don't release material here as it might be shared
        // The PhysicsManager will handle material cleanup
        material = nullptr;
    }
    
    initialized = false;
    std::cout << "PhysicalObject: Cleanup completed." << std::endl;
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

    if (!physicsManager.getPhysics()) {
        std::cerr << "PhysicalObject::createShape() - PhysicsManager not initialized yet!" << std::endl;
        return;
    }

    // Create the shape with unique configuration
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

    if (!shape) {
        std::cerr << "Failed to create shape!" << std::endl;
        return;
    }

    // Configure filter data for custom filter shader
    physx::PxU32 gameObjectLayer = owner ? owner->getLayer() : LAYER_0;
    physx::PxU32 gameObjectLayerMask = owner ? owner->getLayerMask() :
        (LAYER_0 | LAYER_1 | LAYER_2 | LAYER_3 | LAYER_4 | LAYER_5 | LAYER_6 | LAYER_7 |
            LAYER_8 | LAYER_9 | LAYER_10 | LAYER_11 | LAYER_12 | LAYER_13 | LAYER_14 | LAYER_15 |
            LAYER_16 | LAYER_17 | LAYER_18 | LAYER_19 | LAYER_TRIGGER | LAYER_PLAYER |
            LAYER_ENEMY | LAYER_ENVIRONMENT);

    // Set up filter data for custom filter shader
    physx::PxFilterData filterData;
    filterData.word0 = gameObjectLayer;  // Collision group
    filterData.word1 = gameObjectLayerMask;  // Collision mask
    filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
    filterData.word3 = 0;

    // Configure shape flags BEFORE setting filter data
    if (isTriggerShape) {
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    } else {
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    }

    // Set filter data AFTER configuring flags
    shape->setSimulationFilterData(filterData);
    shape->setQueryFilterData(filterData);
    
    // Establecer la referencia del collider para el inspector
    setColliderReference(shape);
}

void PhysicalObject::verifyTriggerSetup() {
    if (!shape || !rigidActor) {
        return;
    }

    // Verificar flags del shape
    physx::PxShapeFlags flags = shape->getFlags();
    bool isTriggerFlag = flags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
    bool isSimulationFlag = flags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
    bool isSceneQueryFlag = flags.isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE);

    // Verificar filter data
    physx::PxFilterData filterData = shape->getSimulationFilterData();

    // Verificar si está correctamente configurado
    bool isCorrectlyConfigured = (isTriggerShape == isTriggerFlag) &&
        (isTriggerShape ? !isSimulationFlag : isSimulationFlag) &&
        ((filterData.word2 & 0x1) == (isTriggerShape ? 1 : 0));
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

void PhysicalObject::setTrigger(bool isTrigger) {
    std::cout << "=== SET TRIGGER DEBUG ===" << std::endl;
    std::cout << "Setting trigger: " << (isTrigger ? "TRUE" : "FALSE") << std::endl;
    std::cout << "Shape exists: " << (shape ? "YES" : "NO") << std::endl;
    std::cout << "RigidActor exists: " << (rigidActor ? "YES" : "NO") << std::endl;
    
    isTriggerShape = isTrigger;

    if (shape && rigidActor) {
        try {
            // Configure shape flags for custom filter shader
            if (isTriggerShape) {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                
                // Asegurarse de que el actor tenga la configuración correcta para triggers
                if (dynamicActor) {
                    // Para objetos dinámicos, asegurarse de que no colisionen pero sigan recibiendo eventos
                    dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
                }
                std::cout << "Trigger flags set successfully" << std::endl;
            } else {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
                
                // Restaurar la configuración normal para colisiones
                if (dynamicActor) {
                    dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
                }
                std::cout << "Normal collision flags set successfully" << std::endl;
            }

            // Update filter data for custom filter shader
            physx::PxFilterData filterData = shape->getSimulationFilterData();
            filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
            std::cout << "Filter data updated successfully" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error in setTrigger: " << e.what() << std::endl;
            std::cout << "Shape is shared, recreating..." << std::endl;
            // Si obtenemos un error, intentamos recrear el shape
            recreateShapeSafely();
            std::cout << "Shape recreated successfully" << std::endl;
        }
    } else {
        std::cout << "Cannot set trigger - shape or rigidActor is null" << std::endl;
    }
    
    std::cout << "==========================" << std::endl;
}


void PhysicalObject::setCollisionGroup(CollisionGroup group) {
    safeSetCollisionFilters(group, CollisionMask::DYNAMIC_MASK);
}

void PhysicalObject::setCollisionMask(CollisionMask mask) {
    safeSetCollisionFilters(CollisionGroup::DYNAMIC_GROUP, mask);
}

void PhysicalObject::setupCollisionFilters(CollisionGroup group, CollisionMask mask) {
    safeSetCollisionFilters(group, mask);
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

// Trigger and contact event methods
void PhysicalObject::setTriggerCallback(std::function<void(PhysicalObject*, PhysicalObject*)> callback) {
    triggerCallback = callback;
    
    // Register with PhysicsEventHandler if we have a rigid actor
    if (rigidActor) {
        auto& physicsManager = PhysicsManager::getInstance();
        PhysicsEventHandler* eventHandler = physicsManager.getEventHandler();
        if (eventHandler) {
            eventHandler->registerTriggerCallback(rigidActor, [this, eventHandler](const TriggerEvent& event) {
                if (triggerCallback) {
                    // Find the other PhysicalObject using the mapping
                    PhysicalObject* other = eventHandler->getPhysicalObject(event.otherActor);
                    triggerCallback(this, other);
                }
            });
        }
    }
}

void PhysicalObject::setContactCallback(std::function<void(PhysicalObject*, PhysicalObject*, const glm::vec3&, const glm::vec3&, float)> callback) {
    contactCallback = callback;
    
    // Register with PhysicsEventHandler if we have a rigid actor
    if (rigidActor) {
        auto& physicsManager = PhysicsManager::getInstance();
        PhysicsEventHandler* eventHandler = physicsManager.getEventHandler();
        if (eventHandler) {
            eventHandler->registerContactCallback(rigidActor, [this, eventHandler](const ContactEvent& event) {
                if (contactCallback) {
                    // Find the other PhysicalObject
                    PhysicalObject* other = nullptr;
                    
                    // Determine which actor is the "other" one
                    if (event.actor1 == rigidActor) {
                        other = eventHandler->getPhysicalObject(event.actor2);
                    } else if (event.actor2 == rigidActor) {
                        other = eventHandler->getPhysicalObject(event.actor1);
                    }
                    
                    contactCallback(this, other, event.contactPoint, event.contactNormal, event.contactForce);
                }
            });
        }
    }
}

// Static raycast methods
struct RaycastHit PhysicalObject::raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    return PhysicsManager::getInstance().raycast(origin, direction, maxDistance);
}

std::vector<struct RaycastHit> PhysicalObject::raycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    return PhysicsManager::getInstance().raycastAll(origin, direction, maxDistance);
}

void PhysicalObject::setLayer(physx::PxU32 layer) {
    currentLayer = layer;
    
    // Force update collision filters
    forceUpdateCollisionFilters();
}

void PhysicalObject::setLayerMask(physx::PxU32 layerMask) {
    currentLayerMask = layerMask;
    
    // Force update collision filters
    forceUpdateCollisionFilters();
}

void PhysicalObject::setupAsTrigger(physx::PxU32 triggerLayer, physx::PxU32 collisionMask) {
    // Set as trigger
    setTrigger(true);
    
    // Set layer configuration
    setLayer(triggerLayer);
    setLayerMask(collisionMask);
}

void PhysicalObject::setupAsSceneChangeTrigger(const std::string& targetSceneName, physx::PxU32 triggerLayer, physx::PxU32 collisionMask) {
    // Set as trigger
    setTrigger(true);
    
    // Set layer configuration
    setLayer(triggerLayer);
    setLayerMask(collisionMask);
    
    // Set up custom trigger callback for scene change
    setTriggerCallback([targetSceneName](PhysicalObject* trigger, PhysicalObject* other) {
        if (trigger && other) {
            auto* triggerOwner = trigger->getOwner();
            auto* otherOwner = other->getOwner();
            
            // Cambiar a la escena específica
            try {
                auto& sceneManager = SceneManager::getInstance();
                sceneManager.setActiveScene(targetSceneName);
            } catch (const std::exception& e) {
            }
        }
    });
}



void PhysicalObject::forceUpdateCollisionFilters() {
    if (shape) {
        try {
                // Configure shape flags for custom filter shader
    if (isTriggerShape) {
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    } else {
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
    }
    
    // Set filter data for custom filter shader
    physx::PxFilterData filterData;
    filterData.word0 = currentLayer;  // Collision group
    filterData.word1 = currentLayerMask;  // Collision mask
    filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
    filterData.word3 = 0;
    
    shape->setSimulationFilterData(filterData);
    shape->setQueryFilterData(filterData);
            
        } catch (const std::exception& e) {
            std::cerr << "Error updating collision filters: " << e.what() << std::endl;
            std::cerr << "Shape may be shared between multiple actors. Recreating shape..." << std::endl;
            
            // Use the safe recreation method
            recreateShapeSafely();
        }
    }
} 

// Force update collision filters with recreation if needed
void PhysicalObject::forceUpdateCollisionFiltersAggressive() {
    // Always recreate the shape to ensure fresh configuration
    recreateShapeSafely();
    
    // Force update the filters after recreation for custom filter shader
    if (shape) {
        try {
            // Configure shape flags
            if (isTriggerShape) {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            } else {
                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            }
            
            // Set filter data for custom filter shader
            physx::PxFilterData filterData;
            filterData.word0 = currentLayer;  // Collision group
            filterData.word1 = currentLayerMask;  // Collision mask
            filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
            filterData.word3 = 0;
            
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
            
            debugCollisionFilters();
        } catch (const std::exception& e) {
            std::cerr << "Error in aggressive filter update: " << e.what() << std::endl;
        }
    }
}

// Debug method to print current collision filter configuration
void PhysicalObject::debugCollisionFilters() {
    if (!shape) {
        return;
    }
    
    // Get current filter data
    physx::PxFilterData simFilter = shape->getSimulationFilterData();
    physx::PxFilterData queryFilter = shape->getQueryFilterData();
    
    // Check shape flags
    physx::PxShapeFlags flags = shape->getFlags();
    
    // Check GameObject layer configuration
    if (owner) {
    }
} 

// Collider reference management methods
void PhysicalObject::setColliderReference(physx::PxShape* collider) {
    colliderReference = collider;
    
    // If we have a reference, update our shape to match it
    if (colliderReference && shape) {
        updateColliderFromReference();
    }
}

void PhysicalObject::updateColliderFromReference() {
    if (!colliderReference || !shape) {
        return;
    }
    
    try {
        // Copy geometry from reference to our shape
        physx::PxGeometryHolder refGeometry = colliderReference->getGeometry();
        
        // Update our shape's geometry based on the reference
        switch (refGeometry.getType()) {
            case physx::PxGeometryType::eBOX: {
                physx::PxBoxGeometry boxGeom = refGeometry.box();
                shape->setGeometry(boxGeom);
                boxHalfExtents = glm::vec3(boxGeom.halfExtents.x, boxGeom.halfExtents.y, boxGeom.halfExtents.z);
                shapeType = ShapeType::Box;
                break;
            }
            case physx::PxGeometryType::eSPHERE: {
                physx::PxSphereGeometry sphereGeom = refGeometry.sphere();
                shape->setGeometry(sphereGeom);
                sphereRadius = sphereGeom.radius;
                shapeType = ShapeType::Sphere;
                break;
            }
            case physx::PxGeometryType::eCAPSULE: {
                physx::PxCapsuleGeometry capsuleGeom = refGeometry.capsule();
                shape->setGeometry(capsuleGeom);
                capsuleRadius = capsuleGeom.radius;
                capsuleHalfHeight = capsuleGeom.halfHeight;
                shapeType = ShapeType::Capsule;
                break;
            }
            case physx::PxGeometryType::ePLANE: {
                physx::PxPlaneGeometry planeGeom = refGeometry.plane();
                shape->setGeometry(planeGeom);
                shapeType = ShapeType::Plane;
                break;
            }
            default:
                break;
        }
        
        // Copy flags from reference
        physx::PxShapeFlags refFlags = colliderReference->getFlags();
        shape->setFlags(refFlags);
        
        // Copy filter data from reference
        physx::PxFilterData refFilterData = colliderReference->getSimulationFilterData();
        shape->setSimulationFilterData(refFilterData);
        shape->setQueryFilterData(refFilterData);
        
        // Update our internal state
        isTriggerShape = refFlags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
        currentLayer = refFilterData.word0;
        currentLayerMask = refFilterData.word1;
        
    } catch (const std::exception& e) {
        std::cerr << "Error updating collider from reference: " << e.what() << std::endl;
    }
}

void PhysicalObject::updateReferenceFromCollider() {
    if (!colliderReference || !shape) {
        return;
    }
    
    try {
        // Copy geometry from our shape to reference
        physx::PxGeometryHolder ourGeometry = shape->getGeometry();
        
        // Update reference's geometry based on our shape
        switch (ourGeometry.getType()) {
            case physx::PxGeometryType::eBOX: {
                physx::PxBoxGeometry boxGeom = ourGeometry.box();
                colliderReference->setGeometry(boxGeom);
                break;
            }
            case physx::PxGeometryType::eSPHERE: {
                physx::PxSphereGeometry sphereGeom = ourGeometry.sphere();
                colliderReference->setGeometry(sphereGeom);
                break;
            }
            case physx::PxGeometryType::eCAPSULE: {
                physx::PxCapsuleGeometry capsuleGeom = ourGeometry.capsule();
                colliderReference->setGeometry(capsuleGeom);
                break;
            }
            case physx::PxGeometryType::ePLANE: {
                physx::PxPlaneGeometry planeGeom = ourGeometry.plane();
                colliderReference->setGeometry(planeGeom);
                break;
            }
            default:
                break;
        }
        
        // Copy flags from our shape to reference
        physx::PxShapeFlags ourFlags = shape->getFlags();
        colliderReference->setFlags(ourFlags);
        
        // Copy filter data from our shape to reference
        physx::PxFilterData ourFilterData = shape->getSimulationFilterData();
        colliderReference->setSimulationFilterData(ourFilterData);
        colliderReference->setQueryFilterData(ourFilterData);
        
    } catch (const std::exception& e) {
        std::cerr << "Error updating reference from collider: " << e.what() << std::endl;
    }
}