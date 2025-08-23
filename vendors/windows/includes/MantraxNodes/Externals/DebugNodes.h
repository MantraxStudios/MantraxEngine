#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include <MantraxNodes/MNodeEngine.h>
#include <imgui/imgui.h>
#include <any>
#include <sstream>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class DebugNodes
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode printNode(
            "Debug",
            "Print",
            [](CustomNode *node)
            {
                std::cout << node->GetInputValue<std::string>(1, "HI") << std::endl;
            },
            SCRIPT,                             // CATEGORY
            true,                               // EXECUTE PIN INPUT
            true,                               // EXECUTE PIN OUT
            {{"Message", std::string("NONE")}}, // INPUT PINS
            {},                                 // OUTPUT PINS
            position                            // PIN POSITION
        );

        engine.PrefabNodes.push_back(printNode);
    }
};
