#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class Inspector : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
};