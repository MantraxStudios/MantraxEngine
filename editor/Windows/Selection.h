#pragma once
#include <imgui/imgui.h>
#include <components/GameObject.h>
#include <render/Light.h>
#include <memory>

class Selection
{
public:
    static GameObject* GameObjectSelect;
    static std::shared_ptr<Light> LightSelect;
};