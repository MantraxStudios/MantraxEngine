#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include "components/GameObject.h"

class Hierarchy : public WindowBehaviour {
public:
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Hierarchy";
        return name;
    }

    bool isChildOf(GameObject* possibleParent, GameObject* child);
    void RenderGameObjectNode(GameObject* gameObject);
};