#include "PhysicsEventHandler.h"
#include "../components/PhysicalObject.h"

void PhysicsEventHandler::registerTriggerCallback(physx::PxActor* triggerActor, std::function<void(const TriggerEvent&)> callback) {
    if (triggerActor) {
        triggerCallbacks[triggerActor] = callback;
    }
}

void PhysicsEventHandler::unregisterTriggerCallback(physx::PxActor* triggerActor) {
    if (triggerActor) {
        triggerCallbacks.erase(triggerActor);
    }
}

void PhysicsEventHandler::processTriggerEvent(const TriggerEvent& event) {
    auto it = triggerCallbacks.find(event.triggerActor);
    if (it != triggerCallbacks.end() && it->second) {
        it->second(event);
    }
}

void PhysicsEventHandler::registerContactCallback(physx::PxActor* actor, std::function<void(const ContactEvent&)> callback) {
    if (actor) {
        contactCallbacks[actor] = callback;
    }
}

void PhysicsEventHandler::unregisterContactCallback(physx::PxActor* actor) {
    if (actor) {
        contactCallbacks.erase(actor);
    }
}

void PhysicsEventHandler::processContactEvent(const ContactEvent& event) {
    // Process for both actors involved in the contact
    auto it1 = contactCallbacks.find(event.actor1);
    if (it1 != contactCallbacks.end() && it1->second) {
        it1->second(event);
    }
    
    auto it2 = contactCallbacks.find(event.actor2);
    if (it2 != contactCallbacks.end() && it2->second) {
        it2->second(event);
    }
}

void PhysicsEventHandler::registerPhysicalObject(physx::PxActor* actor, PhysicalObject* physicalObject) {
    if (actor && physicalObject) {
        actorToPhysicalObject[actor] = physicalObject;
    }
}

PhysicalObject* PhysicsEventHandler::getPhysicalObject(physx::PxActor* actor) {
    auto it = actorToPhysicalObject.find(actor);
    return (it != actorToPhysicalObject.end()) ? it->second : nullptr;
}

void PhysicsEventHandler::clear() {
    triggerCallbacks.clear();
    contactCallbacks.clear();
    actorToPhysicalObject.clear();
} 