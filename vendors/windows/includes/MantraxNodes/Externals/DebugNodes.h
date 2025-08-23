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
    static std::string AnyToString(const std::any &value)
    {
        if (value.type() == typeid(int))
        {
            return std::to_string(std::any_cast<int>(value));
        }
        else if (value.type() == typeid(float))
        {
            return std::to_string(std::any_cast<float>(value));
        }
        else if (value.type() == typeid(double))
        {
            return std::to_string(std::any_cast<double>(value));
        }
        else if (value.type() == typeid(bool))
        {
            return std::any_cast<bool>(value) ? "true" : "false";
        }
        else if (value.type() == typeid(std::string))
        {
            return std::any_cast<std::string>(value);
        }
        else if (value.type() == typeid(const char *))
        {
            return std::string(std::any_cast<const char *>(value));
        }
        else if (value.type() == typeid(glm::vec2))
        {
            auto v = std::any_cast<glm::vec2>(value);
            return "vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        }
        else if (value.type() == typeid(glm::vec3))
        {
            auto v = std::any_cast<glm::vec3>(value);
            return "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        }
        else if (value.type() == typeid(glm::vec4))
        {
            auto v = std::any_cast<glm::vec4>(value);
            return "vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
        }

        return "<unsupported type>";
    }

    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode printNode(
            "Debug",
            "Print",
            [](CustomNode *node)
            {
                // obtener como std::any en lugar de string fijo
                std::any val = node->GetInputValue<std::any>(1, std::string("HI"));
                std::cout << DebugNodes::AnyToString(val) << std::endl;
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
