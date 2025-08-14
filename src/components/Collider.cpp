#include "Collider.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include "../core/PhysicsEventHandler.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Collider::Collider(GameObject* obj) : Component() {
    owner = obj;
    shape = nullptr;
    material = nullptr;
    staticActor = nullptr;
    transformNeedsUpdate = false; // Initialize transform dirty flag
    
    shapeType = ShapeType::Box;
    boxHalfExtents = glm::vec3(0.5f, 0.5f, 0.5f);
    sphereRadius = 0.5f;
    capsuleRadius = 0.5f;
    capsuleHalfHeight = 1.0f;
    
    friction = 0.5f;
    restitution = 0.0f;
    isTriggerShape = false;
    
    currentLayer = LAYER_DYNAMIC;
    currentLayerMask = LAYER_STATIC | LAYER_DYNAMIC;
    
    initialized = false;
}

Collider::~Collider() {
    destroy();
}

void Collider::defines() {
    // Define variables for inspector
}

void Collider::start() {
    if (!initialized) {
        initializeCollider();
    }
}

void Collider::update() {
    // Only update transform if marked as dirty and we have a static actor
    if (transformNeedsUpdate && staticActor) {
        updateActorTransform();
        transformNeedsUpdate = false; // Reset flag after update
    }
}

void Collider::destroy() {
    if (shape) {
        // Detach from rigidbody before releasing
        detachFromRigidbody();
        shape->release();
        shape = nullptr;
    }
    
    if (staticActor) {
        auto& physicsManager = PhysicsManager::getInstance();
        if (physicsManager.getScene()) {
            physicsManager.getScene()->removeActor(*staticActor);
        }
        staticActor->release();
        staticActor = nullptr;
    }
    
    if (material) {
        material->release();
        material = nullptr;
    }
    
    initialized = false;
}

void Collider::initializeCollider() {
    if (initialized) return;
    
    createShape();
    
    if (shape) {
        initialized = true;
        std::cout << "[Collider] Collider initialized for " << owner->Name << std::endl;
    }
}

void Collider::createShape() {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxPhysics* physics = physicsManager.getPhysics();
    
    if (!physics) {
        std::cerr << "[Collider] PhysX not initialized" << std::endl;
        return;
    }
    
    std::cout << "[Collider] Creating shape for " << owner->Name << " with trigger: " << (isTriggerShape ? "YES" : "NO") << std::endl;
    
    // Create material
    if (!material) {
        material = physics->createMaterial(friction, friction, restitution);
    }
    
    // Create geometry based on shape type
    physx::PxGeometry* geometry = nullptr;
    physx::PxBoxGeometry* boxGeom = nullptr;
    physx::PxSphereGeometry* sphereGeom = nullptr;
    physx::PxCapsuleGeometry* capsuleGeom = nullptr; 
    physx::PxPlaneGeometry* planeGeom = nullptr;
    
    switch (shapeType) {
        case ShapeType::Box:
            boxGeom = new physx::PxBoxGeometry(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z);
            geometry = boxGeom;
            break;
        case ShapeType::Sphere:
            sphereGeom = new physx::PxSphereGeometry(sphereRadius);
            geometry = sphereGeom;
            break;
        case ShapeType::Capsule:
            capsuleGeom = new physx::PxCapsuleGeometry(capsuleRadius, capsuleHalfHeight);
            geometry = capsuleGeom;
            break;
        case ShapeType::Plane:
            planeGeom = new physx::PxPlaneGeometry();
            geometry = planeGeom;
            break;
    }
    
    if (geometry) {
        // Create shape
        shape = physics->createShape(*geometry, *material);
        
        if (shape) {
            // Set user data to link back to this component
            shape->userData = this;
            
            // Configure shape flags FIRST (triggers need this before being attached)
            configureShapeFlags();
            
            // Set collision filters
            forceUpdateCollisionFilters();
            
            // Check if we need to create our own static actor (no Rigidbody attached)
            createIndependentActorIfNeeded();
            
            std::cout << "[Collider] Created " << getShapeTypeString() << " shape for " << owner->Name 
                      << " - Trigger: " << (isTriggerShape ? "YES" : "NO") << std::endl;
        }
        
        // Clean up geometry
        delete geometry;
    }
}

void Collider::configureShapeFlags() {
    if (!shape) return;
    
    try {
        std::cout << "[Collider] Configuring shape flags for " << owner->Name 
                  << " - Trigger: " << (isTriggerShape ? "YES" : "NO") << std::endl;
        
        if (isTriggerShape) {
            // Configurar flags para trigger
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            
            // Configuración especial para actores dinámicos con triggers
            auto rigidbody = owner->getComponent<Rigidbody>();
            if (rigidbody && rigidbody->getBodyType() == BodyType::Dynamic) {
                auto* dynamicActor = static_cast<physx::PxRigidDynamic*>(rigidbody->getRigidActor());
                if (dynamicActor) {
                    // CRÍTICO: Para triggers dinámicos, el actor debe participar en simulación
                    // pero con configuraciones especiales
                    dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
                    
                    // Configurar el actor como kinematic trigger si es necesario
                    // Esto permite detección de triggers sin respuesta física
                    dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
                    
                    // Asegurar que el mass sea correcto para triggers dinámicos
                    if (dynamicActor->getMass() <= 0.0f) {
                        dynamicActor->setMass(1.0f);
                        dynamicActor->setMassSpaceInertiaTensor(physx::PxVec3(1.0f));
                    }
                    
                    std::cout << "[Collider] Configured dynamic trigger actor for " << owner->Name << std::endl;
                }
            } else if (staticActor) {
                // Para actores estáticos, asegurar que están en la simulación
                staticActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
                std::cout << "[Collider] Configured static trigger actor for " << owner->Name << std::endl;
            }
            
        } else {
            // Configurar flags para colisión normal
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
            
            // Restaurar simulación normal para actores dinámicos
            auto rigidbody = owner->getComponent<Rigidbody>();
            if (rigidbody && rigidbody->getBodyType() == BodyType::Dynamic) {
                auto* dynamicActor = static_cast<physx::PxRigidDynamic*>(rigidbody->getRigidActor());
                if (dynamicActor) {
                    dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
                }
            }
        }
        
        // Siempre habilitar visualización para debug
        shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, true);
        
        // Verificar que los flags se aplicaron correctamente
        bool isCurrentlyTrigger = shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
        bool isCurrentlySimulation = shape->getFlags().isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
        
        std::cout << "[Collider] Flags applied - Trigger: " << (isCurrentlyTrigger ? "YES" : "NO")
                  << ", Simulation: " << (isCurrentlySimulation ? "YES" : "NO") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[Collider] Error configuring shape flags: " << e.what() << std::endl;
    }
}

void Collider::createIndependentActorIfNeeded() {
    // Check if there's already a Rigidbody component
    auto rigidbody = owner->getComponent<Rigidbody>();
    if (rigidbody && rigidbody->getRigidActor()) {
        // Rigidbody exists, attach to it
        rigidbody->getRigidActor()->attachShape(*shape);
        std::cout << "[Collider] Attached to existing Rigidbody on " << owner->Name << std::endl;
        return;
    }
    
    // No Rigidbody, create our own static actor
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxPhysics* physics = physicsManager.getPhysics();
    physx::PxScene* scene = physicsManager.getScene();
    
    if (!physics || !scene) {
        std::cerr << "[Collider] Cannot create independent actor - PhysX not initialized" << std::endl;
        return;
    }
    
    // Get transform from GameObject
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();
    
    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
    );
    
    // Create static actor
    staticActor = physics->createRigidStatic(transform);
    if (staticActor) {
        // Set user data to link back to GameObject
        staticActor->userData = owner;
        
        // Attach our shape
        staticActor->attachShape(*shape);
        
        // Add to scene
        scene->addActor(*staticActor);
        
        // Ensure initial position is correct
        syncTransformToPhysX();
        
        std::cout << "[Collider] Created independent static actor for " << owner->Name << std::endl;
    }
}

void Collider::verifyTriggerSetup() {
    if (!shape) {
        std::cout << "[Collider] ERROR: Cannot verify - shape is null!" << std::endl;
        return;
    }
    
    std::cout << "=== TRIGGER VERIFICATION ===" << std::endl;
    std::cout << "Object: " << owner->Name << std::endl;
    
    // Verificar flags de shape
    bool isCurrentlyTrigger = shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
    bool isSimulation = shape->getFlags().isSet(physx::PxShapeFlag::eSIMULATION_SHAPE);
    bool isSceneQuery = shape->getFlags().isSet(physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
    
    std::cout << "Shape Flags:" << std::endl;
    std::cout << "  Trigger: " << (isCurrentlyTrigger ? "✓" : "✗") << std::endl;
    std::cout << "  Simulation: " << (isSimulation ? "✓" : "✗") << std::endl;
    std::cout << "  SceneQuery: " << (isSceneQuery ? "✓" : "✗") << std::endl;
    
    // Verificar datos de filtro
    physx::PxFilterData filterData = shape->getSimulationFilterData();
    std::cout << "Filter Data:" << std::endl;
    std::cout << "  Layer (word0): " << filterData.word0 << std::endl;
    std::cout << "  Mask (word1): " << filterData.word1 << std::endl;
    std::cout << "  Trigger Flag (word2): " << filterData.word2 << std::endl;
    
    // Verificar actor
    physx::PxRigidActor* actor = shape->getActor();
    if (actor) {
        std::cout << "Actor Status:" << std::endl;
        std::cout << "  In Scene: " << (actor->getScene() ? "✓" : "✗") << std::endl;
        std::cout << "  Simulation Disabled: " << (actor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) ? "✓" : "✗") << std::endl;
        
        // Si es dinámico, verificar propiedades adicionales
        if (actor->getType() == physx::PxActorType::eRIGID_DYNAMIC) {
            auto* dynamicActor = static_cast<physx::PxRigidDynamic*>(actor);
            bool isKinematic = dynamicActor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
            std::cout << "  Kinematic: " << (isKinematic ? "✓" : "✗") << std::endl;
            std::cout << "  Mass: " << dynamicActor->getMass() << std::endl;
        }
    } else {
        std::cout << "ERROR: Shape not attached to any actor!" << std::endl;
    }
    
    // Verificar consistencia
    bool configurationValid = true;
    if (isTriggerShape) {
        if (!isCurrentlyTrigger) {
            std::cout << "ERROR: Expected trigger but shape flag not set!" << std::endl;
            configurationValid = false;
        }
        if (isSimulation) {
            std::cout << "ERROR: Trigger should not have simulation flag!" << std::endl;
            configurationValid = false;
        }
        if (filterData.word2 != 0x1) {
            std::cout << "ERROR: Trigger filter flag not set!" << std::endl;
            configurationValid = false;
        }
    }
    
    std::cout << "Configuration: " << (configurationValid ? "VALID ✓" : "INVALID ✗") << std::endl;
    std::cout << "==========================" << std::endl;
}

// Material properties
void Collider::setFriction(float newFriction) {
    friction = std::max(0.0f, newFriction);
    if (material) {
        material->setStaticFriction(friction);
        material->setDynamicFriction(friction);
    }
}

void Collider::setRestitution(float newRestitution) {
    restitution = std::clamp(newRestitution, 0.0f, 1.0f);
    if (material) {
        material->setRestitution(restitution);
    }
}

// Shape properties
void Collider::setShapeType(ShapeType type) {
    if (shapeType == type) return;
    
    shapeType = type;
    
    // Recreate shape with new type
    if (initialized) {
        destroy();
        initializeCollider();
    }
}

void Collider::setTrigger(bool isTrigger) {
    std::cout << "=== SET TRIGGER DEBUG (Collider) ===" << std::endl;
    std::cout << "Setting trigger: " << (isTrigger ? "TRUE" : "FALSE") << " for " << owner->Name << std::endl;
    std::cout << "Shape exists: " << (shape ? "YES" : "NO") << std::endl;
    std::cout << "Initialized: " << (initialized ? "YES" : "NO") << std::endl;
    
    bool wasTriggerbefore = isTriggerShape;
    isTriggerShape = isTrigger;
    
    if (initialized && shape) {
        // If we're changing trigger state, force recreation for reliability
        if (wasTriggerbefore != isTriggerShape) {
            std::cout << "[Collider] Trigger state changed, forcing recreation..." << std::endl;
            recreateShapeSafely();
            std::cout << "[Collider] Shape recreated with new trigger state" << std::endl;
        } else {
            try {
                // Configure shape flags like PhysicalObject
                configureShapeFlags();
                
                // Update filter data for custom filter shader (like PhysicalObject)
                physx::PxFilterData filterData = shape->getSimulationFilterData();
                filterData.word2 = isTriggerShape ? 0x1 : 0x0;  // Trigger flag
                shape->setSimulationFilterData(filterData);
                shape->setQueryFilterData(filterData);
                std::cout << "[Collider] Filter data updated successfully" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "[Collider] Error in setTrigger: " << e.what() << std::endl;
                std::cout << "[Collider] Shape is shared, recreating..." << std::endl;
                // If we get an error, try to recreate the shape (like PhysicalObject)
                recreateShapeSafely();
                std::cout << "[Collider] Shape recreated successfully" << std::endl;
            }
        }
        
        // Always verify the final setup
        verifyTriggerSetup();
        
        // Debug output
        std::cout << "[Collider] Set trigger mode to " << (isTrigger ? "TRUE" : "FALSE") 
                  << " for " << owner->Name << std::endl;
    } else {
        std::cout << "[Collider] Cannot set trigger - not initialized or shape is null" << std::endl;
    }
    
    std::cout << "==================================" << std::endl;
}

void Collider::setBoxHalfExtents(const glm::vec3& extents) {
    boxHalfExtents = glm::vec3(
        std::max(0.01f, extents.x),
        std::max(0.01f, extents.y),
        std::max(0.01f, extents.z)
    );
    
    if (initialized && shapeType == ShapeType::Box) {
        // Recreate shape with new extents
        destroy();
        initializeCollider();
    }
}

void Collider::setSphereRadius(float radius) {
    sphereRadius = std::max(0.01f, radius);
    
    if (initialized && shapeType == ShapeType::Sphere) {
        destroy();
        initializeCollider();
    }
}

void Collider::setCapsuleRadius(float radius) {
    capsuleRadius = std::max(0.01f, radius);
    
    if (initialized && shapeType == ShapeType::Capsule) {
        destroy();
        initializeCollider();
    }
}

void Collider::setCapsuleHalfHeight(float halfHeight) {
    capsuleHalfHeight = std::max(0.01f, halfHeight);
    
    if (initialized && shapeType == ShapeType::Capsule) {
        destroy();
        initializeCollider();
    }
}

// Event callbacks
void Collider::setTriggerCallback(std::function<void(Collider*, Collider*)> callback) {
    triggerCallback = callback;
}

void Collider::setContactCallback(std::function<void(Collider*, Collider*, const glm::vec3&, const glm::vec3&, float)> callback) {
    contactCallback = callback;
}

// Collision filters
void Collider::setCollisionGroup(CollisionGroup group) {
    currentLayer = static_cast<physx::PxU32>(group);
    forceUpdateCollisionFilters();
}

void Collider::setCollisionMask(CollisionMask mask) {
    currentLayerMask = static_cast<physx::PxU32>(mask);
    forceUpdateCollisionFilters();
}

void Collider::setupCollisionFilters(CollisionGroup group, CollisionMask mask) {
    currentLayer = static_cast<physx::PxU32>(group);
    currentLayerMask = static_cast<physx::PxU32>(mask);
    forceUpdateCollisionFilters();
}

void Collider::setLayer(physx::PxU32 layer) {
    currentLayer = layer;
    forceUpdateCollisionFilters();
}

void Collider::setLayerMask(physx::PxU32 layerMask) {
    currentLayerMask = layerMask;
    forceUpdateCollisionFilters();
}

void Collider::forceUpdateCollisionFilters() {
    if (!shape) return;
    
    physx::PxFilterData filterData;
    filterData.word0 = currentLayer;           // Collision group/layer
    filterData.word1 = currentLayerMask;       // Collision mask - qué capas puede colisionar
    filterData.word2 = isTriggerShape ? 0x1 : 0x0; // Trigger flag
    filterData.word3 = 0;                      // Reservado para uso futuro
    
    // Aplicar datos de filtro
    shape->setSimulationFilterData(filterData);
    shape->setQueryFilterData(filterData);
    
    std::cout << "[Collider] Updated collision filters for " << owner->Name << std::endl;
    std::cout << "  Layer (word0): " << filterData.word0 << std::endl;
    std::cout << "  Mask (word1): " << filterData.word1 << std::endl; 
    std::cout << "  Trigger (word2): " << filterData.word2 << std::endl;
    
    // Verificar consistencia de flags
    bool currentlyTrigger = shape->getFlags().isSet(physx::PxShapeFlag::eTRIGGER_SHAPE);
    if (currentlyTrigger != isTriggerShape) {
        std::cout << "[Collider] WARNING: Shape trigger flag inconsistent! Fixing..." << std::endl;
        configureShapeFlags();
    }
}

std::string Collider::getShapeTypeString() const {
    switch (shapeType) {
        case ShapeType::Box: return "Box";
        case ShapeType::Sphere: return "Sphere";
        case ShapeType::Capsule: return "Capsule";
        case ShapeType::Plane: return "Plane";
        default: return "Unknown";
    }
}

void Collider::debugCollisionFilters() {
    if (!shape) {
        std::cout << "[Collider] No shape to debug" << std::endl;
        return;
    }
    
    physx::PxFilterData filterData = shape->getSimulationFilterData();
    std::cout << "[Collider] Debug filters for " << owner->Name << ":" << std::endl;
    std::cout << "  Word0 (Group): " << filterData.word0 << std::endl;
    std::cout << "  Word1 (Mask): " << filterData.word1 << std::endl;
    std::cout << "  Word2 (Trigger): " << filterData.word2 << std::endl;
    std::cout << "  Word3: " << filterData.word3 << std::endl;
}

void Collider::setupAsTrigger(physx::PxU32 triggerLayer, physx::PxU32 collisionMask) {
    std::cout << "=== SETUP AS TRIGGER (Improved) ===" << std::endl;
    std::cout << "[Collider] Setting up " << owner->Name << " as trigger" << std::endl;
    std::cout << "  Layer: " << triggerLayer << ", Mask: " << collisionMask << std::endl;
    
    // 1. Primero configurar las propiedades
    currentLayer = triggerLayer;
    currentLayerMask = collisionMask;
    isTriggerShape = true;
    
    // 2. Si ya está inicializado, aplicar los cambios
    if (initialized && shape) {
        // Configurar flags de shape
        configureShapeFlags();
        
        // Configurar filtros de colisión
        forceUpdateCollisionFilters();
        
        // Verificar configuración final
        verifyTriggerSetup();
    }
    
    std::cout << "[Collider] Trigger setup completed for " << owner->Name << std::endl;
    std::cout << "=================================" << std::endl;
}

void Collider::attachToRigidbody(Rigidbody* rigidbody) {
    if (!shape || !rigidbody || !rigidbody->getRigidActor()) return;
    
    // If we have our own static actor, remove it first
    if (staticActor) {
        auto& physicsManager = PhysicsManager::getInstance();
        if (physicsManager.getScene()) {
            physicsManager.getScene()->removeActor(*staticActor);
        }
        staticActor->release();
        staticActor = nullptr;
        std::cout << "[Collider] Removed independent static actor" << std::endl;
    }
    
    // Attach shape to rigidbody actor
    rigidbody->getRigidActor()->attachShape(*shape);
    
    std::cout << "[Collider] Attached to Rigidbody on " << owner->Name << std::endl;
}

void Collider::detachFromRigidbody() {
    if (!shape) return;
    
    // Find rigidbody component and detach
    auto rigidbody = owner->getComponent<Rigidbody>();
    if (rigidbody && rigidbody->getRigidActor()) {
        rigidbody->getRigidActor()->detachShape(*shape);
        std::cout << "[Collider] Detached from Rigidbody on " << owner->Name << std::endl;
        
        // Create independent actor so collider continues to work
        createIndependentActorIfNeeded();
    }
}

// Serialization
std::string Collider::serializeComponent() const {
    json j;
    j["shapeType"] = static_cast<int>(shapeType);
    j["friction"] = friction;
    j["restitution"] = restitution;
    j["isTrigger"] = isTriggerShape;
    j["currentLayer"] = currentLayer;
    j["currentLayerMask"] = currentLayerMask;
    
    // Shape-specific properties
    j["boxHalfExtents"] = {boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z};
    j["sphereRadius"] = sphereRadius;
    j["capsuleRadius"] = capsuleRadius;
    j["capsuleHalfHeight"] = capsuleHalfHeight;
    
    return j.dump();
}

void Collider::deserialize(const std::string& data) {
    try {
        json j = json::parse(data);
        
        if (j.contains("shapeType")) {
            shapeType = static_cast<ShapeType>(j["shapeType"]);
        }
        if (j.contains("friction")) {
            friction = j["friction"];
        }
        if (j.contains("restitution")) {
            restitution = j["restitution"];
        }
        if (j.contains("isTrigger")) {
            isTriggerShape = j["isTrigger"];
        }
        if (j.contains("currentLayer")) {
            currentLayer = j["currentLayer"];
        }
        if (j.contains("currentLayerMask")) {
            currentLayerMask = j["currentLayerMask"];
        }
        
        // Shape-specific properties
        if (j.contains("boxHalfExtents") && j["boxHalfExtents"].is_array() && j["boxHalfExtents"].size() >= 3) {
            boxHalfExtents = glm::vec3(j["boxHalfExtents"][0], j["boxHalfExtents"][1], j["boxHalfExtents"][2]);
        }
        if (j.contains("sphereRadius")) {
            sphereRadius = j["sphereRadius"];
        }
        if (j.contains("capsuleRadius")) {
            capsuleRadius = j["capsuleRadius"];
        }
        if (j.contains("capsuleHalfHeight")) {
            capsuleHalfHeight = j["capsuleHalfHeight"];
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[Collider] Deserialization error: " << e.what() << std::endl;
    }
}

// Transform synchronization methods
void Collider::syncTransformToPhysX() {
    if (!staticActor) return;
    
    // Get current transform from GameObject
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();
    
    physx::PxTransform transform(
        physx::PxVec3(position.x, position.y, position.z),
        physx::PxQuat(rotation.x, rotation.y, rotation.z, rotation.w)
    );
    
    staticActor->setGlobalPose(transform);
}

void Collider::updateActorTransform() {
    if (!staticActor) return;
    
    // Get current transform from GameObject
    glm::vec3 currentPosition = owner->getWorldPosition();
    glm::quat currentRotation = owner->getWorldRotationQuat();
    
    // Get current PhysX transform
    physx::PxTransform pxTransform = staticActor->getGlobalPose();
    glm::vec3 pxPosition(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
    glm::quat pxRotation(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
    
    // Check if transform has changed (with small tolerance for floating point)
    const float positionTolerance = 0.001f;
    const float rotationTolerance = 0.001f;
    
    bool positionChanged = glm::distance(currentPosition, pxPosition) > positionTolerance;
    bool rotationChanged = glm::abs(glm::dot(currentRotation, pxRotation)) < (1.0f - rotationTolerance);
    
    if (positionChanged || rotationChanged) {
        // Update PhysX actor transform
        physx::PxTransform newTransform(
            physx::PxVec3(currentPosition.x, currentPosition.y, currentPosition.z),
            physx::PxQuat(currentRotation.x, currentRotation.y, currentRotation.z, currentRotation.w)
        );
        
        staticActor->setGlobalPose(newTransform);
        
        // Optional: Debug output (remove if too verbose)
        // std::cout << "[Collider] Updated static actor position for " << owner->Name << std::endl;
    }
}

void Collider::forceTransformUpdate() {
    if (staticActor) {
        syncTransformToPhysX();
        transformNeedsUpdate = false; // Reset dirty flag
    }
}

void Collider::markTransformDirty() {
    transformNeedsUpdate = true;
}

void Collider::recreateShapeSafely() {
    std::cout << "[Collider] Starting safe shape recreation for " << owner->Name << std::endl;
    
    // Guardar propiedades actuales
    bool wasTrigger = isTriggerShape;
    physx::PxU32 savedLayer = currentLayer;
    physx::PxU32 savedMask = currentLayerMask;
    
    // Determinar si tenemos rigidbody
    auto rigidbody = owner->getComponent<Rigidbody>();
    bool hasRigidbody = (rigidbody && rigidbody->getRigidActor());
    
    // Limpiar configuración actual
    destroy();
    
    // Recrear
    initializeCollider();
    
    // Restaurar propiedades
    if (initialized && shape) {
        isTriggerShape = wasTrigger;
        currentLayer = savedLayer;
        currentLayerMask = savedMask;
        
        // Aplicar configuración
        configureShapeFlags();
        forceUpdateCollisionFilters();
        
        std::cout << "[Collider] Shape recreated successfully" << std::endl;
        
        // Verificar resultado
        verifyTriggerSetup();
    } else {
        std::cout << "[Collider] ERROR: Failed to recreate shape!" << std::endl;
    }
}