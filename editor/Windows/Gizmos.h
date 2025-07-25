#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Forward declarations
class GameObject;
class Camera;

class Gizmos : public WindowBehaviour
{
public:
    void OnRenderGUI() override;
};