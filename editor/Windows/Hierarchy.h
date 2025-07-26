#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class Hierarchy : public WindowBehaviour {
public:
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Hierarchy";
        return name;
    }
};