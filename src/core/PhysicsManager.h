#pragma once

#include "../core/CoreExporter.h"
#include <MantraxPhysics/MWorld.h>

class MANTRAXCORE_API PhysicsManager {
public:
    static PhysicsManager& getInstance();
    MWorld* physicsWorld;

    bool initialize();
    void update(float deltaTime);
    void cleanup();
    
    // Getter for the physics world
    MWorld* getWorld() const { return physicsWorld; }
    
    // Collision system helpers
    void updateCollisions();

private:
    PhysicsManager();
    ~PhysicsManager();

    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;
}; 