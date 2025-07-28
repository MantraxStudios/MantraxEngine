#pragma once

#include "PhysicsEvents.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>

// Forward declarations
class PhysicalObject;

class PhysicsEventHandler {
private:
    // Trigger callbacks
    std::unordered_map<physx::PxActor*, std::function<void(const TriggerEvent&)>> triggerCallbacks;
    
    // Contact callbacks
    std::unordered_map<physx::PxActor*, std::function<void(const ContactEvent&)>> contactCallbacks;
    
    // Actor to PhysicalObject mapping
    std::unordered_map<physx::PxActor*, PhysicalObject*> actorToPhysicalObject;
    
public:
    PhysicsEventHandler() = default;
    ~PhysicsEventHandler() = default;
    
    // Trigger system
    void registerTriggerCallback(physx::PxActor* triggerActor, std::function<void(const TriggerEvent&)> callback);
    void unregisterTriggerCallback(physx::PxActor* triggerActor);
    void processTriggerEvent(const TriggerEvent& event);
    
    // Contact system
    void registerContactCallback(physx::PxActor* actor, std::function<void(const ContactEvent&)> callback);
    void unregisterContactCallback(physx::PxActor* actor);
    void processContactEvent(const ContactEvent& event);
    
    // Actor mapping
    void registerPhysicalObject(physx::PxActor* actor, PhysicalObject* physicalObject);
    PhysicalObject* getPhysicalObject(physx::PxActor* actor);
    
    // Clear all data
    void clear();
}; 