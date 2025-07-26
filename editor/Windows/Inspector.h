#pragma once
#include "WindowBehaviour.h"
#include "../../src/components/GameObject.h"
#include "../../src/components/AudioSource.h"
#include "render/Light.h"
#include <memory>

class Inspector : public WindowBehaviour {
public:
    Inspector() = default;
    void OnRenderGUI() override;

private:
    void RenderGameObjectInspector(GameObject* gameObject);
    void RenderLightInspector(std::shared_ptr<Light> light);
};