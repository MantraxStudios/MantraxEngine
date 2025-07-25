#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>

class SceneView : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
};