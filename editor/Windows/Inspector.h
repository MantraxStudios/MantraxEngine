#pragma once
#include "WindowBehaviour.h"
#include "components/GameObject.h"
#include "render/Light.h"
#include <memory>

class Inspector : public WindowBehaviour {
public:
    void OnRenderGUI() override;

private:
    void RenderGameObjectInspector(GameObject* go);
    void RenderLightInspector(std::shared_ptr<Light> light);
};