#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include <MantraxNodes/MNodeEngine.h>
#include <imgui/imgui.h>

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
            SCRIPT,                // CATEGORY
            true,                  // EXECUTE PIN INPUT
            true,                  // EXECUTE PIN OUT
            {{"Message", "NONE"}}, // INPUT PINS
            {},                    // OUTPUT PINS
            position               // PIN POSITION
        );

        engine.PrefabNodes.push_back(printNode);
    }
};
