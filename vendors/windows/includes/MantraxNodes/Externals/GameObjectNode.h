#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include <MantraxNodes/MNodeEngine.h>
#include <imgui/imgui.h>

class GameObjectNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode findObjectNode(
            "Object",
            "Find Object",
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

        // Crear el nodo usando el engine
        PremakeNode setNameNode(
            "Object",
            "Set Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);
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
            SCRIPT,                                                                   // CATEGORY
            true,                                                                     // EXECUTE PIN INPUT
            true,                                                                     // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"New Tag", std::string("Default")}}, // INPUT PINS
            {{"New Tag", std::string("Default")}},                                    // OUTPUT PINS
            position                                                                  // PIN POSITION
        );

        PremakeNode getNameNode(
            "Object",
            "Get Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);
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

        PremakeNode setScaleNode(
            "Object",
            "Set Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);
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

        PremakeNode getPosNode(
            "Object",
            "Get Position",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(1, obj->getWorldPosition());
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Position", glm::vec3(0, 0, 0)}},  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode getScaleNode(
            "Object",
            "Get Scale",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);

                if (obj)
                {
                    node->SetOutputValue<glm::vec3>(1, obj->getWorldScale());
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Scale", glm::vec3(0, 0, 0)}},     // OUTPUT PINS
            position                             // PIN POSITION
        );

        engine.PrefabNodes.push_back(getNameNode);
        engine.PrefabNodes.push_back(getTagNode);
        engine.PrefabNodes.push_back(setNameNode);
        engine.PrefabNodes.push_back(setTagNode);

        engine.PrefabNodes.push_back(findObjectNode);
        engine.PrefabNodes.push_back(setPosNode);
        engine.PrefabNodes.push_back(setScaleNode);
        engine.PrefabNodes.push_back(getPosNode);
        engine.PrefabNodes.push_back(getScaleNode);
    }
};
