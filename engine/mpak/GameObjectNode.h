#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include "MNodeEngine.h"
#include <imgui/imgui.h>

#include <components/AudioSource.h>
#include <components/CharacterController.h>
#include <components/Collider.h>
#include <components/LightComponent.h>
#include <components/Rigidbody.h>
#include <components/SpriteAnimator.h>

class GameObjectNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // Crear el nodo usando el engine
        PremakeNode setNameNode(
            "Object",
            "Set Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    std::string newName = node->GetInputValue<std::string>(2, "Hello");
                    obj->Name = newName;
                    node->SetOutputValue<std::string>(1, obj->Name);
                    std::cout << "GameObject renamed to: " << newName << std::endl;
                }
                else
                {
                    std::cout << "ChangeNameNode received nullptr GameObject" << std::endl;
                }
            },
            SCRIPT,                                                                  // CATEGORY
            true,                                                                    // EXECUTE PIN INPUT
            true,                                                                    // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"New Name", std::string("Hello")}}, // INPUT PINS
            {{"Name", std::string("Name")}},                                         // OUTPUT PINS
            position                                                                 // PIN POSITION
        );

        // Crear el nodo usando el engine
        PremakeNode setTagNode(
            "Object",
            "Set Tag",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    std::string newName = node->GetInputValue<std::string>(2, "Default");
                    obj->Tag = newName;
                    node->SetOutputValue<std::string>(1, obj->Tag);
                    std::cout << "GameObject renamed to: " << newName << std::endl;
                }
                else
                {
                    std::cout << "ChangeNameNode received nullptr GameObject" << std::endl;
                }
            },
            SCRIPT,                                                               // CATEGORY
            true,                                                                 // EXECUTE PIN INPUT
            true,                                                                 // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Tag", std::string("Default")}}, // INPUT PINS
            {{"New Tag", std::string("Default")}},                                // OUTPUT PINS
            position                                                              // PIN POSITION
        );

        PremakeNode getNameNode(
            "Object",
            "Get Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    std::string name = obj->Name;
                    node->SetOutputValue<std::string>(0, name);
                }
                else
                {
                    node->SetOutputValue<std::string>(0, "NULL");
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Name", std::string("")}},         // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode getTagNode(
            "Object",
            "Get Tag",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    std::string name = obj->Tag;
                    node->SetOutputValue<std::string>(0, name);
                }
                else
                {
                    node->SetOutputValue<std::string>(0, "NULL");
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Name", std::string("")}},         // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setPosNode(
            "Object",
            "Set Position",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 pos = node->GetInputValue<glm::vec3>(2, glm::vec3(0, 0, 0));

                if (obj)
                {
                    obj->setWorldPosition(pos);
                    node->SetOutputValue<glm::vec3>(1, pos);
                }
            },
            SCRIPT,                                                                // CATEGORY
            true,                                                                  // EXECUTE PIN INPUT
            true,                                                                  // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Position", glm::vec3(0, 0, 0)}}, // INPUT PINS
            {{"Position", glm::vec3(0, 0, 0)}},                                    // OUTPUT PINS
            position                                                               // PIN POSITION
        );

        PremakeNode getPosNode(
            "Object",
            "Get Position",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getWorldPosition());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Position", glm::vec3(0, 0, 0)}},  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setLocalPosNode(
            "Object",
            "Set Local Position",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 pos = node->GetInputValue<glm::vec3>(2, glm::vec3(0, 0, 0));

                if (obj)
                {
                    obj->setLocalPosition(pos);
                    node->SetOutputValue<glm::vec3>(1, pos);
                }
            },
            SCRIPT,                                                                // CATEGORY
            true,                                                                  // EXECUTE PIN INPUT
            true,                                                                  // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Position", glm::vec3(0, 0, 0)}}, // INPUT PINS
            {{"Position", glm::vec3(0, 0, 0)}},                                    // OUTPUT PINS
            position                                                               // PIN POSITION
        );

        PremakeNode getLocalPosNode(
            "Object",
            "Get Local Position",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getLocalPosition());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Position", glm::vec3(0, 0, 0)}},  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setScaleNode(
            "Object",
            "Set Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 valGet = node->GetInputValue<glm::vec3>(2, glm::vec3(0, 0, 0));

                if (obj)
                {
                    obj->setWorldScale(valGet);
                    node->SetOutputValue<glm::vec3>(1, valGet);
                }
            },
            SCRIPT,                                                             // CATEGORY
            true,                                                               // EXECUTE PIN INPUT
            true,                                                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Scale", glm::vec3(0, 0, 0)}}, // INPUT PINS
            {{"Scale", glm::vec3(0, 0, 0)}},                                    // OUTPUT PINS
            position                                                            // PIN POSITION
        );

        PremakeNode getScaleNode(
            "Object",
            "Get Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getWorldScale());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Scale", glm::vec3(0, 0, 0)}},     // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setLocalScaleNode(
            "Object",
            "Set Local Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 valGet = node->GetInputValue<glm::vec3>(2, glm::vec3(1, 1, 1));

                if (obj)
                {
                    obj->setLocalScale(valGet);
                    node->SetOutputValue<glm::vec3>(1, valGet);
                }
            },
            SCRIPT,                                                             // CATEGORY
            true,                                                               // EXECUTE PIN INPUT
            true,                                                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Scale", glm::vec3(1, 1, 1)}}, // INPUT PINS
            {{"Scale", glm::vec3(1, 1, 1)}},                                    // OUTPUT PINS
            position                                                            // PIN POSITION
        );

        PremakeNode getLocalScaleNode(
            "Object",
            "Get Local Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getLocalScale());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Scale", glm::vec3(1, 1, 1)}},     // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setRotNode(
            "Object",
            "Set Rotation",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::quat rot = node->GetInputValue<glm::quat>(2, glm::quat(1, 0, 0, 0)); // identidad

                if (obj)
                {
                    obj->setWorldRotationQuat(rot);
                    node->SetOutputValue<glm::quat>(1, rot);
                }
            },
            SCRIPT,
            true, // EXECUTE PIN INPUT
            true, // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Rotation", glm::quat(1, 0, 0, 0)}},
            {{"Rotation", glm::quat(1, 0, 0, 0)}},
            position);

        PremakeNode getRotNode(
            "Object",
            "Get Rotation",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::quat>(0, obj->getWorldRotationQuat());
                }
            },
            SCRIPT,
            false, // no EXECUTE PIN
            false,
            {{"Object", (GameObject *)nullptr}},
            {{"Rotation", glm::quat(1, 0, 0, 0)}},
            position);

        PremakeNode setLocalRotNode(
            "Object",
            "Set Local Rotation",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::quat rot = node->GetInputValue<glm::quat>(2, glm::quat(1, 0, 0, 0));

                if (obj)
                {
                    obj->setLocalRotationQuat(rot);
                    node->SetOutputValue<glm::quat>(1, rot);
                }
            },
            SCRIPT,
            true,
            true,
            {{"Object", (GameObject *)nullptr}, {"Rotation", glm::quat(1, 0, 0, 0)}},
            {{"Rotation", glm::quat(1, 0, 0, 0)}},
            position);

        PremakeNode getLocalRotNode(
            "Object",
            "Get Local Rotation",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::quat>(0, obj->getLocalRotationQuat());
                }
            },
            SCRIPT,
            false,
            false,
            {{"Object", (GameObject *)nullptr}},
            {{"Rotation", glm::quat(1, 0, 0, 0)}},
            position);

        PremakeNode setEulerNode(
            "Object",
            "Set Rotation Euler",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 euler = node->GetInputValue<glm::vec3>(2, glm::vec3(0, 0, 0));

                if (obj)
                {
                    obj->setWorldRotationEuler(euler); // tu GameObject debería aceptar Euler
                    node->SetOutputValue<glm::vec3>(1, euler);
                }
            },
            SCRIPT,
            true,
            true,
            {{"Object", (GameObject *)nullptr}, {"Rotation", glm::vec3(0, 0, 0)}},
            {{"Rotation", glm::vec3(0, 0, 0)}},
            position);

        PremakeNode getEulerNode(
            "Object",
            "Get Rotation Euler",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getLocalRotationEuler());
                }
            },
            SCRIPT,
            false,
            false,
            {{"Object", (GameObject *)nullptr}},
            {{"Rotation", glm::vec3(0, 0, 0)}},
            position);

        PremakeNode setLocalEulerNode(
            "Object",
            "Set Local Rotation Euler",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                glm::vec3 euler = node->GetInputValue<glm::vec3>(2, glm::vec3(0, 0, 0));

                if (obj)
                {
                    obj->setLocalRotationEuler(euler);
                    node->SetOutputValue<glm::vec3>(1, euler);
                }
            },
            SCRIPT,
            true,
            true,
            {{"Object", (GameObject *)nullptr}, {"Rotation", glm::vec3(0, 0, 0)}},
            {{"Rotation", glm::vec3(0, 0, 0)}},
            position);

        PremakeNode getLocalEulerNode(
            "Object",
            "Get Local Rotation Euler",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                if (obj == nullptr)
                    obj = node->_SelfObject;

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(0, obj->getLocalRotationEuler());
                }
            },
            SCRIPT,
            false,
            false,
            {{"Object", (GameObject *)nullptr}},
            {{"Rotation", glm::vec3(0, 0, 0)}},
            position);

        PremakeNode findObjectNode(
            "Object",
            "Find Object Per Index",
            [](CustomNode *node)
            {
                int index = node->GetInputValue<int>(1, 0);

                auto &gameObjects = SceneManager::getInstance().getActiveScene()->getGameObjects();

                if (index >= 0 && index < static_cast<int>(gameObjects.size()))
                {
                    GameObject *obj = gameObjects[index];

                    node->SetOutputValue<GameObject *>(1, obj);
                }
                else
                {
                    std::cout << "Invalid GameObject index: " << index << " (max: " << gameObjects.size() - 1 << ")" << std::endl;
                    node->SetOutputValue<GameObject *>(1, nullptr);
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Index", 0}},                      // INPUT PINS
            {{"Object", (GameObject *)nullptr}}, // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode thisObject(
            "Object",
            "This Object",
            [](CustomNode *node)
            {
                node->SetOutputValue<GameObject *>(0, node->_SelfObject);
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {},                                  // INPUT PINS
            {{"Object", (GameObject *)nullptr}}, // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode findObjectByNameNode(
            "Object",
            "Find Object By Name",
            [](CustomNode *node)
            {
                std::string targetName = node->GetInputValue<std::string>(1, "");

                auto &gameObjects = SceneManager::getInstance().getActiveScene()->getGameObjects();
                GameObject *foundObj = nullptr;

                for (auto *obj : gameObjects)
                {
                    if (obj && obj->Name == targetName)
                    {
                        foundObj = obj;
                        break; // encontramos el primero que coincide
                    }
                }

                if (!foundObj)
                {
                    std::cout << "GameObject with name \"" << targetName << "\" not found!" << std::endl;
                }

                node->SetOutputValue<GameObject *>(1, foundObj);
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Name", std::string("")}},         // INPUT PINS
            {{"Object", (GameObject *)nullptr}}, // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setParentNode(
            "Object",
            "Attach To",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(2, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                if (object != nullptr)
                {
                    if (node->GetInputValue<GameObject *>(1, nullptr) != nullptr)
                    {
                        object->setParent(node->GetInputValue<GameObject *>(1, nullptr));
                        node->SetOutputValue<GameObject *>(1, object->getParent());
                    }
                }
            },
            SCRIPT,                                                                     // CATEGORY
            true,                                                                       // EXECUTE PIN INPUT
            true,                                                                       // EXECUTE PIN OUT
            {{"New Parent", (GameObject *)nullptr}, {"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Parent", (GameObject *)nullptr}},                                        // OUTPUT PINS
            position                                                                    // PIN POSITION
        );

        PremakeNode removeParent(
            "Object",
            "Detach",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(1, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                if (object != nullptr)
                {
                    object->setParent(nullptr);
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {},                                  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode countChilds(
            "Object",
            "Count Childs",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(0, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                if (object != nullptr)
                {
                    node->SetOutputValue<int>(0, object->getChildCount());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Childs", 0}},                     // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode getChild(
            "Object",
            "Get Child",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(1, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                int index = node->GetInputValue<int>(2, 0);

                if (object != nullptr)
                {
                    node->SetOutputValue<GameObject *>(1, object->getChild(index));
                }
            },
            SCRIPT,                                                  // CATEGORY
            true,                                                    // EXECUTE PIN INPUT
            true,                                                    // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Child Index", 0}}, // INPUT PINS
            {{"Child", (GameObject *)nullptr}},                      // OUTPUT PINS
            position                                                 // PIN POSITION
        );

        PremakeNode getChildByName(
            "Object",
            "Get Child By Name",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(1, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                std::string childName = node->GetInputValue<std::string>(2, "");

                GameObject *foundChild = nullptr;

                if (object != nullptr)
                {
                    int childCount = object->getChildCount(); // Asumo que tenés una función así
                    for (int i = 0; i < childCount; ++i)
                    {
                        GameObject *child = object->getChild(i);
                        if (child && child->Name == childName)
                        {
                            foundChild = child;
                            break; // devuelve el primero que coincide
                        }
                    }
                }

                node->SetOutputValue<GameObject *>(1, foundChild);
            },
            SCRIPT,                                                               // CATEGORY
            true,                                                                 // EXECUTE PIN INPUT
            true,                                                                 // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Child Name", std::string("")}}, // INPUT PINS
            {{"Child", (GameObject *)nullptr}},                                   // OUTPUT PINS
            position                                                              // PIN POSITION
        );

        PremakeNode destroyNode(
            "Object",
            "Destroy Object",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(1, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                if (object != nullptr)
                {
                    object->destroy();
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {},                                  // OUTPUT PINS
            position                             // PIN POSITION
        );

        engine.PrefabNodes.push_back(thisObject);
        engine.PrefabNodes.push_back(findObjectByNameNode);
        engine.PrefabNodes.push_back(findObjectNode);

        engine.PrefabNodes.push_back(setParentNode);
        engine.PrefabNodes.push_back(removeParent);

        engine.PrefabNodes.push_back(countChilds);
        engine.PrefabNodes.push_back(getChild);
        engine.PrefabNodes.push_back(getChildByName);

        engine.PrefabNodes.push_back(destroyNode);

        engine.PrefabNodes.push_back(getNameNode);
        engine.PrefabNodes.push_back(setNameNode);

        engine.PrefabNodes.push_back(setTagNode);
        engine.PrefabNodes.push_back(getTagNode);

        engine.PrefabNodes.push_back(setPosNode);
        engine.PrefabNodes.push_back(getPosNode);

        engine.PrefabNodes.push_back(setLocalPosNode);
        engine.PrefabNodes.push_back(getLocalPosNode);

        engine.PrefabNodes.push_back(setScaleNode);
        engine.PrefabNodes.push_back(getScaleNode);

        engine.PrefabNodes.push_back(setLocalScaleNode);
        engine.PrefabNodes.push_back(getLocalScaleNode);

        engine.PrefabNodes.push_back(setRotNode);
        engine.PrefabNodes.push_back(getRotNode);

        engine.PrefabNodes.push_back(setLocalRotNode);
        engine.PrefabNodes.push_back(getLocalRotNode);

        engine.PrefabNodes.push_back(setEulerNode);
        engine.PrefabNodes.push_back(getEulerNode);

        engine.PrefabNodes.push_back(setLocalEulerNode);
        engine.PrefabNodes.push_back(getLocalEulerNode);
    }
};