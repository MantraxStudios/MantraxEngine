#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "MNodeEditor.h"
#include <mpak/MNodeEngine.h>

#include <iostream>
#include <vector>

class EditorNode : public WindowBehaviour
{
public:
    std::vector<MNodeEditor *> Engines_Nodes;
    void OnRenderGUI() override;

    const std::string &getName() const override
    {
        static const std::string name = "Glyphs";
        return name;
    }

    void OpenNewEditor(MNodeEngine *engine);
};