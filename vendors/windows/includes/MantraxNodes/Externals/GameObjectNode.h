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
    // Método para crear el nodo Find Object
    void FindObject(MNodeEngine &engine)
    {
        // Crear el nodo usando el engine
        CustomNode *findObjectNode = engine.CreateNode(
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
            ImVec2(100, 100)                     // PIN POSITION
        );
    }

    // Método para crear el nodo Change Name
    void ChangeNameNode(MNodeEngine &engine)
    {
        // Crear el nodo usando el engine
        CustomNode *changeNameNode = engine.CreateNode(
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
            ImVec2(300, 100)                                                         // PIN POSITION
        );
    }

    // Método para crear el nodo Get GameObject Name
    void GetNameNode(MNodeEngine &engine)
    {
        // Crear el nodo usando el engine
        CustomNode *getNameNode = engine.CreateNode(
            "Get GameObject Name",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);
                if (obj)
                {
                    std::string name = obj->Name;
                    node->SetOutputValue<std::string>(0, name);
                    std::cout << "✅ GameObject name: " << name << std::endl;
                }
                else
                {
                    std::cout << "⚠️ GetNameNode received nullptr GameObject" << std::endl;
                    node->SetOutputValue<std::string>(0, "NULL");
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Name", std::string("")}},         // OUTPUT PINS
            ImVec2(500, 100)                     // PIN POSITION
        );
    }

    // Método para crear el nodo Move GameObject
    void MoveGameObjectNode(MNodeEngine &engine)
    {
        // Crear el nodo usando el engine
        CustomNode *moveNode = engine.CreateNode(
            "Move GameObject",
            [](CustomNode *node)
            {
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);
                float deltaX = node->GetInputValue<float>(1, 0.0f);
                float deltaY = node->GetInputValue<float>(2, 0.0f);
                float deltaZ = node->GetInputValue<float>(3, 0.0f);

                if (obj)
                {
                    // Aquí podrías implementar la lógica de movimiento real
                    // Por ahora solo mostramos la información
                    std::cout << "✅ Moving GameObject: " << obj->Name
                              << " by (" << deltaX << ", " << deltaY << ", " << deltaZ << ")" << std::endl;

                    // Establecer la nueva posición como salida
                    node->SetOutputValue<Vector3>(0, Vector3(deltaX, deltaY, deltaZ));
                }
                else
                {
                    std::cout << "⚠️ MoveGameObjectNode received nullptr GameObject" << std::endl;
                }
            },
            SCRIPT,                                                                                       // CATEGORY
            true,                                                                                         // EXECUTE PIN INPUT
            true,                                                                                         // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}, {"Delta X", 0.0f}, {"Delta Y", 0.0f}, {"Delta Z", 0.0f}}, // INPUT PINS
            {{"New Position", Vector3(0, 0, 0)}},                                                         // OUTPUT PINS
            ImVec2(700, 100)                                                                              // PIN POSITION
        );
    }

    void CreateAllGameObjectNodes(MNodeEngine &engine)
    {
        std::cout << "🔧 Creating all GameObject nodes..." << std::endl;

        FindObject(engine);
        ChangeNameNode(engine);
        GetNameNode(engine);
        MoveGameObjectNode(engine);
    }
};
