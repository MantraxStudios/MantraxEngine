#pragma once
#include "WindowBehaviour.h"
#include "../../src/ui/UIBehaviour.h"
#include "../../src/render/RenderPipeline.h"
#include <vector>
#include <memory>
#include <string>

class CanvasManager : public WindowBehaviour {
private:
public:
    void OnRenderGUI() override;
    const std::string& getName() const override;
}; 