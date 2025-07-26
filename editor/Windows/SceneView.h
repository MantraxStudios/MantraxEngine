#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>


class SceneView : public WindowBehaviour
{
public:
	glm::vec2 WorldPoint;
	void OnRenderGUI() override;
	const std::string& getName() const override {
		static const std::string name = "Scene View";
		return name;
	}
	static bool mouseCaptured;
};