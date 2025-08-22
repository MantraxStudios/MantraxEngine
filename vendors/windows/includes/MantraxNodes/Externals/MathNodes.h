#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include <MantraxNodes/MNodeEngine.h>
#include <imgui/imgui.h>

class MathNodes
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode vec3Node(
            "Math",
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

        engine.PrefabNodes.push_back(vec3Node);
    }
};
