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
            "GameObject",
            "Find Object",
            [](CustomNode *node)
            {
                int index = node->GetInputValue<int>(0, 0);

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
        PremakeNode changeNameNode(
            "GameObject",
            "Change Name Game Object",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);
                if (obj)
                {
                    std::string newName = node->GetInputValue<std::string>(2, "Hello");
                    obj->Name = newName;
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
            {},                                                                      // OUTPUT PINS
            position                                                                 // PIN POSITION
        );

        PremakeNode getNameNode(
            "GameObject",
            "Get GameObject Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(1, nullptr);
                if (obj)
                {
                    std::string name = obj->Name;
                    node->SetOutputValue<std::string>(1, name);
                    std::cout << "GameObject name: " << name << std::endl;
                }
                else
                {
                    std::cout << "GetNameNode received nullptr GameObject" << std::endl;
                    node->SetOutputValue<std::string>(1, "NULL");
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Name", std::string("")}},         // OUTPUT PINS
            position                             // PIN POSITION
        );

        engine.PrefabNodes.push_back(getNameNode);
        engine.PrefabNodes.push_back(changeNameNode);
        engine.PrefabNodes.push_back(findObjectNode);
    }
};
