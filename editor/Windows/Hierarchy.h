#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class Hierarchy : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
};