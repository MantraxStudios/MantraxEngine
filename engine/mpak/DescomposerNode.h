#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/SceneManager.h>
#include "MNodeEngine.h"
#include <imgui/imgui.h>
#include <any>
#include <sstream>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class DescomposerNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        // ----------- VEC2 TO FLOATS -----------
        PremakeNode vec2ToFloatsNode(
            "Decompose", "Vec2 To Floats",
            [](CustomNode *node)
            {
                glm::vec2 input = node->GetInputValue<glm::vec2>(0, glm::vec2(0.0f));
                node->SetOutputValue<float>(0, input.x);
                node->SetOutputValue<float>(1, input.y);
            },
            SCRIPT, false, false,
            {{"Vec2", glm::vec2(0.0f)}},
            {{"X", 0.0f}, {"Y", 0.0f}},
            position);

        // ----------- VEC3 TO FLOATS -----------
        PremakeNode vec3ToFloatsNode(
            "Decompose", "Vec3 To Floats",
            [](CustomNode *node)
            {
                glm::vec3 input = node->GetInputValue<glm::vec3>(0, glm::vec3(0.0f));
                node->SetOutputValue<float>(0, input.x);
                node->SetOutputValue<float>(1, input.y);
                node->SetOutputValue<float>(2, input.z);
            },
            SCRIPT, false, false,
            {{"Vec3", glm::vec3(0.0f)}},
            {{"X", 0.0f}, {"Y", 0.0f}, {"Z", 0.0f}},
            position);

        // ----------- MAT4 TO VEC4 COLUMNS -----------
        PremakeNode mat4ToVec4Node(
            "Decompose", "Mat4 To Vec4 Columns",
            [](CustomNode *node)
            {
                glm::mat4 input = node->GetInputValue<glm::mat4>(0, glm::mat4(1.0f));
                node->SetOutputValue<glm::vec4>(0, input[0]); // Columna 0
                node->SetOutputValue<glm::vec4>(1, input[1]); // Columna 1
                node->SetOutputValue<glm::vec4>(2, input[2]); // Columna 2
                node->SetOutputValue<glm::vec4>(3, input[3]); // Columna 3
            },
            SCRIPT, false, false,
            {{"Mat4", glm::mat4(1.0f)}},
            {{"Col0", glm::vec4(0)}, {"Col1", glm::vec4(0)}, {"Col2", glm::vec4(0)}, {"Col3", glm::vec4(0)}},
            position);

        // ----------- MAT4 TO FLOATS -----------
        PremakeNode mat4ToFloatsNode(
            "Decompose", "Mat4 To Floats",
            [](CustomNode *node)
            {
                glm::mat4 input = node->GetInputValue<glm::mat4>(0, glm::mat4(1.0f));
                int outIndex = 0;
                for (int col = 0; col < 4; col++)
                {
                    for (int row = 0; row < 4; row++)
                    {
                        node->SetOutputValue<float>(outIndex++, input[col][row]);
                    }
                }
            },
            SCRIPT, false, false,
            {{"Mat4", glm::mat4(1.0f)}},
            {{"m00", 0}, {"m01", 0}, {"m02", 0}, {"m03", 0}, {"m10", 0}, {"m11", 0}, {"m12", 0}, {"m13", 0}, {"m20", 0}, {"m21", 0}, {"m22", 0}, {"m23", 0}, {"m30", 0}, {"m31", 0}, {"m32", 0}, {"m33", 0}},
            position);

        // ----------- MAT3 TO FLOATS -----------
        PremakeNode mat3ToFloatsNode(
            "Decompose", "Mat3 To Floats",
            [](CustomNode *node)
            {
                glm::mat3 input = node->GetInputValue<glm::mat3>(0, glm::mat3(1.0f));
                int outIndex = 0;
                for (int col = 0; col < 3; col++)
                {
                    for (int row = 0; row < 3; row++)
                    {
                        node->SetOutputValue<float>(outIndex++, input[col][row]);
                    }
                }
            },
            SCRIPT, false, false,
            {{"Mat3", glm::mat3(1.0f)}},
            {{"m00", 0}, {"m01", 0}, {"m02", 0}, {"m10", 0}, {"m11", 0}, {"m12", 0}, {"m20", 0}, {"m21", 0}, {"m22", 0}},
            position);

        engine.PrefabNodes.push_back(vec2ToFloatsNode);
        engine.PrefabNodes.push_back(vec3ToFloatsNode);
        engine.PrefabNodes.push_back(mat4ToVec4Node);
        engine.PrefabNodes.push_back(mat4ToFloatsNode);
        engine.PrefabNodes.push_back(mat3ToFloatsNode);
    }
};