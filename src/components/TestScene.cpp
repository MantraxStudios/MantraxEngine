#include "TestScene.h"
#include "../render/NativeGeometry.h"
#include "../render/Material.h"
#include "../render/RenderPipeline.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void TestScene::initialize() {
    std::cout << "TestScene::initialize() called" << std::endl;
    
    // Create camera
    auto newCamera = std::make_unique<Camera>(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    newCamera->setPosition({ 0.0f, 5.0f, 10.0f });
    newCamera->setTarget({ 0.0f, 0.0f, 0.0f });
    std::cout << "TestScene: Camera created, setting to scene..." << std::endl;
    setCamera(std::move(newCamera));
    std::cout << "TestScene: Camera set. Has camera: " << (getCamera() ? "YES" : "NO") << std::endl;

    // Ensure we have RenderPipeline access
    if (!renderPipeline) {
        std::cerr << "ERROR: TestScene initialized without RenderPipeline!" << std::endl;
        return;
    }

    // Create materials using RenderPipeline
    auto redMaterial = renderPipeline->createMaterial(glm::vec3(0.9f, 0.2f, 0.2f), "Red Material");
    auto blueMaterial = renderPipeline->createMaterial(glm::vec3(0.2f, 0.3f, 0.9f), "Blue Material");
    auto greenMaterial = renderPipeline->createMaterial(glm::vec3(0.2f, 0.9f, 0.2f), "Green Material");

    // Create geometry using RenderPipeline
    cubeGeometry = renderPipeline->createNativeGeometry();

    // Create some cubes with different materials
    auto* redCube = new GameObject(cubeGeometry);
    redCube->setLocalPosition({ -2.0f, 0.0f, 0.0f });
    redCube->setMaterial(redMaterial);
    addGameObject(redCube);

    auto* blueCube = new GameObject(cubeGeometry);
    blueCube->setLocalPosition({ 0.0f, 0.0f, 0.0f });
    blueCube->setMaterial(blueMaterial);
    addGameObject(blueCube);

    auto* greenCube = new GameObject(cubeGeometry);
    greenCube->setLocalPosition({ 2.0f, 0.0f, 0.0f });
    greenCube->setMaterial(greenMaterial);
    addGameObject(greenCube);

    // Add a directional light
    auto directionalLight = std::make_shared<Light>(LightType::Directional);
    directionalLight->setDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
    directionalLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    directionalLight->setIntensity(1.0f);
    addLight(directionalLight);
    
    std::cout << "TestScene initialized using RenderPipeline resource management" << std::endl;
}

void TestScene::update(float deltaTime) {
    // Add any scene-specific update logic here
    // For example, you could rotate objects, update animations, etc.
}