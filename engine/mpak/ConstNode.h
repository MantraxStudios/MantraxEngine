#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include "MNodeEngine.h"
#include <imgui/imgui.h>

class ConstNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode vec3Node(
            "Const",
            "Vector 3",
            [](CustomNode *node)
            {
                node->SetOutputValue<glm::vec3>(0, glm::vec3(
                                                       node->GetInputValue<float>(0, 0),
                                                       node->GetInputValue<float>(1, 0),
                                                       node->GetInputValue<float>(2, 0)));
            },
            SCRIPT, // CATEGORY
            false,  // EXECUTE PIN INPUT
            false,  // EXECUTE PIN OUT
            {
                {"X", float(0)},
                {"Y", float(0)},
                {"Z", float(0)},
            },                               // INPUT PINS
            {{"Value", glm::vec3(0, 0, 0)}}, // OUTPUT PINS
            position                         // PIN POSITION
        );

        PremakeNode vec2Node(
            "Const",
            "Vector 2",
            [](CustomNode *node)
            {
                node->SetOutputValue<glm::vec2>(0, glm::vec2(
                                                       node->GetInputValue<float>(0, 0),
                                                       node->GetInputValue<float>(1, 0)));
            },
            SCRIPT, // CATEGORY
            false,  // EXECUTE PIN INPUT
            false,  // EXECUTE PIN OUT
            {
                {"X", float(0)},
                {"Y", float(0)},
            },                            // INPUT PINS
            {{"Value", glm::vec2(0, 0)}}, // OUTPUT PINS
            position                      // PIN POSITION
        );

        PremakeNode stringNode(
            "Const",
            "String",
            [](CustomNode *node)
            {
                node->SetOutputValue<std::string>(0, node->GetInputValue<std::string>(0, "Hello"));
            },
            SCRIPT,                           // CATEGORY
            false,                            // EXECUTE PIN INPUT
            false,                            // EXECUTE PIN OUT
            {{"Text", std::string("Hello")}}, // INPUT PINS
            {{"", std::string("Hello")}},     // OUTPUT PINS
            position                          // PIN POSITION
        );

        PremakeNode intNode(
            "Const",
            "Int",
            [](CustomNode *node)
            {
                node->SetOutputValue<int>(0, node->GetInputValue<int>(0, 0));
            },
            SCRIPT,         // CATEGORY
            false,          // EXECUTE PIN INPUT
            false,          // EXECUTE PIN OUT
            {{"Value", 0}}, // INPUT PINS
            {{"", 0}},      // OUTPUT PINS
            position        // PIN POSITION
        );

        PremakeNode floatNode(
            "Const",
            "Float",
            [](CustomNode *node)
            {
                node->SetOutputValue<float>(0, node->GetInputValue<float>(0, 0.0f));
            },
            SCRIPT,            // CATEGORY
            false,             // EXECUTE PIN INPUT
            false,             // EXECUTE PIN OUT
            {{"Value", 0.0f}}, // INPUT PINS
            {{"", 0.0f}},      // OUTPUT PINS
            position           // PIN POSITION
        );

        PremakeNode quatNode(
            "Const",
            "Quaternion",
            [](CustomNode *node)
            {
                node->SetOutputValue<glm::quat>(0, glm::quat(
                                                       node->GetInputValue<float>(0, 1.0f), // W (por defecto 1, identidad)
                                                       node->GetInputValue<float>(1, 0.0f), // X
                                                       node->GetInputValue<float>(2, 0.0f), // Y
                                                       node->GetInputValue<float>(3, 0.0f)  // Z
                                                       ));
            },
            SCRIPT, // CATEGORY
            false,  // EXECUTE PIN INPUT
            false,  // EXECUTE PIN OUT
            {
                {"W", float(1.0f)},
                {"X", float(0.0f)},
                {"Y", float(0.0f)},
                {"Z", float(0.0f)},
            },                                  // INPUT PINS
            {{"Value", glm::quat(1, 0, 0, 0)}}, // OUTPUT PINS (identidad)
            position                            // PIN POSITION
        );

        engine.PrefabNodes.push_back(quatNode);
        engine.PrefabNodes.push_back(vec3Node);
        engine.PrefabNodes.push_back(vec2Node);
        engine.PrefabNodes.push_back(stringNode);
        engine.PrefabNodes.push_back(intNode);
        engine.PrefabNodes.push_back(floatNode);
    }
};