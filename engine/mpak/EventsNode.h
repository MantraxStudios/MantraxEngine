#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include "MNodeEngine.h"
#include <imgui/imgui.h>

class EventsNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode startEventNode(
            "Events",
            "On Start",
            [](CustomNode *node)
            {
                node->nodeData["started"] = true;
                node->nodeData["startTime"] = std::chrono::steady_clock::now();
            },
            INPUT_OUTPUT, // category
            false,        // hasExecInput
            true,         // hasExecOutput
            {},           // inputPins
            {},           // outputPins - solo pin de ejecución, sin datos
            position      // PIN POSITION
        );

        PremakeNode tickEventNode(
            "Events",
            "On Tick",
            [](CustomNode *node) {},
            INPUT_OUTPUT, // category
            false,        // hasExecInput
            true,         // hasExecOutput
            {},           // inputPins
            {},           // outputPins - solo pin de ejecución, sin datos
            position      // PIN POSITION
        );

        PremakeNode triggerEventNode(
            "Events",
            "On Trigger",
            [](CustomNode *node) {},
            INPUT_OUTPUT, // category
            false,        // hasExecInput
            true,         // hasExecOutput
            {},           // inputPins
            {},           // outputPins - solo pin de ejecución, sin datos
            position      // PIN POSITION
        );

        engine.PrefabNodes.push_back(startEventNode);
        engine.PrefabNodes.push_back(tickEventNode);
        engine.PrefabNodes.push_back(triggerEventNode);
    }
};