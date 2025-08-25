#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "MNodeEngine.h"
#include "../components/Rigidbody.h"

class RigidBodyNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // ----------- GET BODY -----------
        PremakeNode getBodyNode(
            "Rigidbody", "Get Body",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(0, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                if (object != nullptr && object->hasComponent<Rigidbody>())
                {
                    node->SetOutputValue<Rigidbody *>(0, object->getComponent<Rigidbody>());
                }
            },
            SCRIPT, false, false,
            {{"Object", (GameObject *)nullptr}},
            {{"Body", (Rigidbody *)nullptr}},
            position);

        PremakeNode addForceBodyNode(
            "Rigidbody", "Add Force",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->addForce(node->GetInputValue<glm::vec3>(2, glm::vec3(0.0f)), physx::PxForceMode::eFORCE);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Direction", glm::vec3(0.0f)}},
            {},
            position);

        PremakeNode addImpulseBodyNode(
            "Rigidbody", "Add Impulse",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->addForce(node->GetInputValue<glm::vec3>(2, glm::vec3(0.0f)), physx::PxForceMode::eIMPULSE);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Direction", glm::vec3(0.0f)}},
            {},
            position);

        PremakeNode addAccelerationBodyNode(
            "Rigidbody", "Add Acceleration",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->addForce(node->GetInputValue<glm::vec3>(2, glm::vec3(0.0f)), physx::PxForceMode::eACCELERATION);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Direction", glm::vec3(0.0f)}},
            {},
            position);

        PremakeNode enableBodyNode(
            "Rigidbody", "Enable",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->enable();
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Entrada: el rigidbody
            {},                               // No tiene salidas
            position);

        PremakeNode disableBodyNode(
            "Rigidbody", "Disable",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->disable();
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Entrada: el rigidbody
            {},                               // No tiene salidas
            position);

        PremakeNode isActiveBodyNode(
            "Rigidbody", "Is Active",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(0, nullptr);

                if (rgb != nullptr)
                {
                    node->SetOutputValue<bool>(0, rgb->isActive());
                }
            },
            SCRIPT, false, false,
            {{"Body", (Rigidbody *)nullptr}}, // Entrada: Rigidbody
            {{"Active", false}},              // Salida: bool
            position);

        PremakeNode wakeBody(
            "Rigidbody", "Wake Up",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->wakeUp();
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Entrada: el rigidbody
            {},                               // No tiene salidas
            position);

        PremakeNode setVelocityBodyNode(
            "Rigidbody", "Set Velocity",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);
                glm::vec3 vel = node->GetInputValue<glm::vec3>(2, glm::vec3(0.0f));

                if (rgb != nullptr)
                {
                    rgb->setVelocity(vel);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Velocity", glm::vec3(0.0f)}}, // Entradas
            {},                                                              // No salidas
            position);

        PremakeNode getVelocityBodyNode(
            "Rigidbody", "Get Velocity",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(0, nullptr);

                if (rgb != nullptr)
                {
                    node->SetOutputValue<glm::vec3>(0, rgb->getVelocity());
                }
                else
                {
                    node->SetOutputValue<glm::vec3>(0, glm::vec3(0.0f));
                }
            },
            SCRIPT, false, false,
            {{"Body", (Rigidbody *)nullptr}}, // Entrada
            {{"Velocity", glm::vec3(0.0f)}},  // Salida
            position);

        PremakeNode setDampingBodyNode(
            "Rigidbody", "Set Damping",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);
                float damping = node->GetInputValue<float>(2, 0.0f);

                if (rgb != nullptr)
                {
                    rgb->setDamping(damping);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Damping", 0.0f}}, // Inputs (desde 1)
            {},                                                  // Sin outputs
            position);

        PremakeNode setGravityFactorBodyNode(
            "Rigidbody", "Set Gravity Factor",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);
                float factor = node->GetInputValue<float>(2, 1.0f);

                if (rgb != nullptr)
                {
                    rgb->setGravityFactor(factor);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Factor", 1.0f}}, // Inputs (desde 1)
            {},                                                 // Sin outputs
            position);

        PremakeNode setMassBodyNode(
            "Rigidbody", "Set Mass",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);
                float mass = node->GetInputValue<float>(2, 1.0f);

                if (rgb != nullptr)
                {
                    rgb->setMass(mass);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}, {"Mass", 1.0f}}, // Inputs (desde 1)
            {},                                               // Sin outputs
            position);

        PremakeNode setBodyStaticNode(
            "Rigidbody", "Set Body Static",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->setBodyType(BodyType::Static);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Input en 1
            {},                               // Sin outputs
            position);

        PremakeNode setBodyDynamicNode(
            "Rigidbody", "Set Body Dynamic",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->setBodyType(BodyType::Dynamic);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Input en 1
            {},                               // Sin outputs
            position);

        PremakeNode setBodyKinematicNode(
            "Rigidbody", "Set Body Kinematic",
            [](CustomNode *node)
            {
                Rigidbody *rgb = node->GetInputValue<Rigidbody *>(1, nullptr);

                if (rgb != nullptr)
                {
                    rgb->setBodyType(BodyType::Kinematic);
                }
            },
            SCRIPT, true, true,
            {{"Body", (Rigidbody *)nullptr}}, // Input en 1
            {},                               // Sin outputs
            position);

        engine.PrefabNodes.push_back(getBodyNode);
        engine.PrefabNodes.push_back(addForceBodyNode);
        engine.PrefabNodes.push_back(addImpulseBodyNode);
        engine.PrefabNodes.push_back(addAccelerationBodyNode);
        engine.PrefabNodes.push_back(enableBodyNode);
        engine.PrefabNodes.push_back(disableBodyNode);
        engine.PrefabNodes.push_back(isActiveBodyNode);
        engine.PrefabNodes.push_back(wakeBody);

        engine.PrefabNodes.push_back(setVelocityBodyNode);
        engine.PrefabNodes.push_back(getVelocityBodyNode);
        engine.PrefabNodes.push_back(setDampingBodyNode);
        engine.PrefabNodes.push_back(setGravityFactorBodyNode);
        engine.PrefabNodes.push_back(setMassBodyNode);

        engine.PrefabNodes.push_back(setBodyStaticNode);
        engine.PrefabNodes.push_back(setBodyDynamicNode);
        engine.PrefabNodes.push_back(setBodyKinematicNode);
    }
};