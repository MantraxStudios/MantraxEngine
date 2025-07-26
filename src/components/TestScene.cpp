#include "TestScene.h"

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

    // Get materials by name from RenderPipeline
    auto redMaterial = renderPipeline->getMaterial("red_material");
    auto blueMaterial = renderPipeline->getMaterial("blue_material");
    auto greenMaterial = renderPipeline->getMaterial("green_material");
    
    // Verify materials were loaded
    if (!redMaterial || !blueMaterial || !greenMaterial) {
        std::cerr << "ERROR: Failed to load required materials in TestScene" << std::endl;
        return;
    }

    // Crear objetos basicos sin geometria (objetos vacios)
    auto* redCube = new GameObject();
    redCube->Name = "RedCube";
    redCube->setLocalPosition({ -2.0f, 0.0f, 0.0f });
    redCube->setMaterial(redMaterial);
    addGameObject(redCube);

    auto* blueCube = new GameObject();
    blueCube->Name = "BlueCube";
    blueCube->setLocalPosition({ 0.0f, 0.0f, 0.0f });
    blueCube->setMaterial(blueMaterial);
    addGameObject(blueCube);

    auto* greenCube = new GameObject();
    greenCube->Name = "GreenCube";
    greenCube->setLocalPosition({ 2.0f, 0.0f, 0.0f });
    greenCube->setMaterial(greenMaterial);
    addGameObject(greenCube);

    // Ejemplo de GameObject vacio (sin geometria) - no se renderiza
    auto* emptyObject = new GameObject();
    emptyObject->Name = "EmptyObject";
    emptyObject->setLocalPosition({ 0.0f, 3.0f, 0.0f });
    emptyObject->setLocalScale({ 2.0f, 2.0f, 2.0f });
    addGameObject(emptyObject);
    
    // Ejemplo de GameObject que recibe geometria despues de la creacion
    auto* delayedGeometryObject = new GameObject();
    delayedGeometryObject->Name = "DelayedGeometryObject";
    delayedGeometryObject->setLocalPosition({ 4.0f, 0.0f, 0.0f });
    // Nota: Sin geometria por defecto, se puede cargar un modelo despues
    delayedGeometryObject->setMaterial(redMaterial);
    addGameObject(delayedGeometryObject);

    // Ejemplo de GameObject con carga automatica de modelo desde path
    // Nota: Este objeto intentara cargar "models/cube.obj" pero si no existe, quedara vacio
    auto* autoLoadObject = new GameObject("models/cube.obj");
    autoLoadObject->Name = "AutoLoadObject";
    autoLoadObject->setLocalPosition({ 6.0f, 0.0f, 0.0f });
    autoLoadObject->setMaterial(blueMaterial);
    addGameObject(autoLoadObject);

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