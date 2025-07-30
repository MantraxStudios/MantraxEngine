#pragma once
#include <iostream>
#include <render/RenderPipeline.h>

class EditorInfo {
public:
	static float RenderPositionX;
	static float RenderPositionY;
	static bool IsHoveringScene;
	static RenderPipeline* pipeline;
	static std::string currentScenePath;
};