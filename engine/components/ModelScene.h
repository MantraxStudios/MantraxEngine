#pragma once
#include "Scene.h"
#include <memory>

// Forward declarations
class AssimpGeometry;

class MANTRAXCORE_API ModelScene : public Scene {
public:
    ModelScene() : Scene("ModelScene") {}
    
    void initialize() override;
    void update(float deltaTime) override;

private:
    std::shared_ptr<AssimpGeometry> modelGeometry;
}; 