#pragma once

#include "Scene.h"
#include <memory>
#include "../core/CoreExporter.h"

// Forward declarations


class MANTRAXCORE_API TestScene : public Scene {
public:
    TestScene() : Scene("TestScene") {}
    
    void initialize() override;
    void update(float deltaTime) override;

private:
    
}; 