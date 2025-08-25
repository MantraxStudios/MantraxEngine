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

        PremakeNode setAlbedoNode(
            "Material",
            "Set Albedo",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setAlbedo(node->GetInputValue<glm::vec3>(2, glm::vec3(1.0f)));
                    node->SetOutputValue<glm::vec3>(1, mat->getAlbedo());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Color", glm::vec3(0, 0, 0)}},
            {{"Color", glm::vec3(0, 0, 0)}},
            position);

        PremakeNode setAlphaNode(
            "Material",
            "Set Alpha",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setAlpha(node->GetInputValue<float>(2, 1.0f));
                    node->SetOutputValue<float>(1, mat->getAlpha());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Intensity", 1.0f}},
            {{"Intensity", 0.0f}},
            position);

        PremakeNode setMetallicNode(
            "Material",
            "Set Metallic",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setMetallic(node->GetInputValue<float>(2, 1.0f));
                    node->SetOutputValue<float>(1, mat->getMetallic());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Intensity", 1.0f}},
            {{"Intensity", 0.0f}},
            position);

        PremakeNode setRoughnessNode(
            "Material",
            "Set Roughness",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setRoughness(node->GetInputValue<float>(2, 1.0f));
                    node->SetOutputValue<float>(1, mat->getRoughness());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Intensity", 1.0f}},
            {{"Intensity", 0.0f}},
            position);

        PremakeNode setNormalStrength(
            "Material",
            "Set Normal Strength",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setNormalStrength(node->GetInputValue<float>(2, 1.0f));
                    node->SetOutputValue<float>(1, mat->getNormalStrength());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Intensity", 1.0f}},
            {{"Intensity", 0.0f}},
            position);

        PremakeNode setEmissiveNode(
            "Material",
            "Set Emissive",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setEmissive(node->GetInputValue<glm::vec3>(2, glm::vec3(1.0f)));
                    node->SetOutputValue<glm::vec3>(1, mat->getEmissive());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Intensity", glm::vec3(1.0f)}},
            {{"Intensity", glm::vec3(1.0f)}},
            position);

        PremakeNode setTilingNode(
            "Material",
            "Set Tiling",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(1, nullptr);
                if (mat != nullptr)
                {
                    mat->setTiling(node->GetInputValue<glm::vec2>(2, glm::vec2(1.0f)));
                    node->SetOutputValue<glm::vec2>(1, mat->getEmissive());
                }
            },
            SCRIPT, true, true,
            {{"Material", (Material *)nullptr}, {"New Tiling", glm::vec2(1.0f)}},
            {{"Tiling", glm::vec2(1.0f)}},
            position);

        // ========= GET VALUES =========

        PremakeNode getAlbedoNode(
            "Material",
            "Get Albedo",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<glm::vec3>(0, mat->getAlbedo());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", glm::vec3(1.0f)}},
            position);

        PremakeNode getMetallicNode(
            "Material",
            "Get Metallic",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<float>(0, mat->getMetallic());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", 1.0f}},
            position);

        PremakeNode getRoughnessNode(
            "Material",
            "Get Roughness",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<float>(0, mat->getRoughness());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", 1.0f}},
            position);

        PremakeNode getEmissiveNode(
            "Material",
            "Get Emissive",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<glm::vec3>(0, mat->getEmissive());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", glm::vec3(1.0f)}},
            position);

        PremakeNode getTilingNode(
            "Material",
            "Get Tiling",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<glm::vec2>(0, mat->getEmissive());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", glm::vec2(1.0f)}},
            position);

        PremakeNode getNormalStrengthNode(
            "Material",
            "Get Normal Strength",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<float>(0, mat->getNormalStrength());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", 1.0f}},
            position);

        PremakeNode getAlphaNode(
            "Material",
            "Get Alpha",
            [](CustomNode *node)
            {
                Material *mat = node->GetInputValue<Material *>(0, nullptr);
                if (mat != nullptr)
                {
                    node->SetOutputValue<float>(0, mat->getAlpha());
                }
            },
            SCRIPT, false, false,
            {{"Material", (Material *)nullptr}},
            {{"Value", 1.0f}},
            position);

        engine.PrefabNodes.push_back(getMaterial);
        engine.PrefabNodes.push_back(getMaterialName);
        engine.PrefabNodes.push_back(setAlbedoNode);
        engine.PrefabNodes.push_back(setAlphaNode);
        engine.PrefabNodes.push_back(setMetallicNode);
        engine.PrefabNodes.push_back(setRoughnessNode);
        engine.PrefabNodes.push_back(setEmissiveNode);
        engine.PrefabNodes.push_back(setNormalStrength);

        // === ADD NODES
        engine.PrefabNodes.push_back(getAlbedoNode);
        engine.PrefabNodes.push_back(getMetallicNode);
        engine.PrefabNodes.push_back(getRoughnessNode);
        engine.PrefabNodes.push_back(getEmissiveNode);
        engine.PrefabNodes.push_back(getTilingNode);
        engine.PrefabNodes.push_back(getNormalStrengthNode);
        engine.PrefabNodes.push_back(getAlphaNode);
    }
};