#pragma once

#include "PhysicsEvents.h"
#include <physx/PxPhysicsAPI.h>
#include <iostream>

// Forward declarations
class PhysicalObject;
class PhysicsEventHandler;

class PhysicsEventCallback : public physx::PxSimulationEventCallback {
private:
    PhysicsEventHandler* eventHandler;

public:
    PhysicsEventCallback(PhysicsEventHandler* handler) : eventHandler(handler) {}

    // Virtual destructor
    virtual ~PhysicsEventCallback() = default;

    // PxSimulationEventCallback virtual methods
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {
        // Handle constraint break events
        for (physx::PxU32 i = 0; i < count; i++) {
            std::cout << "Constraint broken: " << constraints[i].externalReference << std::endl;
        }
    }

    void onWake(physx::PxActor** actors, physx::PxU32 count) override {
        // Handle wake events
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Actor woke up: " << (void*)actors[i] << std::endl;
            }
        }
    }

    void onSleep(physx::PxActor** actors, physx::PxU32 count) override {
        // Handle sleep events
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Actor went to sleep: " << (void*)actors[i] << std::endl;
            }
        }
    }

    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override {
        // Delegate contact events to event handler
        if (!eventHandler) return;

        for (physx::PxU32 i = 0; i < nbPairs; i++) {
            const physx::PxContactPair& pair = pairs[i];

            // Check if contact was found or lost
            if (pair.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                // Extract contact information
                physx::PxContactPairPoint contactPoints[32];
                physx::PxU32 nbContacts = pair.extractContacts(contactPoints, 32);

                for (physx::PxU32 j = 0; j < nbContacts; j++) {
                    const physx::PxContactPairPoint& contact = contactPoints[j];

                    ContactEvent event;
                    event.type = ContactEvent::BEGIN;
                    event.actor1 = pairHeader.actors[0];
                    event.actor2 = pairHeader.actors[1];
                    event.contactPoint = glm::vec3(contact.position.x, contact.position.y, contact.position.z);
                    event.contactNormal = glm::vec3(contact.normal.x, contact.normal.y, contact.normal.z);
                    event.contactForce = contact.impulse.magnitude();

                    // Process the event through event handler
                    eventHandler->processContactEvent(event);
                }
            }
        }
    }

    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {
        // Delegate trigger events to event handler
        if (!eventHandler) return;
        std::cout << "=== TRIGGER EVENT DETECTED ===" << std::endl;
        std::cout << "Number of trigger pairs: " << count << std::endl;

        for (physx::PxU32 i = 0; i < count; i++) {
            const physx::PxTriggerPair& pair = pairs[i];
            std::cout << "Trigger Pair " << i << ":" << std::endl;
            std::cout << "  Trigger Actor: " << (void*)pair.triggerActor << std::endl;
            std::cout << "  Other Actor: " << (void*)pair.otherActor << std::endl;
            std::cout << "  Status: " << pair.status << std::endl;

            TriggerEvent event;

            // CORRECCIÓN: Usar == en lugar de &
            if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND) {
                event.type = TriggerEvent::ENTER;
                std::cout << "  Event Type: ENTER" << std::endl;
            }
            else if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
                event.type = TriggerEvent::EXIT;
                std::cout << "  Event Type: EXIT" << std::endl;
            }
            else {
                // Para debug, imprimir el valor real
                std::cout << "  Event Type: UNKNOWN (status value: " << (int)pair.status << ")" << std::endl;
                event.type = TriggerEvent::ENTER; // Por defecto
            }

            event.triggerActor = pair.triggerActor;
            event.otherActor = pair.otherActor;

            // Process the event through event handler
            eventHandler->processTriggerEvent(event);
        }
        std::cout << "=== END TRIGGER EVENT ===" << std::endl;
    }

    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {
        // Handle advance events (early pose integration preview)
        // This is called for rigid bodies with PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW flag set
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Rigid body advanced: " << (void*)bodyBuffer[i] << std::endl;
            }
        }
    }
};