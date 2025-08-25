#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "MNodeEngine.h"
#include <imgui/imgui.h>

class ConvertsNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // ----------- STRING TO INT -----------
        PremakeNode stringToIntNode(
            "Convert", "String To Int",
            [](CustomNode *node)
            {
                std::string input = node->GetInputValue<std::string>(0, "");
                int result = 0;
                try
                {
                    result = std::stoi(input);
                }
                catch (...)
                {
                    result = 0;
                }
                node->SetOutputValue<int>(0, result);
            },
            SCRIPT, false, false,
            {{"String", std::string("")}},
            {{"Int", 0}},
            position);

        // ----------- STRING TO FLOAT -----------
        PremakeNode stringToFloatNode(
            "Convert", "String To Float",
            [](CustomNode *node)
            {
                std::string input = node->GetInputValue<std::string>(0, "");
                float result = 0.0f;
                try
                {
                    result = std::stof(input);
                }
                catch (...)
                {
                    result = 0.0f;
                }
                node->SetOutputValue<float>(0, result);
            },
            SCRIPT, false, false,
            {{"String", std::string("")}},
            {{"Float", 0.0f}},
            position);

        // ----------- INT TO STRING -----------
        PremakeNode intToStringNode(
            "Convert", "Int To String",
            [](CustomNode *node)
            {
                int input = node->GetInputValue<int>(0, 0);
                node->SetOutputValue<std::string>(0, std::to_string(input));
            },
            SCRIPT, false, false,
            {{"Int", 0}},
            {{"String", std::string("")}},
            position);

        // ----------- FLOAT TO STRING -----------
        PremakeNode floatToStringNode(
            "Convert", "Float To String",
            [](CustomNode *node)
            {
                float input = node->GetInputValue<float>(0, 0.0f);
                node->SetOutputValue<std::string>(0, std::to_string(input));
            },
            SCRIPT, false, false,
            {{"Float", 0.0f}},
            {{"String", std::string("")}},
            position);

        // ----------- FLOAT TO INT -----------
        PremakeNode floatToIntNode(
            "Convert", "Float To Int",
            [](CustomNode *node)
            {
                float input = node->GetInputValue<float>(0, 0.0f);
                node->SetOutputValue<int>(0, static_cast<int>(input));
            },
            SCRIPT, false, false,
            {{"Float", 0.0f}},
            {{"Int", 0}},
            position);

        // ----------- INT TO FLOAT -----------
        PremakeNode intToFloatNode(
            "Convert", "Int To Float",
            [](CustomNode *node)
            {
                int input = node->GetInputValue<int>(0, 0);
                node->SetOutputValue<float>(0, static_cast<float>(input));
            },
            SCRIPT, false, false,
            {{"Int", 0}},
            {{"Float", 0.0f}},
            position);

        // ----------- STRING TO BOOL -----------
        PremakeNode stringToBoolNode(
            "Convert", "String To Bool",
            [](CustomNode *node)
            {
                std::string input = node->GetInputValue<std::string>(0, "");
                bool result = (input == "true" || input == "1");
                node->SetOutputValue<bool>(0, result);
            },
            SCRIPT, false, false,
            {{"String", std::string("")}},
            {{"Bool", false}},
            position);

        // ----------- BOOL TO STRING -----------
        PremakeNode boolToStringNode(
            "Convert", "Bool To String",
            [](CustomNode *node)
            {
                bool input = node->GetInputValue<bool>(0, false);
                node->SetOutputValue<std::string>(0, input ? "true" : "false");
            },
            SCRIPT, false, false,
            {{"Bool", false}},
            {{"String", std::string("")}},
            position);

        // ----------- INT TO BOOL -----------
        PremakeNode intToBoolNode(
            "Convert", "Int To Bool",
            [](CustomNode *node)
            {
                int input = node->GetInputValue<int>(0, 0);
                node->SetOutputValue<bool>(0, input != 0);
            },
            SCRIPT, false, false,
            {{"Int", 0}},
            {{"Bool", false}},
            position);

        // ----------- BOOL TO INT -----------
        PremakeNode boolToIntNode(
            "Convert", "Bool To Int",
            [](CustomNode *node)
            {
                bool input = node->GetInputValue<bool>(0, false);
                node->SetOutputValue<int>(0, input ? 1 : 0);
            },
            SCRIPT, false, false,
            {{"Bool", false}},
            {{"Int", 0}},
            position);

        // ----------- FLOAT TO BOOL -----------
        PremakeNode floatToBoolNode(
            "Convert", "Float To Bool",
            [](CustomNode *node)
            {
                float input = node->GetInputValue<float>(0, 0.0f);
                node->SetOutputValue<bool>(0, input != 0.0f);
            },
            SCRIPT, false, false,
            {{"Float", 0.0f}},
            {{"Bool", false}},
            position);

        // ----------- BOOL TO FLOAT -----------
        PremakeNode boolToFloatNode(
            "Convert", "Bool To Float",
            [](CustomNode *node)
            {
                bool input = node->GetInputValue<bool>(0, false);
                node->SetOutputValue<float>(0, input ? 1.0f : 0.0f);
            },
            SCRIPT, false, false,
            {{"Bool", false}},
            {{"Float", 0.0f}},
            position);

        // Agregar todos los nodos al engine
        engine.PrefabNodes.push_back(stringToIntNode);
        engine.PrefabNodes.push_back(stringToFloatNode);
        engine.PrefabNodes.push_back(intToStringNode);
        engine.PrefabNodes.push_back(intToFloatNode);
        engine.PrefabNodes.push_back(floatToStringNode);
        engine.PrefabNodes.push_back(floatToIntNode);

        engine.PrefabNodes.push_back(stringToBoolNode);
        engine.PrefabNodes.push_back(boolToStringNode);
        engine.PrefabNodes.push_back(intToBoolNode);
        engine.PrefabNodes.push_back(boolToIntNode);
        engine.PrefabNodes.push_back(floatToBoolNode);
        engine.PrefabNodes.push_back(boolToFloatNode);
    }
};