#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <MantraxNodes/MNodeEngine.h>
#include <imgui/imgui.h>

class ConditionNodes
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // -------- INT COMPARISON NODES --------
        PremakeNode intCompareNode(
            "Condition", "Int Compare",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(1, 0);
                int b = node->GetInputValue<int>(2, 0);
                std::string op = node->GetInputValue<std::string>(3, "==");
                bool result = false;

                if (op == "==")
                    result = (a == b);
                else if (op == "!=")
                    result = (a != b);
                else if (op == "<")
                    result = (a < b);
                else if (op == "<=")
                    result = (a <= b);
                else if (op == ">")
                    result = (a > b);
                else if (op == ">=")
                    result = (a >= b);

                node->SetOutputValue<bool>(1, result);
            },
            SCRIPT, true, true,
            {{"A", 0}, {"B", 0}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- FLOAT COMPARISON NODES --------
        PremakeNode floatCompareNode(
            "Condition", "Float Compare",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(1, 0.0f);
                float b = node->GetInputValue<float>(2, 0.0f);
                std::string op = node->GetInputValue<std::string>(3, "==");
                bool result = false;

                if (op == "==")
                    result = (a == b);
                else if (op == "!=")
                    result = (a != b);
                else if (op == "<")
                    result = (a < b);
                else if (op == "<=")
                    result = (a <= b);
                else if (op == ">")
                    result = (a > b);
                else if (op == ">=")
                    result = (a >= b);

                node->SetOutputValue<bool>(1, result);
            },
            SCRIPT, true, true,
            {{"A", 0.0f}, {"B", 0.0f}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- STRING COMPARISON NODES --------
        PremakeNode stringCompareNode(
            "Condition", "String Compare",
            [](CustomNode *node)
            {
                std::string a = node->GetInputValue<std::string>(1, "");
                std::string b = node->GetInputValue<std::string>(2, "");
                std::string op = node->GetInputValue<std::string>(3, "==");
                bool result = false;

                if (op == "==")
                    result = (a == b);
                else if (op == "!=")
                    result = (a != b);
                else if (op == "<")
                    result = (a < b); // lexicographical
                else if (op == "<=")
                    result = (a <= b);
                else if (op == ">")
                    result = (a > b);
                else if (op == ">=")
                    result = (a >= b);

                node->SetOutputValue<bool>(1, result);
            },
            SCRIPT, true, true,
            {{"A", std::string("")}, {"B", std::string("")}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- BOOL LOGIC NODES --------
        PremakeNode boolLogicNode(
            "Condition", "Bool Logic",
            [](CustomNode *node)
            {
                bool a = node->GetInputValue<bool>(1, false);
                bool b = node->GetInputValue<bool>(2, false);
                std::string op = node->GetInputValue<std::string>(3, "AND");
                bool result = false;

                if (op == "AND")
                    result = a && b;
                else if (op == "OR")
                    result = a || b;
                else if (op == "XOR")
                    result = a != b;
                else if (op == "NAND")
                    result = !(a && b);
                else if (op == "NOR")
                    result = !(a || b);

                node->SetOutputValue<bool>(1, result);
            },
            SCRIPT, true, true,
            {{"A", false}, {"B", false}, {"Operator", std::string("AND")}},
            {{"Result", false}},
            position);

        // Agregar todos los nodos al engine
        engine.PrefabNodes.push_back(intCompareNode);
        engine.PrefabNodes.push_back(floatCompareNode);
        engine.PrefabNodes.push_back(stringCompareNode);
        engine.PrefabNodes.push_back(boolLogicNode);
    }
};
