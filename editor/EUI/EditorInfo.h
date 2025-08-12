#pragma once
#include <iostream>
#include <render/RenderPipeline.h>

class EditorInfo {
public:
	static float RenderPositionX;
	static float RenderPositionY;
	static bool IsHoveringScene;
	static bool IsPlaying;
	static std::string SelectedProject;
	static std::string SelectedProjectPath;
	static RenderPipeline* pipeline;
	static std::string currentScenePath;
};