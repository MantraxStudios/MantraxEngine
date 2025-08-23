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
        // -------- BRANCH NODE --------
        PremakeNode branchNode(
            "Condition", "Branch",
            [](CustomNode *node) {},
            SCRIPT, true, true, // Branch sí tiene ejecución
            {{"Value", false}}, // condición
            {},                 // outputs exec se manejan en ExecuteFrom
            position);

        PremakeNode forNode(
            "Condition", "For Loop",
            [](CustomNode *node) {

            },
            SCRIPT, true, true,          // Branch sí tiene ejecución
            {{"Start", 0}, {"End", 10}}, // condición
            {{"Index", 0}},              // outputs exec se manejan en ExecuteFrom
            position);

        // -------- INT COMPARISON NODE --------
        PremakeNode intCompareNode(
            "Condition", "Int Compare",
            [](CustomNode *node)
            {
                int a = node->GetInputValue<int>(0, 0);
                int b = node->GetInputValue<int>(1, 0);
                std::string op = node->GetInputValue<std::string>(2, "==");
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
            SCRIPT, false, false, // NO exec
            {{"A", 0}, {"B", 0}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- FLOAT COMPARISON NODE --------
        PremakeNode floatCompareNode(
            "Condition", "Float Compare",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 0.0f);
                std::string op = node->GetInputValue<std::string>(2, "==");
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
            SCRIPT, false, false, // NO exec
            {{"A", 0.0f}, {"B", 0.0f}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- STRING COMPARISON NODE --------
        PremakeNode stringCompareNode(
            "Condition", "String Compare",
            [](CustomNode *node)
            {
                std::string a = node->GetInputValue<std::string>(0, "");
                std::string b = node->GetInputValue<std::string>(1, "");
                std::string op = node->GetInputValue<std::string>(2, "==");
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
            SCRIPT, false, false, // NO exec
            {{"A", std::string("")}, {"B", std::string("")}, {"Operator", std::string("==")}},
            {{"Result", false}},
            position);

        // -------- BOOL LOGIC NODE --------
        PremakeNode boolLogicNode(
            "Condition", "Bool Logic",
            [](CustomNode *node)
            {
                bool a = node->GetInputValue<bool>(0, false);
                bool b = node->GetInputValue<bool>(1, false);
                std::string op = node->GetInputValue<std::string>(2, "AND");
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
            SCRIPT, false, false, // NO exec
            {{"A", false}, {"B", false}, {"Operator", std::string("AND")}},
            {{"Result", false}},
            position);

        // -------- REGISTRO --------
        engine.PrefabNodes.push_back(forNode);
        engine.PrefabNodes.push_back(branchNode);
        engine.PrefabNodes.push_back(intCompareNode);
        engine.PrefabNodes.push_back(floatCompareNode);
        engine.PrefabNodes.push_back(stringCompareNode);
        engine.PrefabNodes.push_back(boolLogicNode);
    }
};
