#pragma once

#include "Scene.h"
#include <memory>
#include "../core/CoreExporter.h"

// Forward declarations
class NativeGeometry;
class Material;

class MANTRAXCORE_API TexturedScene : public Scene {
public:
    TexturedScene() : Scene("TexturedScene") {}
    
    void initialize() override;
    void update(float deltaTime) override;

private:
    std::shared_ptr<NativeGeometry> cubeGeometry;
}; 