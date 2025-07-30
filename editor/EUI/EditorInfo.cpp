#pragma once
#include "EditorInfo.h"
#include <iostream>



float EditorInfo::RenderPositionX = 0.0f;
float EditorInfo::RenderPositionY = 0.0f;
bool EditorInfo::IsHoveringScene = false;
RenderPipeline* EditorInfo::pipeline = nullptr;
std::string EditorInfo::currentScenePath;