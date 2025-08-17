#include "TexturedScene.h"

#include "../render/Material.h"
#include "../render/RenderPipeline.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void TexturedScene::initialize() {
    // Create camera
    auto newCamera = std::make_unique<Camera>(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    newCamera->setPosition({ 0.0f, 5.0f, 10.0f });
    newCamera->setTarget({ 0.0f, 0.0f, 0.0f });
    setCamera(std::move(newCamera));

    // Ensure we have RenderPipeline access
    if (!renderPipeline) {
        std::cerr << "ERROR: TexturedScene initialized without RenderPipeline!" << std::endl;
        return;
    }

    // Get PBR material by name from RenderPipeline
    auto pbrMaterial = renderPipeline->getMaterial("diamond_plate_material");
    
    // Verify material was loaded
    if (!pbrMaterial) {
        std::cerr << "ERROR: Failed to load diamond_plate_material in TexturedScene" << std::endl;
        return;
    }

    // Create a grid of textured cubes (sin geometria por defecto)
    const int gridSize = 3;
    const float spacing = 3.0f;
    const float startPos = -((gridSize - 1) * spacing) / 2.0f;

    for (int x = 0; x < gridSize; x++) {
        for (int z = 0; z < gridSize; z++) {
            auto* cube = new GameObject();
            float xPos = startPos + x * spacing;
            float zPos = startPos + z * spacing;
            cube->setLocalPosition({ xPos, 0.0f, zPos });
            cube->setMaterial(pbrMaterial);
            
            // Add some variation in rotation
            float rotY = (x + z) * 45.0f;
            glm::quat rotation = glm::angleAxis(glm::radians(rotY), glm::vec3(0, 1, 0));
            cube->setLocalRotationQuat(rotation);
            
            addGameObject(cube);
        }
    }

    // Add a directional light
    auto directionalLight = std::make_shared<Light>(LightType::Directional);
    directionalLight->setDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
    directionalLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    directionalLight->setIntensity(1.0f);
    addLight(directionalLight);

    // Add a point light for extra detail
    auto pointLight = std::make_shared<Light>(LightType::Point);
    pointLight->setPosition(glm::vec3(2.0f, 4.0f, 2.0f));
    pointLight->setColor(glm::vec3(1.0f, 0.9f, 0.8f));
    pointLight->setIntensity(2.0f);
    pointLight->setAttenuation(1.0f, 0.09f, 0.032f);
    addLight(pointLight);
}

void TexturedScene::update(float deltaTime) {
    // Add any scene-specific update logic here
} 