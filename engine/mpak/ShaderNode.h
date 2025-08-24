#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "MNodeEngine.h"
#include <imgui/imgui.h>
#include "../render/Material.h"

class ShaderNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode getMaterial(
            "Material",
            "Get Material",
            [](CustomNode *node)
            {
                // Intenta obtener el GameObject del pin 0
                GameObject *obj = node->GetInputValue<GameObject *>(0, nullptr);

                // Si no hay conexión, usa el propio objeto del nodo
                if (obj == nullptr)
                    obj = node->_SelfObject;

                // Si todavía es válido y tiene material, lo pasa a la salida
                if (obj != nullptr && obj->material)
                {
                    node->SetOutputValue<Material *>(0, obj->material.get());
                }
                else
                {
                    node->SetOutputValue<Material *>(0, nullptr);
                }
            },
            SCRIPT, false, false,
            {{"Object", (GameObject *)nullptr}},
            {{"Material", (Material *)nullptr}},
            position);

        PremakeNode getMaterialName(
            "Material",
            "Get Material Name",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<std::string>(0, mat->getName());
                }
                else
                {
                    node->SetOutputValue<std::string>(0, "No Material");
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Name", std::string("------")}},
            position);

        engine.PrefabNodes.push_back(getMaterial);
        engine.PrefabNodes.push_back(getMaterialName);
    }
};