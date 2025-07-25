#pragma once
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class SceneView : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
};