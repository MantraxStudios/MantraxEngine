#include "PhysicalObject.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include "../core/PhysicsEventHandler.h"
#include "../components/SceneManager.h"
#include <iostream>
#include <functional>
#include <algorithm>

using json = nlohmann::json;


void PhysicalObject::defines() {
//    set_var("Body Type", &bodyType);                      // int* o BodyType*
//    set_var("Shape Type", &shapeType);                    // int* o ShapeType*
//    set_var("Mass", &mass);                               // float*
//    set_var("Friction", &friction);                       // float*
//    set_var("Restitution", &restitution);                 // float*
//    set_var("Damping", &damping);                         // float*
//    set_var("Gravity Factor", &gravityFactor);            // float*
//    set_var("Is Trigger", &isTriggerShape);               // bool*
//    set_var("Initialized", &initialized);                 // bool*
//
//    set_var("Box Half Extents", &boxHalfExtents);         // glm::vec3*
//    set_var("Sphere Radius", &sphereRadius);              // float*
//    set_var("Capsule Radius", &capsuleRadius);            // float*
//    set_var("Capsule Half Height", &capsuleHalfHeight);   // float*
//
//    set_var("Current Layer", &currentLayer);              // int*
//    set_var("Current Layer Mask", &currentLayerMask);     // int*
//
//    set_var("CSharp Bridge Enabled", &csharpBridgeEnabled); // bool*
//    set_var("CSharp Object Name", &csharpObjectName);       // std::string*
//
//    set_var("Velocity", &velocity);                       // glm::vec3* (debes exponer esta variable temporalmente)
//    set_var("Angular Velocity", &angularVelocity);        // glm::vec3* (idem arriba)
//    set_var("Is Awake", &isAwake);                        // bool*
//    set_var("Gravity Disabled", &gravityDisabled);        // bool*
//    set_var("Simulation Disabled", &simulationDisabled);  // bool*
//    set_var("Is Kinematic", &isKinematic);                // bool*
//
//    // Shape flags (pueden ser variables bool temporales para edición)
//    set_var("Shape Is Trigger", &shapeIsTrigger);           // bool*
//    set_var("Shape Is Simulation", &shapeIsSimulation);     // bool*
//    set_var("Shape Is Scene Query", &shapeIsSceneQuery);    // bool*
//    set_var("Shape Is Visualization", &shapeIsVisualization);// bool*
//
//    // Filter data (puedes editarlos como enteros sueltos)
//    set_var("Filter Word 0", &filterWord0);               // uint32_t*
//    set_var("Filter Word 1", &filterWord1);               // uint32_t*
//    set_var("Filter Word 2", &filterWord2);               // uint32_t*
//    set_var("Filter Word 3", &filterWord3);               // uint32_t*
//
//    set_var("Has Trigger Callback", &hasTriggerCallback); // bool*
//    set_var("Has Contact Callback", &hasContactCallback); // bool*
//
}

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
    
    std::cout << "[PhysicalObject] Creating body with type: " << static_cast<int>(bodyType) << std::endl;
    
    if (bodyType == BodyType::Dynamic) {
        std::cout << "[PhysicalObject] Creating Dynamic body" << std::endl;
        dynamicActor = physicsManager.createDynamicBody(transform, mass);
        rigidActor = dynamicActor;
        std::cout << "[PhysicalObject] Dynamic actor created: " << (dynamicActor ? "SUCCESS" : "FAILED") << std::endl;
    } else {
        std::cout << "[PhysicalObject] Creating Static body" << std::endl;
        staticActor = physicsManager.createStaticBody(transform);
        rigidActor = staticActor;
        std::cout << "[PhysicalObject] Static actor created: " << (staticActor ? "SUCCESS" : "FAILED") << std::endl;
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
            configureTriggerFlags();
            std::cout << "Trigger configuration completed successfully" << std::endl;
            
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

// Nueva función helper para configurar triggers correctamente
void PhysicalObject::configureTriggerFlags() {
    if (!shape) return;
    
    if (isTriggerShape) {
        // CONFIGURACIÓN CORRECTA PARA TRIGGERS:
        // 1. El shape debe ser trigger y NO participar en simulación de colisiones
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        
        // 2. CRÍTICO: Solo para actores DINÁMICOS se configura eDISABLE_SIMULATION
        //    Los actores ESTÁTICOS NO necesitan esta configuración
        if (dynamicActor) {
            // Para triggers dinámicos: El actor debe seguir participando en la simulación
            // para poder detectar eventos de trigger. NO usar eDISABLE_SIMULATION!
            dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
            std::cout << "[PhysicalObject] Dynamic trigger actor: simulation ENABLED for trigger detection" << std::endl;
        } else if (staticActor) {
            // Para actores estáticos: NO necesitan configuración adicional
            // Los triggers estáticos funcionan automáticamente con solo los flags del shape
            std::cout << "[PhysicalObject] Static trigger actor: no additional configuration needed" << std::endl;
        }
        
        std::cout << "[PhysicalObject] Trigger flags configured: objects can now pass through" << std::endl;
    } else {
        // Configuración normal para colisiones sólidas
        shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
        shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
        shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        
        // Solo configurar actores dinámicos (los estáticos no necesitan esta flag)
        if (dynamicActor) {
            dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
        }
        
        std::cout << "[PhysicalObject] Normal collision flags configured: solid collisions enabled" << std::endl;
    }

    // Update filter data for custom filter shader
    physx::PxFilterData filterData = shape->getSimulationFilterData();
    filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
    shape->setSimulationFilterData(filterData);
    shape->setQueryFilterData(filterData);
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
        std::cout << "[PhysicalObject] Adding force: (" << force.x << ", " << force.y << ", " << force.z << ")" << std::endl;
        std::cout << "[PhysicalObject] Force mode: " << (mode == physx::PxForceMode::eFORCE ? "FORCE" : "IMPULSE") << std::endl;
        std::cout << "[PhysicalObject] Actor is sleeping: " << (dynamicActor->isSleeping() ? "YES" : "NO") << std::endl;
        
        // Get current velocity before applying force
        physx::PxVec3 currentVel = dynamicActor->getLinearVelocity();
        std::cout << "[PhysicalObject] Current velocity: (" << currentVel.x << ", " << currentVel.y << ", " << currentVel.z << ")" << std::endl;
        
        dynamicActor->addForce(physx::PxVec3(force.x, force.y, force.z), mode);
        
        // Get velocity after applying force
        physx::PxVec3 newVel = dynamicActor->getLinearVelocity();
        std::cout << "[PhysicalObject] New velocity: (" << newVel.x << ", " << newVel.y << ", " << newVel.z << ")" << std::endl;
        
    } else {
        std::cout << "[PhysicalObject] ERROR: dynamicActor is null! Cannot add force." << std::endl;
        std::cout << "[PhysicalObject] Body type: " << static_cast<int>(bodyType) << std::endl;
        std::cout << "[PhysicalObject] Initialized: " << (initialized ? "true" : "false") << std::endl;
    }
}

void PhysicalObject::addTorque(const glm::vec3& torque, physx::PxForceMode::Enum mode) {
    if (dynamicActor) {
        dynamicActor->addTorque(physx::PxVec3(torque.x, torque.y, torque.z), mode);
    }
}

void PhysicalObject::addImpulse(const glm::vec3& impulse, physx::PxForceMode::Enum mode) {
    if (dynamicActor) {
        std::cout << "[PhysicalObject] Adding impulse: (" << impulse.x << ", " << impulse.y << ", " << impulse.z << ")" << std::endl;
        std::cout << "[PhysicalObject] Impulse mode: " << (mode == physx::PxForceMode::eIMPULSE ? "IMPULSE" : "FORCE") << std::endl;
        std::cout << "[PhysicalObject] Actor is sleeping: " << (dynamicActor->isSleeping() ? "YES" : "NO") << std::endl;
        
        // Get current velocity before applying impulse
        physx::PxVec3 currentVel = dynamicActor->getLinearVelocity();
        std::cout << "[PhysicalObject] Current velocity: (" << currentVel.x << ", " << currentVel.y << ", " << currentVel.z << ")" << std::endl;
        
        dynamicActor->addForce(physx::PxVec3(impulse.x, impulse.y, impulse.z), mode);
        
        // Get velocity after applying impulse
        physx::PxVec3 newVel = dynamicActor->getLinearVelocity();
        std::cout << "[PhysicalObject] New velocity: (" << newVel.x << ", " << newVel.y << ", " << newVel.z << ")" << std::endl;
        
    } else {
        std::cout << "[PhysicalObject] ERROR: dynamicActor is null! Cannot add impulse." << std::endl;
        std::cout << "[PhysicalObject] Body type: " << static_cast<int>(bodyType) << std::endl;
        std::cout << "[PhysicalObject] Initialized: " << (initialized ? "true" : "false") << std::endl;
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
            // Usar la nueva función helper para configuración consistente
            configureTriggerFlags();
            
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

std::string PhysicalObject::serializeComponent() const {
    nlohmann::json j;

    // Propiedades básicas del cuerpo
    j["bodyType"] = static_cast<int>(bodyType);
    j["shapeType"] = static_cast<int>(shapeType);
    j["mass"] = mass;
    j["friction"] = friction;
    j["restitution"] = restitution;
    j["damping"] = damping;
    j["gravityFactor"] = gravityFactor;
    j["isTrigger"] = isTriggerShape;
    j["initialized"] = initialized;

    // Safe glm::vec3 to JSON
    auto safeVec3 = [](const glm::vec3& v) -> std::vector<float> {
        auto isSafe = [](float f) { return std::isfinite(f) ? f : 0.0f; };
        return { isSafe(v.x), isSafe(v.y), isSafe(v.z) };
        };

    // Propiedades de formas geométricas
    j["boxHalfExtents"] = safeVec3(boxHalfExtents);
    j["sphereRadius"] = std::isfinite(sphereRadius) ? sphereRadius : 0.5f;
    j["capsuleRadius"] = std::isfinite(capsuleRadius) ? capsuleRadius : 0.5f;
    j["capsuleHalfHeight"] = std::isfinite(capsuleHalfHeight) ? capsuleHalfHeight : 0.5f;

    // Configuración de capas y colisiones
    j["currentLayer"] = currentLayer;
    j["currentLayerMask"] = currentLayerMask;

    // Propiedades C# Bridge
    j["csharpBridgeEnabled"] = csharpBridgeEnabled;
    j["csharpObjectName"] = csharpObjectName;

    // Velocidad actual (si el objeto está inicializado)
    if (initialized && dynamicActor) {
        glm::vec3 velocity = getVelocity();
        j["velocity"] = safeVec3(velocity);

        // Velocidad angular
        physx::PxVec3 angVel = dynamicActor->getAngularVelocity();
        j["angularVelocity"] = safeVec3(glm::vec3(angVel.x, angVel.y, angVel.z));

        // Estado de activación
        j["isAwake"] = !dynamicActor->isSleeping();

        // Flags del actor
        j["gravityDisabled"] = dynamicActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
        j["simulationDisabled"] = dynamicActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION);

        // Configuración de kinematic
        j["isKinematic"] = dynamicActor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
    }
    else {
        // Valores por defecto si no está inicializado
        j["velocity"] = safeVec3(glm::vec3(0.0f));
        j["angularVelocity"] = safeVec3(glm::vec3(0.0f));
        j["isAwake"] = true;
        j["gravityDisabled"] = (gravityFactor == 0.0f);
        j["simulationDisabled"] = false;
        j["isKinematic"] = false;
    }

    // Información de la forma (si existe)
    if (shape) {
        physx::PxShapeFlags flags = shape->getFlags();
        j["shapeFlags"] = {
            {"isTrigger", flags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE)},
            {"isSimulation", flags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE)},
            {"isSceneQuery", flags.isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE)},
            {"isVisualization", flags.isSet(physx::PxShapeFlag::eVISUALIZATION)}
        };

        // Filter data
        physx::PxFilterData filterData = shape->getSimulationFilterData();
        j["filterData"] = {
            {"word0", filterData.word0},
            {"word1", filterData.word1},
            {"word2", filterData.word2},
            {"word3", filterData.word3}
        };
    }

    // Estado de callbacks (si están configurados)
    j["hasTriggerCallback"] = (triggerCallback != nullptr);
    j["hasContactCallback"] = (contactCallback != nullptr);

    try {
        return j.dump();
    }
    catch (const std::exception& e) {
        std::cerr << "PhysicalObject::serializeComponent error: " << e.what() << std::endl;
        return "{}";
    }
}

void PhysicalObject::deserialize(const std::string& data) {
    try {
        json j = json::parse(data);

        // 2. Restaurar propiedades básicas (como variables locales, igual que en ImGui)
        BodyType deserializedBodyType = static_cast<BodyType>(j.value("bodyType", static_cast<int>(BodyType::Static)));
        ShapeType deserializedShapeType = static_cast<ShapeType>(j.value("shapeType", static_cast<int>(ShapeType::Box)));
        float deserializedMass = j.value("mass", 1.0f);
        float deserializedFriction = j.value("friction", 0.5f);
        float deserializedRestitution = j.value("restitution", 0.1f);
        float deserializedDamping = j.value("damping", 0.0f);
        float deserializedGravityFactor = j.value("gravityFactor", 1.0f);
        bool deserializedIsTrigger = j.value("isTrigger", false);

        // Propiedades de formas geométricas
        glm::vec3 deserializedBoxExtents = boxHalfExtents; // valor por defecto
        if (j.contains("boxHalfExtents") && j["boxHalfExtents"].is_array()) {
            auto e = j["boxHalfExtents"];
            if (e.size() >= 3) {
                deserializedBoxExtents = glm::vec3(e[0], e[1], e[2]);
            }
        }
        float deserializedSphereRadius = j.value("sphereRadius", 0.5f);
        float deserializedCapsuleRadius = j.value("capsuleRadius", 0.5f);
        float deserializedCapsuleHalfHeight = j.value("capsuleHalfHeight", 0.5f);

        // Configuración de capas
        physx::PxU32 deserializedCurrentLayer = j.value("currentLayer", static_cast<physx::PxU32>(0));
        physx::PxU32 deserializedCurrentLayerMask = j.value("currentLayerMask", static_cast<physx::PxU32>(0xFFFFFFFF));

        // Propiedades C# Bridge
        bool deserializedCsharpBridgeEnabled = j.value("csharpBridgeEnabled", false);
        std::string deserializedCsharpObjectName = j.value("csharpObjectName", "Unknown");

        // Velocidades para restaurar después
        glm::vec3 deserializedVelocity(0.0f);
        if (j.contains("velocity") && j["velocity"].is_array()) {
            auto vel = j["velocity"];
            if (vel.size() >= 3) {
                deserializedVelocity = glm::vec3(vel[0], vel[1], vel[2]);
            }
        }

        glm::vec3 deserializedAngularVelocity(0.0f);
        if (j.contains("angularVelocity") && j["angularVelocity"].is_array()) {
            auto angVel = j["angularVelocity"];
            if (angVel.size() >= 3) {
                deserializedAngularVelocity = glm::vec3(angVel[0], angVel[1], angVel[2]);
            }
        }

        // Estados del actor
        bool deserializedShouldBeAwake = j.value("isAwake", true);
        bool deserializedGravityDisabled = j.value("gravityDisabled", false);
        bool deserializedSimulationDisabled = j.value("simulationDisabled", false);
        bool deserializedIsKinematic = j.value("isKinematic", false);


        // 4. Si la inicialización fue exitosa, aplicar todas las propiedades
        // usando los mismos métodos que usa ImGui
        if (initialized) {

            // Aplicar Body Type (igual que en ImGui)
            if (bodyType != deserializedBodyType) {
                setBodyType(deserializedBodyType);
            }

            // Aplicar Shape Type (igual que en ImGui)
            if (shapeType != deserializedShapeType) {
                setShapeType(deserializedShapeType);
            }

            // Aplicar propiedades de formas específicas (igual que en ImGui)
            switch (deserializedShapeType) {
            case ShapeType::Box:
                if (boxHalfExtents != deserializedBoxExtents) {
                    setBoxHalfExtents(deserializedBoxExtents);
                }
                break;
            case ShapeType::Sphere:
                if (sphereRadius != deserializedSphereRadius) {
                    setSphereRadius(deserializedSphereRadius);
                }
                break;
            case ShapeType::Capsule:
                if (capsuleRadius != deserializedCapsuleRadius) {
                    setCapsuleRadius(deserializedCapsuleRadius);
                }
                if (capsuleHalfHeight != deserializedCapsuleHalfHeight) {
                    setCapsuleHalfHeight(deserializedCapsuleHalfHeight);
                }
                break;
            default:
                break;
            }

            // Aplicar propiedades físicas (igual que en ImGui)
            if (mass != deserializedMass) {
                setMass(deserializedMass);
            }
            if (friction != deserializedFriction) {
                setFriction(deserializedFriction);
            }
            if (restitution != deserializedRestitution) {
                setRestitution(deserializedRestitution);
            }
            if (damping != deserializedDamping) {
                setDamping(deserializedDamping);
            }
            if (gravityFactor != deserializedGravityFactor) {
                setGravityFactor(deserializedGravityFactor);
            }

            // Aplicar configuración de trigger (igual que en ImGui)
            if (isTriggerShape != deserializedIsTrigger) {
                setTrigger(deserializedIsTrigger);
            }

            // Aplicar configuración de capas (igual que en ImGui)
            if (currentLayer != deserializedCurrentLayer) {
                setLayer(deserializedCurrentLayer);
            }
            if (currentLayerMask != deserializedCurrentLayerMask) {
                setLayerMask(deserializedCurrentLayerMask);
            }

            // Aplicar velocidades (solo para objetos dinámicos)
            if (dynamicActor) {
                // Velocidad lineal
                if (deserializedVelocity != glm::vec3(0.0f)) {
                    setVelocity(deserializedVelocity);
                }

                // Velocidad angular
                if (deserializedAngularVelocity != glm::vec3(0.0f)) {
                    dynamicActor->setAngularVelocity(physx::PxVec3(
                        deserializedAngularVelocity.x,
                        deserializedAngularVelocity.y,
                        deserializedAngularVelocity.z
                    ));
                }

                // Flags del actor
                dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, deserializedGravityDisabled);
                dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, deserializedSimulationDisabled);

                // Configuración kinematic
                dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, deserializedIsKinematic);

                // Estado de activación (hacer esto al final)
                if (deserializedShouldBeAwake) {
                    wakeUp();
                }
                else {
                    dynamicActor->putToSleep();
                }
            }

            // Aplicar configuración C# Bridge
            if (deserializedCsharpBridgeEnabled) {
                enableCSharpBridge(true);
                setCSharpObjectName(deserializedCsharpObjectName);
            }

            // Restaurar filter data específico si existe
            if (j.contains("filterData") && shape) {
                auto filterJson = j["filterData"];
                physx::PxFilterData filterData;
                filterData.word0 = filterJson.value("word0", currentLayer);
                filterData.word1 = filterJson.value("word1", currentLayerMask);
                filterData.word2 = filterJson.value("word2", isTriggerShape ? 0x1 : 0x0);
                filterData.word3 = filterJson.value("word3", 0);

                shape->setSimulationFilterData(filterData);
                shape->setQueryFilterData(filterData);
            }

            // Configurar flags específicos de la forma si existen
            if (j.contains("shapeFlags") && shape) {
                auto shapeFlags = j["shapeFlags"];

                shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE,
                    shapeFlags.value("isTrigger", isTriggerShape));
                shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE,
                    shapeFlags.value("isSimulation", !isTriggerShape));
                shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE,
                    shapeFlags.value("isSceneQuery", true));
                shape->setFlag(physx::PxShapeFlag::eVISUALIZATION,
                    shapeFlags.value("isVisualization", true));
            }

            // Sincronizar transformación final
            syncTransformToPhysX();

            // Configurar callbacks por defecto si se indicó que existían
            bool hadTriggerCallback = j.value("hasTriggerCallback", false);
            bool hadContactCallback = j.value("hasContactCallback", false);

            if (hadTriggerCallback && isTriggerShape && !triggerCallback) {
                setTriggerCallback([](PhysicalObject* trigger, PhysicalObject* other) {
                    // Callback por defecto para triggers
                    });
            }

            if (hadContactCallback && !contactCallback) {
                setContactCallback([](PhysicalObject* obj1, PhysicalObject* obj2,
                    const glm::vec3& contactPoint, const glm::vec3& contactNormal, float contactForce) {
                        // Callback por defecto para contactos
                    });
            }

            // Debug final
            debugCollisionFilters();
        }

    }
    catch (const std::exception& e) {
        std::cerr << "PhysicalObject::deserialize error: " << e.what() << std::endl;

        // En caso de error, establecer valores por defecto seguros
        bodyType = BodyType::Static;
        shapeType = ShapeType::Box;
        mass = 1.0f;
        friction = 0.5f;
        restitution = 0.1f;
        damping = 0.0f;
        gravityFactor = 1.0f;
        boxHalfExtents = glm::vec3(0.5f);
        sphereRadius = 0.5f;
        capsuleRadius = 0.5f;
        capsuleHalfHeight = 0.5f;
        isTriggerShape = false;
        currentLayer = 0;
        currentLayerMask = 0xFFFFFFFF;
        csharpBridgeEnabled = false;
        csharpObjectName = "Unknown";

        // Intentar inicializar con valores por defecto
        if (!initialized) {
            try {
                initializePhysics();
            }
            catch (...) {
                std::cerr << "Failed to initialize physics with default values" << std::endl;
            }
        }
    }
}