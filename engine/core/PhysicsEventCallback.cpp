#include "PhysicsEventCallback.h"
#include "../components/SceneManager.h"
#include "../components/ScriptExecutor.h"
#include "../components/PhysicalObject.h"

    // PxSimulationEventCallback virtual methods
    void PhysicsEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {
        // Handle constraint break events
        for (physx::PxU32 i = 0; i < count; i++) {
            std::cout << "Constraint broken: " << constraints[i].externalReference << std::endl;
        }
    }

    void PhysicsEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count) {
        // Handle wake events
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Actor woke up: " << (void*)actors[i] << std::endl;
            }
        }
    }

    void PhysicsEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count) {
        // Handle sleep events
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Actor went to sleep: " << (void*)actors[i] << std::endl;
            }
        }
    }

    void PhysicsEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
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

    void PhysicsEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
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
                
                // Find ScriptExecutor components and call OnTriggerEnter
                PhysicalObject* triggerPhysical = eventHandler->getPhysicalObject(pair.triggerActor);
                PhysicalObject* otherPhysical = eventHandler->getPhysicalObject(pair.otherActor);
                
                if (triggerPhysical && otherPhysical) {
                    GameObject* triggerObject = triggerPhysical->getOwner();
                    GameObject* otherObject = otherPhysical->getOwner();
                    
                    if (triggerObject && otherObject) {
                        // Find all ScriptExecutor components on the trigger object
                        auto scriptExecutors = triggerObject->getAllComponents();
                        for (const auto* comp : scriptExecutors) {
                            if (const ScriptExecutor* scriptExec = dynamic_cast<const ScriptExecutor*>(comp)) {
                                // Call OnTriggerEnter on the script
                                const_cast<ScriptExecutor*>(scriptExec)->onTriggerEnter(otherObject);
                            }
                        }
                    }
                }
            }
            else if (pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
                event.type = TriggerEvent::EXIT;
                std::cout << "  Event Type: EXIT" << std::endl;
                
                // Find ScriptExecutor components and call OnTriggerExit
                PhysicalObject* triggerPhysical = eventHandler->getPhysicalObject(pair.triggerActor);
                PhysicalObject* otherPhysical = eventHandler->getPhysicalObject(pair.otherActor);
                
                if (triggerPhysical && otherPhysical) {
                    GameObject* triggerObject = triggerPhysical->getOwner();
                    GameObject* otherObject = otherPhysical->getOwner();
                    
                    if (triggerObject && otherObject) {
                        // Find all ScriptExecutor components on the trigger object
                        auto scriptExecutors = triggerObject->getAllComponents();
                        for (const auto* comp : scriptExecutors) {
                            if (const ScriptExecutor* scriptExec = dynamic_cast<const ScriptExecutor*>(comp)) {
                                // Call OnTriggerExit on the script
                                const_cast<ScriptExecutor*>(scriptExec)->onTriggerExit(otherObject);
                            }
                        }
                    }
                }
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

    void PhysicsEventCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {
        // Handle advance events (early pose integration preview)
        // This is called for rigid bodies with PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW flag set
        for (physx::PxU32 i = 0; i < count; i++) {
            if (eventHandler) {
                std::cout << "Rigid body advanced: " << (void*)bodyBuffer[i] << std::endl;
            }
        }
    }
