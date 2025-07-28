#include "PhysicsManager.h"
#include "PhysicsEventHandler.h"
#include "PhysicsEventCallback.h"
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxDefaultCpuDispatcher.h>
#include <physx/extensions/PxDefaultSimulationFilterShader.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <iostream>

PhysicsManager *PhysicsManager::instance = nullptr;

// Custom filter shader implementation
physx::PxFilterFlags CustomFilterShader(
    physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
    physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
    physx::PxPairFlags &pairFlags, const void *constantBlock, physx::PxU32 constantBlockSize)
{
    // Debug: Check if either object is a trigger
    bool isTrigger0 = physx::PxFilterObjectIsTrigger(attributes0);
    bool isTrigger1 = physx::PxFilterObjectIsTrigger(attributes1);

    std::cout << "=== FILTER SHADER DEBUG ===" << std::endl;
    std::cout << "Object 0 - Trigger: " << (isTrigger0 ? "YES" : "NO") << ", Group: " << filterData0.word0 << ", Mask: " << filterData0.word1 << ", Trigger Flag: " << filterData0.word2 << std::endl;
    std::cout << "Object 1 - Trigger: " << (isTrigger1 ? "YES" : "NO") << ", Group: " << filterData1.word0 << ", Mask: " << filterData1.word1 << ", Trigger Flag: " << filterData1.word2 << std::endl;

    // Check for triggers - si cualquiera de los objetos es un trigger
    if (isTrigger0 || isTrigger1 || filterData0.word2 & 0x1 || filterData1.word2 & 0x1)
    {
        // Configurar flags para notificaciones de trigger
        pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT |
                    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND |
                    physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
        std::cout << "TRIGGER DETECTED - Setting trigger notification flags" << std::endl;
        std::cout << "==========================" << std::endl;
        return physx::PxFilterFlag::eDEFAULT;
    }

    // Suppress kinematic-kinematic collisions
    if (physx::PxFilterObjectIsKinematic(attributes0) && physx::PxFilterObjectIsKinematic(attributes1))
    {
        std::cout << "KINEMATIC-KINEMATIC SUPPRESSED" << std::endl;
        std::cout << "==========================" << std::endl;
        return physx::PxFilterFlag::eSUPPRESS;
    }

    // Layer-based collision filtering
    if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
    {
        pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT |       // Habilitar contactos básicos
                    physx::PxPairFlag::eSOLVE_CONTACT |         // Resolver contactos físicamente
                    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND |    // Notificar cuando se encuentra contacto
                    physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | // Notificar mientras el contacto persiste
                    physx::PxPairFlag::eNOTIFY_TOUCH_LOST |     // Notificar cuando se pierde el contacto
                    physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;  // Proporcionar puntos de contacto

        std::cout << "NORMAL COLLISION ALLOWED" << std::endl;
        std::cout << "==========================" << std::endl;
        return physx::PxFilterFlag::eDEFAULT;
    }

    std::cout << "COLLISION SUPPRESSED (no layer match)" << std::endl;
    std::cout << "==========================" << std::endl;
    return physx::PxFilterFlag::eSUPPRESS;
}

// REMOVED: Custom filter shader - now using default PhysX filter shader
// The event handling is now done through the PhysicsEventCallback system

PhysicsManager::PhysicsManager()
{
    foundation = nullptr;
    physics = nullptr;
    scene = nullptr;
    defaultMaterial = nullptr;
    cpuDispatcher = nullptr;
    controllerManager = nullptr;

    // Initialize tolerance scale for better physics simulation
    toleranceScale.length = 100;
    toleranceScale.speed = 981;

    // Initialize PVD objects
    pvd = nullptr;
    pvdTransport = nullptr;
    pvdClient = nullptr;

    eventHandler = new PhysicsEventHandler();
    eventCallback = new PhysicsEventCallback(eventHandler);
}

PhysicsManager::~PhysicsManager()
{
    cleanup();
    if (eventCallback)
    {
        delete eventCallback;
        eventCallback = nullptr;
    }
    if (eventHandler)
    {
        delete eventHandler;
        eventHandler = nullptr;
    }
}

PhysicsManager &PhysicsManager::getInstance()
{
    if (!instance)
    {
        instance = new PhysicsManager();
    }
    return *instance;
}

bool PhysicsManager::initialize()
{
    // Initialize PhysX foundation
    foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!foundation)
    {
        std::cerr << "PxCreateFoundation failed!" << std::endl;
        return false;
    }

    // Initialize PVD (PhysX Visual Debugger)
    if (!initializePVD())
    {
        std::cerr << "PVD initialization failed!" << std::endl;
        // Continue without PVD - it's optional for debugging
    }

    // Initialize PhysX physics with PVD and custom tolerance scale
    bool recordMemoryAllocations = true;
    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, toleranceScale, recordMemoryAllocations, pvd);
    if (!physics)
    {
        std::cerr << "PxCreatePhysics failed!" << std::endl;
        return false;
    }

    // Create CPU dispatcher
    cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);
    if (!cpuDispatcher)
    {
        std::cerr << "PxDefaultCpuDispatcherCreate failed!" << std::endl;
        return false;
    }

    // Create scene with custom filter shader
    physx::PxSceneDesc sceneDesc(toleranceScale);
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    sceneDesc.cpuDispatcher = cpuDispatcher;
    sceneDesc.filterShader = CustomFilterShader;       // Use custom filter shader
    sceneDesc.simulationEventCallback = eventCallback; // Use the separate event callback

    // Enhanced scene flags
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

    // Enhanced filtering modes
    sceneDesc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
    sceneDesc.staticKineFilteringMode = physx::PxPairFilteringMode::eKEEP;

    // Validate scene descriptor
    if (!sceneDesc.isValid())
    {
        std::cout << "\nScene Descriptor Validation:" << std::endl;
        std::cout << "CPU Dispatcher: " << (sceneDesc.cpuDispatcher ? "OK" : "Missing") << std::endl;
        std::cout << "Filter Shader: " << (sceneDesc.filterShader ? "OK" : "Missing") << std::endl;
        std::cout << "Gravity: " << sceneDesc.gravity.x << ", " << sceneDesc.gravity.y << ", " << sceneDesc.gravity.z << std::endl;
        std::cerr << "Invalid scene descriptor!" << std::endl;
        return false;
    }

    scene = physics->createScene(sceneDesc);
    if (!scene)
    {
        std::cerr << "createScene failed!" << std::endl;
        return false;
    }

    // Create controller manager
    controllerManager = PxCreateControllerManager(*scene);
    if (!controllerManager)
    {
        std::cerr << "PxCreateControllerManager failed!" << std::endl;
        return false;
    }
    std::cout << "Controller manager created successfully." << std::endl;

    // Setup PVD client for the scene
    if (pvd && pvd->isConnected())
    {
        pvdClient = scene->getScenePvdClient();
        if (pvdClient)
        {
            setupPvdFlags();
        }
    }

    // Create default material
    defaultMaterial = physics->createMaterial(0.5f, 0.5f, 0.1f);
    if (!defaultMaterial)
    {
        std::cerr << "createMaterial failed!" << std::endl;
        return false;
    }

    std::cout << "PhysicsManager initialized successfully with custom filter shader!" << std::endl;

    // Verify trigger configuration
    verifyTriggerConfiguration();

    return true;
}

// PVD (PhysX Visual Debugger) methods
bool PhysicsManager::initializePVD()
{
    if (!foundation)
    {
        std::cerr << "PVD initialization failed: Foundation not initialized!" << std::endl;
        return false;
    }

    // Create PVD
    pvd = PxCreatePvd(*foundation);
    if (!pvd)
    {
        std::cerr << "PxCreatePvd failed!" << std::endl;
        return false;
    }

    // Create PVD transport (default: TCP/IP)
    pvdTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    if (!pvdTransport)
    {
        std::cerr << "PVD transport creation failed!" << std::endl;
        return false;
    }

    // Connect PVD
    if (!pvd->connect(*pvdTransport, physx::PxPvdInstrumentationFlag::eALL))
    {
        std::cerr << "PVD connection failed!" << std::endl;
        return false;
    }

    std::cout << "PVD initialized successfully!" << std::endl;
    return true;
}

void PhysicsManager::setupPvdFlags()
{
    if (!pvdClient)
    {
        std::cerr << "Cannot setup PVD flags: PVD client not initialized!" << std::endl;
        return;
    }

    // Configure PVD flags for debugging
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
    pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);

    std::cout << "PVD flags configured successfully!" << std::endl;
}

void PhysicsManager::connectPVD()
{
    if (!pvd || !pvdTransport)
    {
        std::cerr << "Cannot connect PVD: PVD not initialized!" << std::endl;
        return;
    }

    if (!pvd->isConnected())
    {
        if (pvd->connect(*pvdTransport, physx::PxPvdInstrumentationFlag::eALL))
        {
            std::cout << "PVD connected successfully!" << std::endl;
        }
        else
        {
            std::cerr << "PVD connection failed!" << std::endl;
        }
    }
    else
    {
        std::cout << "PVD already connected!" << std::endl;
    }
}

void PhysicsManager::disconnectPVD()
{
    if (pvd && pvd->isConnected())
    {
        pvd->disconnect();
        std::cout << "PVD disconnected!" << std::endl;
    }
}

bool PhysicsManager::isPVDConnected() const
{
    return pvd && pvd->isConnected();
}

void PhysicsManager::update(float deltaTime)
{
    if (scene)
    {
        scene->simulate(deltaTime);
        scene->fetchResults(true);

        // Debug: Log that physics simulation completed
        static int frameCount = 0;
        frameCount++;
        if (frameCount % 60 == 0)
        { // Log every 60 frames (1 second at 60fps)
            std::cout << "Physics simulation frame: " << frameCount << std::endl;
        }
    }
}

void PhysicsManager::cleanup()
{
    // Disconnect PVD
    disconnectPVD();

    if (scene)
    {
        scene->release();
        scene = nullptr;
    }
    if (controllerManager)
    {
        controllerManager->release();
        controllerManager = nullptr;
    }
    if (cpuDispatcher)
    {
        cpuDispatcher->release();
        cpuDispatcher = nullptr;
    }
    if (physics)
    {
        physics->release();
        physics = nullptr;
    }

    // Cleanup PVD objects
    if (pvdTransport)
    {
        pvdTransport->release();
        pvdTransport = nullptr;
    }
    if (pvd)
    {
        pvd->release();
        pvd = nullptr;
    }

    if (foundation)
    {
        foundation->release();
        foundation = nullptr;
    }
}

physx::PxMaterial *PhysicsManager::createMaterial(float staticFriction, float dynamicFriction, float restitution)
{
    if (physics)
    {
        return physics->createMaterial(staticFriction, dynamicFriction, restitution);
    }
    return nullptr;
}

physx::PxRigidDynamic *PhysicsManager::createDynamicBody(const physx::PxTransform &transform, float mass)
{
    if (physics)
    {
        physx::PxRigidDynamic *dynamic = physics->createRigidDynamic(transform);
        if (dynamic)
        {
            physx::PxRigidBodyExt::updateMassAndInertia(*dynamic, mass);
        }
        return dynamic;
    }
    return nullptr;
}

physx::PxRigidStatic *PhysicsManager::createStaticBody(const physx::PxTransform &transform)
{
    if (physics)
    {
        return physics->createRigidStatic(transform);
    }
    return nullptr;
}

physx::PxShape *PhysicsManager::createBoxShape(const physx::PxVec3 &halfExtents, physx::PxMaterial *material)
{
    if (physics)
    {
        physx::PxBoxGeometry geometry(halfExtents);
        // El tercer parámetro 'true' hace que el shape sea exclusivo (no compartido)
        physx::PxShape *shape = physics->createShape(geometry, material ? *material : *defaultMaterial, true);

        // Ensure the shape is unique and not shared
        if (shape)
        {
            // Set unique flags to prevent sharing
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);

            // Set unique filter data to prevent sharing
            physx::PxFilterData filterData;
            filterData.word0 = 0;
            filterData.word1 = 0;
            filterData.word2 = 0;
            filterData.word3 = 0;
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
        }
        return shape;
    }
    return nullptr;
}

physx::PxShape *PhysicsManager::createSphereShape(float radius, physx::PxMaterial *material)
{
    if (physics)
    {
        physx::PxSphereGeometry geometry(radius);
        physx::PxShape *shape = physics->createShape(geometry, material ? *material : *defaultMaterial);

        // Ensure the shape is unique and not shared
        if (shape)
        {
            // Set unique flags to prevent sharing
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);

            // Set unique filter data to prevent sharing
            physx::PxFilterData filterData;
            filterData.word0 = 0;
            filterData.word1 = 0;
            filterData.word2 = 0;
            filterData.word3 = 0;
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
        }

        return shape;
    }
    return nullptr;
}

physx::PxShape *PhysicsManager::createCapsuleShape(float radius, float halfHeight, physx::PxMaterial *material)
{
    if (physics)
    {
        physx::PxCapsuleGeometry geometry(radius, halfHeight);
        physx::PxShape *shape = physics->createShape(geometry, material ? *material : *defaultMaterial);

        // Ensure the shape is unique and not shared
        if (shape)
        {
            // Set unique flags to prevent sharing
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);

            // Set unique filter data to prevent sharing
            physx::PxFilterData filterData;
            filterData.word0 = 0;
            filterData.word1 = 0;
            filterData.word2 = 0;
            filterData.word3 = 0;
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
        }

        return shape;
    }
    return nullptr;
}

physx::PxShape *PhysicsManager::createPlaneShape(physx::PxMaterial *material)
{
    if (physics)
    {
        physx::PxPlaneGeometry geometry;
        physx::PxShape *shape = physics->createShape(geometry, material ? *material : *defaultMaterial);

        // Ensure the shape is unique and not shared
        if (shape)
        {
            // Set unique flags to prevent sharing
            shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
            shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);

            // Set unique filter data to prevent sharing
            physx::PxFilterData filterData;
            filterData.word0 = 0;
            filterData.word1 = 0;
            filterData.word2 = 0;
            filterData.word3 = 0;
            shape->setSimulationFilterData(filterData);
            shape->setQueryFilterData(filterData);
        }

        return shape;
    }
    return nullptr;
}

void PhysicsManager::addActor(physx::PxActor &actor)
{
    if (scene)
    {
        scene->addActor(actor);
    }
}

void PhysicsManager::removeActor(physx::PxActor &actor)
{
    if (scene)
    {
        scene->removeActor(actor);
    }
}

void PhysicsManager::setGravity(const physx::PxVec3 &gravity)
{
    if (scene)
    {
        scene->setGravity(gravity);
    }
}

physx::PxVec3 PhysicsManager::getGravity() const
{
    if (scene)
    {
        return scene->getGravity();
    }
    return physx::PxVec3(0.0f, -9.81f, 0.0f);
}

RaycastHit PhysicsManager::raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance)
{
    RaycastHit hit;
    hit.hit = false;

    if (!scene)
        return hit;

    physx::PxRaycastBuffer buffer;
    physx::PxQueryFilterData filterData;
    filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

    bool status = scene->raycast(
        physx::PxVec3(origin.x, origin.y, origin.z),
        physx::PxVec3(direction.x, direction.y, direction.z).getNormalized(),
        maxDistance,
        buffer,
        physx::PxHitFlag::eDEFAULT,
        filterData);

    if (status && buffer.hasBlock)
    {
        hit.hit = true;
        hit.position = glm::vec3(buffer.block.position.x, buffer.block.position.y, buffer.block.position.z);
        hit.normal = glm::vec3(buffer.block.normal.x, buffer.block.normal.y, buffer.block.normal.z);
        hit.distance = buffer.block.distance;
        hit.actor = buffer.block.actor;
        hit.shape = buffer.block.shape;
    }

    return hit;
}

std::vector<RaycastHit> PhysicsManager::raycastAll(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance)
{
    std::vector<RaycastHit> hits;

    if (!scene)
        return hits;

    physx::PxRaycastBuffer buffer;
    physx::PxQueryFilterData filterData;
    filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;

    bool status = scene->raycast(
        physx::PxVec3(origin.x, origin.y, origin.z),
        physx::PxVec3(direction.x, direction.y, direction.z).getNormalized(),
        maxDistance,
        buffer,
        physx::PxHitFlag::eDEFAULT,
        filterData);

    if (status)
    {
        // Add the blocking hit
        if (buffer.hasBlock)
        {
            RaycastHit hit;
            hit.hit = true;
            hit.position = glm::vec3(buffer.block.position.x, buffer.block.position.y, buffer.block.position.z);
            hit.normal = glm::vec3(buffer.block.normal.x, buffer.block.normal.y, buffer.block.normal.z);
            hit.distance = buffer.block.distance;
            hit.actor = buffer.block.actor;
            hit.shape = buffer.block.shape;
            hits.push_back(hit);
        }

        // Add all touching hits
        for (physx::PxU32 i = 0; i < buffer.nbTouches; i++)
        {
            RaycastHit hit;
            hit.hit = true;
            hit.position = glm::vec3(buffer.touches[i].position.x, buffer.touches[i].position.y, buffer.touches[i].position.z);
            hit.normal = glm::vec3(buffer.touches[i].normal.x, buffer.touches[i].normal.y, buffer.touches[i].normal.z);
            hit.distance = buffer.touches[i].distance;
            hit.actor = buffer.touches[i].actor;
            hit.shape = buffer.touches[i].shape;
            hits.push_back(hit);
        }
    }

    return hits;
}

// METODO ADICIONAL: Verificar configuración de scene con default filter shader
void PhysicsManager::verifyTriggerConfiguration()
{
    if (!scene)
    {
        std::cout << "Scene is null!" << std::endl;
        return;
    }

    std::cout << "=== SCENE TRIGGER CONFIGURATION (DEFAULT FILTER SHADER) ===" << std::endl;

    // Verificar que el callback esté configurado
    physx::PxSimulationEventCallback *callback = scene->getSimulationEventCallback();
    if (callback)
    {
        std::cout << "Simulation Event Callback: CONFIGURED" << std::endl;
    }
    else
    {
        std::cout << "Simulation Event Callback: NOT CONFIGURED" << std::endl;
    }

    // Verificar flags de la escena
    physx::PxSceneFlags sceneFlags = scene->getFlags();
    std::cout << "Scene Flags:" << std::endl;
    std::cout << "  eENABLE_CCD: " << ((sceneFlags & physx::PxSceneFlag::eENABLE_CCD) ? "YES" : "NO") << std::endl;

    // Verificar que estamos usando el custom filter shader
    std::cout << "Filter Shader: CUSTOM (CustomFilterShader)" << std::endl;

    // Verificar estado de PVD
    std::cout << "PVD Status:" << std::endl;
    std::cout << "  Connected: " << (isPVDConnected() ? "YES" : "NO") << std::endl;
    if (pvdClient)
    {
        std::cout << "  PVD Client: CONFIGURED" << std::endl;
    }
    else
    {
        std::cout << "  PVD Client: NOT CONFIGURED" << std::endl;
    }

    // Verificar controller manager
    std::cout << "Controller Manager: " << (controllerManager ? "CONFIGURED" : "NOT CONFIGURED") << std::endl;

    // Verificar tolerance scale
    std::cout << "Tolerance Scale:" << std::endl;
    std::cout << "  Length: " << toleranceScale.length << std::endl;
    std::cout << "  Speed: " << toleranceScale.speed << std::endl;

    std::cout << "==================================" << std::endl;
}

void PhysicsManager::setupCollisionFilter(physx::PxShape *shape, CollisionGroup group, CollisionMask mask, bool isTrigger)
{
    if (!shape)
        return;

    physx::PxFilterData filterData;
    filterData.word0 = static_cast<physx::PxU32>(group); // Collision group
    filterData.word1 = static_cast<physx::PxU32>(mask);  // Collision mask
    filterData.word2 = isTrigger ? 0x1 : 0;              // 0 = not trigger, 1 = trigger

    try
    {
        shape->setSimulationFilterData(filterData);
        shape->setQueryFilterData(filterData);
    }
    catch (const std::exception &e)
    {
        std::cout << "Error setting filter data: " << e.what() << std::endl;
        throw std::runtime_error("Cannot modify shape filter data - shape may be shared");
    }
}

void PhysicsManager::setupShapeCollisionFilter(physx::PxShape *shape, CollisionGroup group, CollisionMask mask, bool isTrigger)
{
    setupCollisionFilter(shape, group, mask, isTrigger);
}

// PxSimulationEventCallback implementations are now in the header file

void PhysicsManager::debugTriggerSetup(physx::PxActor *actor, bool isTrigger)
{
    if (!actor)
    {
        std::cout << "DEBUG: Actor is null!" << std::endl;
        return;
    }

    std::cout << "=== DEBUG TRIGGER SETUP ===" << std::endl;
    std::cout << "Actor: " << (void *)actor << std::endl;
    std::cout << "Is Trigger: " << (isTrigger ? "YES" : "NO") << std::endl;

    // Check actor type
    physx::PxActorType::Enum actorType = actor->getType();
    std::cout << "Actor Type: " << (actorType == physx::PxActorType::eRIGID_DYNAMIC ? "DYNAMIC" : actorType == physx::PxActorType::eRIGID_STATIC ? "STATIC"
                                                                                                                                                 : "UNKNOWN")
              << std::endl;

    // Check shapes using the correct PhysX API for rigid actors
    if (actorType == physx::PxActorType::eRIGID_DYNAMIC || actorType == physx::PxActorType::eRIGID_STATIC)
    {
        physx::PxRigidActor *rigidActor = static_cast<physx::PxRigidActor *>(actor);
        physx::PxU32 numShapes = rigidActor->getNbShapes();
        std::cout << "Number of shapes: " << numShapes << std::endl;

        if (numShapes > 0)
        {
            physx::PxShape *shapes[10];
            physx::PxU32 numShapesRetrieved = rigidActor->getShapes(shapes, 10);

            for (physx::PxU32 i = 0; i < numShapesRetrieved; i++)
            {
                physx::PxShape *shape = shapes[i];
                if (shape)
                {
                    physx::PxShapeFlags flags = shape->getFlags();
                    physx::PxFilterData filterData = shape->getSimulationFilterData();

                    std::cout << "Shape " << i << ":" << std::endl;
                    std::cout << "  TRIGGER_SHAPE flag: " << (flags.isSet(physx::PxShapeFlag::eTRIGGER_SHAPE) ? "YES" : "NO") << std::endl;
                    std::cout << "  SIMULATION_SHAPE flag: " << (flags.isSet(physx::PxShapeFlag::eSIMULATION_SHAPE) ? "YES" : "NO") << std::endl;
                    std::cout << "  Filter Word0: " << filterData.word0 << std::endl;
                    std::cout << "  Filter Word1: " << filterData.word1 << std::endl;
                    std::cout << "  Filter Word2: " << filterData.word2 << std::endl;
                }
            }
        }
    }

    std::cout << "==========================" << std::endl;
}