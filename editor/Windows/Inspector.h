#pragma once
#include "WindowBehaviour.h"
#include "../../src/components/GameObject.h"
#include "../../src/components/AudioSource.h"
#include "../../src/components/PhysicalObject.h"
#include "../../src/core/PhysicsManager.h"
#include "render/Light.h"
#include <memory>

class Inspector : public WindowBehaviour {
public:
    Inspector() = default;
    void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Inspector";
        return name;
    }

private:
    void RenderGameObjectInspector(GameObject* gameObject);
    void RenderLightInspector(std::shared_ptr<Light> light);
};