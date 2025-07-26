#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class MainBar : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Main Bar";
        return name;
    }
};