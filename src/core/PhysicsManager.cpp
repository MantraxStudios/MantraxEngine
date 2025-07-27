#include "PhysicsManager.h"
#include <iostream>

PhysicsManager* PhysicsManager::instance = nullptr;

PhysicsManager::PhysicsManager() 
    : foundation(nullptr), physics(nullptr), scene(nullptr), 
      defaultMaterial(nullptr), cpuDispatcher(nullptr) {
}

PhysicsManager::~PhysicsManager() {
    cleanup();
}

PhysicsManager& PhysicsManager::getInstance() {
    if (instance == nullptr) {
        instance = new PhysicsManager();
    }
    return *instance;
}

bool PhysicsManager::initialize() {
    try {
        // Create foundation
        foundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
        if (!foundation) {
            std::cerr << "PxCreateFoundation failed!" << std::endl;
            return false;
        }

        // Create physics
        physx::PxTolerancesScale mToleranceScale;
        mToleranceScale.length = 100;        // typical length of an object
        mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
        
        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, mToleranceScale, true);
        if (!physics) {
            std::cerr << "PxCreatePhysics failed!" << std::endl;
            return false;
        }

        // Create CPU dispatcher
        cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
        if (!cpuDispatcher) {
            std::cerr << "PxDefaultCpuDispatcherCreate failed!" << std::endl;
            return false;
        }
        
        // Verify dispatcher is valid
        if (!cpuDispatcher->getWorkerCount()) {
            std::cerr << "CPU dispatcher has no workers!" << std::endl;
            return false;
        }

        // Create scene
        physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
        sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
        sceneDesc.cpuDispatcher = cpuDispatcher;
        sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
        sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
        
        scene = physics->createScene(sceneDesc);
        if (!scene) {
            std::cerr << "createScene failed!" << std::endl;
            return false;
        }

        // Create default material
        defaultMaterial = physics->createMaterial(0.5f, 0.5f, 0.1f);
        if (!defaultMaterial) {
            std::cerr << "createMaterial failed!" << std::endl;
            return false;
        }

        std::cout << "PhysicsManager initialized successfully!" << std::endl;
        std::cout << "  - Foundation: " << (foundation ? "OK" : "FAILED") << std::endl;
        std::cout << "  - Physics: " << (physics ? "OK" : "FAILED") << std::endl;
        std::cout << "  - Scene: " << (scene ? "OK" : "FAILED") << std::endl;
        std::cout << "  - CPU Dispatcher: " << (cpuDispatcher ? "OK" : "FAILED") << std::endl;
        std::cout << "  - Default Material: " << (defaultMaterial ? "OK" : "FAILED") << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "PhysicsManager initialization failed: " << e.what() << std::endl;
        return false;
    }
}

void PhysicsManager::update(float deltaTime) {
    if (scene && physics && cpuDispatcher) {
        scene->simulate(deltaTime);
        scene->fetchResults(true);
    }
}

void PhysicsManager::cleanup() {
    if (scene) {
        scene->release();
        scene = nullptr;
    }
    
    if (cpuDispatcher) {
        cpuDispatcher->release();
        cpuDispatcher = nullptr;
    }
    
    if (physics) {
        physics->release();
        physics = nullptr;
    }
    
    if (foundation) {
        foundation->release();
        foundation = nullptr;
    }
}

physx::PxMaterial* PhysicsManager::createMaterial(float staticFriction, float dynamicFriction, float restitution) {
    if (physics) {
        return physics->createMaterial(staticFriction, dynamicFriction, restitution);
    }
    return nullptr;
}

physx::PxRigidDynamic* PhysicsManager::createDynamicBody(const physx::PxTransform& transform, float mass) {
    if (physics) {
        physx::PxRigidDynamic* body = physics->createRigidDynamic(transform);
        if (body) {
            physx::PxRigidBodyExt::updateMassAndInertia(*body, mass);
        }
        return body;
    }
    return nullptr;
}

physx::PxRigidStatic* PhysicsManager::createStaticBody(const physx::PxTransform& transform) {
    if (physics) {
        return physics->createRigidStatic(transform);
    }
    return nullptr;
}

physx::PxShape* PhysicsManager::createBoxShape(const physx::PxVec3& halfExtents, physx::PxMaterial* material) {
    if (physics) {
        if (!material) material = defaultMaterial;
        return physics->createShape(physx::PxBoxGeometry(halfExtents.x, halfExtents.y, halfExtents.z), *material);
    }
    return nullptr;
}

physx::PxShape* PhysicsManager::createSphereShape(float radius, physx::PxMaterial* material) {
    if (physics) {
        if (!material) material = defaultMaterial;
        return physics->createShape(physx::PxSphereGeometry(radius), *material);
    }
    return nullptr;
}

physx::PxShape* PhysicsManager::createCapsuleShape(float radius, float halfHeight, physx::PxMaterial* material) {
    if (physics) {
        if (!material) material = defaultMaterial;
        return physics->createShape(physx::PxCapsuleGeometry(radius, halfHeight), *material);
    }
    return nullptr;
}

physx::PxShape* PhysicsManager::createPlaneShape(physx::PxMaterial* material) {
    if (physics) {
        if (!material) material = defaultMaterial;
        return physics->createShape(physx::PxPlaneGeometry(), *material);
    }
    return nullptr;
}

void PhysicsManager::addActor(physx::PxActor& actor) {
    if (scene) {
        scene->addActor(actor);
    }
}

void PhysicsManager::removeActor(physx::PxActor& actor) {
    if (scene) {
        scene->removeActor(actor);
    }
}

void PhysicsManager::setGravity(const physx::PxVec3& gravity) {
    if (scene) {
        scene->setGravity(gravity);
    }
}

physx::PxVec3 PhysicsManager::getGravity() const {
    if (scene) {
        return scene->getGravity();
    }
    return physx::PxVec3(0.0f, -9.81f, 0.0f);
} 