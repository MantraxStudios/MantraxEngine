#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "MNodeEngine.h"

class MathNodes
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // ----------- INT -----------
        PremakeNode addIntNode(
            "Math", "Add Int",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(0, 0);
                int b = node->GetInputValue<int>(1, 0);
                node->SetOutputValue<int>(0, a + b);
            },
            SCRIPT, false, false,
            {{"A", 0}, {"B", 0}},
            {{"Result", 0}},
            position);

        PremakeNode subIntNode(
            "Math", "Subtract Int",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(0, 0);
                int b = node->GetInputValue<int>(1, 0);
                node->SetOutputValue<int>(0, a - b);
            },
            SCRIPT, false, false,
            {{"A", 0}, {"B", 0}},
            {{"Result", 0}},
            position);

        PremakeNode mulIntNode(
            "Math", "Multiply Int",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(0, 0);
                int b = node->GetInputValue<int>(1, 0);
                node->SetOutputValue<int>(0, a * b);
            },
            SCRIPT, false, false,
            {{"A", 0}, {"B", 0}},
            {{"Result", 0}},
            position);

        PremakeNode divIntNode(
            "Math", "Divide Int",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(0, 0);
                int b = node->GetInputValue<int>(1, 1);
                if (b != 0)
                    node->SetOutputValue<int>(0, a / b);
                else
                    node->SetOutputValue<int>(0, 0);
            },
            SCRIPT, false, false,
            {{"A", 0}, {"B", 1}},
            {{"Result", 0}},
            position);

        // ----------- FLOAT -----------
        PremakeNode addFloatNode(
            "Math", "Add Float",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 0.0f);
                node->SetOutputValue<float>(0, a + b);
            },
            SCRIPT, false, false,
            {{"A", 0.0f}, {"B", 0.0f}},
            {{"Result", 0.0f}},
            position);

        PremakeNode subFloatNode(
            "Math", "Subtract Float",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 0.0f);
                node->SetOutputValue<float>(0, a - b);
            },
            SCRIPT, false, false,
            {{"A", 0.0f}, {"B", 0.0f}},
            {{"Result", 0.0f}},
            position);

        PremakeNode mulFloatNode(
            "Math", "Multiply Float",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 0.0f);
                node->SetOutputValue<float>(0, a * b);
            },
            SCRIPT, false, false,
            {{"A", 0.0f}, {"B", 0.0f}},
            {{"Result", 0.0f}},
            position);

        PremakeNode divFloatNode(
            "Math", "Divide Float",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 1.0f);
                if (b != 0.0f)
                    node->SetOutputValue<float>(0, a / b);
                else
                    node->SetOutputValue<float>(0, 0.0f);
            },
            SCRIPT, false, false,
            {{"A", 0.0f}, {"B", 1.0f}},
            {{"Result", 0.0f}},
            position);

        // ----------- VEC2 -----------
        PremakeNode addVec2Node(
            "Math", "Add Vec2",
            [](CustomNode *node)
            {
                glm::vec2 a = node->GetInputValue<glm::vec2>(0, glm::vec2(0.0f, 0.0f));
                glm::vec2 b = node->GetInputValue<glm::vec2>(1, glm::vec2(0.0f, 0.0f));
                node->SetOutputValue<glm::vec2>(0, a + b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec2(0.0f, 0.0f)}, {"B", glm::vec2(0.0f, 0.0f)}},
            {{"Result", glm::vec2(0.0f, 0.0f)}},
            position);

        PremakeNode subVec2Node(
            "Math", "Subtract Vec2",
            [](CustomNode *node)
            {
                glm::vec2 a = node->GetInputValue<glm::vec2>(0, glm::vec2(0.0f, 0.0f));
                glm::vec2 b = node->GetInputValue<glm::vec2>(1, glm::vec2(0.0f, 0.0f));
                node->SetOutputValue<glm::vec2>(0, a - b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec2(0.0f, 0.0f)}, {"B", glm::vec2(0.0f, 0.0f)}},
            {{"Result", glm::vec2(0.0f, 0.0f)}},
            position);

        PremakeNode mulVec2Node(
            "Math", "Multiply Vec2",
            [](CustomNode *node)
            {
                glm::vec2 a = node->GetInputValue<glm::vec2>(0, glm::vec2(1.0f, 1.0f));
                glm::vec2 b = node->GetInputValue<glm::vec2>(1, glm::vec2(1.0f, 1.0f));
                node->SetOutputValue<glm::vec2>(0, a * b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec2(1.0f, 1.0f)}, {"B", glm::vec2(1.0f, 1.0f)}},
            {{"Result", glm::vec2(1.0f, 1.0f)}},
            position);

        PremakeNode divVec2Node(
            "Math", "Divide Vec2",
            [](CustomNode *node)
            {
                glm::vec2 a = node->GetInputValue<glm::vec2>(0, glm::vec2(1.0f, 1.0f));
                glm::vec2 b = node->GetInputValue<glm::vec2>(1, glm::vec2(1.0f, 1.0f));
                glm::vec2 res(0.0f, 0.0f);
                if (b.x != 0.0f)
                    res.x = a.x / b.x;
                if (b.y != 0.0f)
                    res.y = a.y / b.y;
                node->SetOutputValue<glm::vec2>(0, res);
            },
            SCRIPT, false, false,
            {{"A", glm::vec2(1.0f, 1.0f)}, {"B", glm::vec2(1.0f, 1.0f)}},
            {{"Result", glm::vec2(1.0f, 1.0f)}},
            position);

        // ----------- VEC3 -----------
        PremakeNode addVec3Node(
            "Math", "Add Vec3",
            [](CustomNode *node)
            {
                glm::vec3 a = node->GetInputValue<glm::vec3>(0, glm::vec3(0.0f, 0.0f, 0.0f));
                glm::vec3 b = node->GetInputValue<glm::vec3>(1, glm::vec3(0.0f, 0.0f, 0.0f));
                node->SetOutputValue<glm::vec3>(0, a + b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec3(0.0f, 0.0f, 0.0f)}, {"B", glm::vec3(0.0f, 0.0f, 0.0f)}},
            {{"Result", glm::vec3(0.0f, 0.0f, 0.0f)}},
            position);

        PremakeNode subVec3Node(
            "Math", "Subtract Vec3",
            [](CustomNode *node)
            {
                glm::vec3 a = node->GetInputValue<glm::vec3>(0, glm::vec3(0.0f, 0.0f, 0.0f));
                glm::vec3 b = node->GetInputValue<glm::vec3>(1, glm::vec3(0.0f, 0.0f, 0.0f));
                node->SetOutputValue<glm::vec3>(0, a - b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec3(0.0f, 0.0f, 0.0f)}, {"B", glm::vec3(0.0f, 0.0f, 0.0f)}},
            {{"Result", glm::vec3(0.0f, 0.0f, 0.0f)}},
            position);

        PremakeNode mulVec3Node(
            "Math", "Multiply Vec3",
            [](CustomNode *node)
            {
                glm::vec3 a = node->GetInputValue<glm::vec3>(0, glm::vec3(1.0f, 1.0f, 1.0f));
                glm::vec3 b = node->GetInputValue<glm::vec3>(1, glm::vec3(1.0f, 1.0f, 1.0f));
                node->SetOutputValue<glm::vec3>(0, a * b);
            },
            SCRIPT, false, false,
            {{"A", glm::vec3(1.0f, 1.0f, 1.0f)}, {"B", glm::vec3(1.0f, 1.0f, 1.0f)}},
            {{"Result", glm::vec3(1.0f, 1.0f, 1.0f)}},
            position);

        PremakeNode divVec3Node(
            "Math", "Divide Vec3",
            [](CustomNode *node)
            {
                glm::vec3 a = node->GetInputValue<glm::vec3>(0, glm::vec3(1.0f, 1.0f, 1.0f));
                glm::vec3 b = node->GetInputValue<glm::vec3>(1, glm::vec3(1.0f, 1.0f, 1.0f));
                glm::vec3 res(0.0f, 0.0f, 0.0f);
                if (b.x != 0.0f)
                    res.x = a.x / b.x;
                if (b.y != 0.0f)
                    res.y = a.y / b.y;
                if (b.z != 0.0f)
                    res.z = a.z / b.z;
                node->SetOutputValue<glm::vec3>(0, res);
            },
            SCRIPT, false, false,
            {{"A", glm::vec3(1.0f, 1.0f, 1.0f)}, {"B", glm::vec3(1.0f, 1.0f, 1.0f)}},
            {{"Result", glm::vec3(1.0f, 1.0f, 1.0f)}},
            position);

        // Agregar todos los nodos al engine
        engine.PrefabNodes.push_back(addIntNode);
        engine.PrefabNodes.push_back(subIntNode);
        engine.PrefabNodes.push_back(mulIntNode);
        engine.PrefabNodes.push_back(divIntNode);

        engine.PrefabNodes.push_back(addFloatNode);
        engine.PrefabNodes.push_back(subFloatNode);
        engine.PrefabNodes.push_back(mulFloatNode);
        engine.PrefabNodes.push_back(divFloatNode);

        engine.PrefabNodes.push_back(addVec2Node);
        engine.PrefabNodes.push_back(subVec2Node);
        engine.PrefabNodes.push_back(mulVec2Node);
        engine.PrefabNodes.push_back(divVec2Node);

        engine.PrefabNodes.push_back(addVec3Node);
        engine.PrefabNodes.push_back(subVec3Node);
        engine.PrefabNodes.push_back(mulVec3Node);
        engine.PrefabNodes.push_back(divVec3Node);
    }
};