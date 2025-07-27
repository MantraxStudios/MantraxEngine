#include "PhysicsTestScene.h"
#include "../render/AssimpGeometry.h"
#include "../render/Material.h"
#include "../render/RenderPipeline.h"
#include "../components/PhysicalObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>

PhysicsTestScene::PhysicsTestScene() : Scene("Physics Test Scene") {
}

void PhysicsTestScene::initialize() {
    std::cout << "[START] PhysicsTestScene::initialize() called - Creating physics test scene" << std::endl;
    
    // Create camera
    auto newCamera = std::make_unique<Camera>(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    newCamera->setPosition({ 0.0f, 10.0f, 15.0f });
    newCamera->setTarget({ 0.0f, 0.0f, 0.0f });
    setCamera(std::move(newCamera));

    // Ensure we have RenderPipeline access
    if (!renderPipeline) {
        std::cerr << "ERROR: PhysicsTestScene initialized without RenderPipeline!" << std::endl;
        return;
    }

    // Create materials
    createPhysicsMaterials();

    // Create ground plane
    createGround();

    // Create falling cubes
    createFallingCubes();

    // Add a directional light
    auto directionalLight = std::make_shared<Light>(LightType::Directional);
    directionalLight->setDirection({ -0.5f, -1.0f, -0.5f });
    directionalLight->setIntensity(1.0f);
    directionalLight->setColor({ 1.0f, 1.0f, 1.0f });
    addLight(directionalLight);

    std::cout << "[END] PhysicsTestScene::initialize() completed successfully" << std::endl;
}

void PhysicsTestScene::update(float deltaTime) {
    // Physics simulation is handled by the PhysicsManager in SceneManager::update()
    Scene::update(deltaTime);
}

void PhysicsTestScene::createPhysicsMaterials() {
    // Create different materials for visual variety
    auto redMaterial = renderPipeline->createMaterial(glm::vec3(0.8f, 0.2f, 0.2f), "RedMaterial");
    auto blueMaterial = renderPipeline->createMaterial(glm::vec3(0.2f, 0.2f, 0.8f), "BlueMaterial");
    auto greenMaterial = renderPipeline->createMaterial(glm::vec3(0.2f, 0.8f, 0.2f), "GreenMaterial");
    auto yellowMaterial = renderPipeline->createMaterial(glm::vec3(0.8f, 0.8f, 0.2f), "YellowMaterial");
}

void PhysicsTestScene::createGround() {
    // Create a simple ground GameObject without specific geometry
    auto* ground = new GameObject();
    ground->Name = "Ground";
    ground->setWorldPosition({ 0.0f, -2.0f, 0.0f });
    ground->setWorldRotationEuler({ -90.0f, 0.0f, 0.0f }); // Rotate to be horizontal
    ground->setMaterial(renderPipeline->createMaterial(glm::vec3(0.5f, 0.5f, 0.5f), "GroundMaterial"));
    
    // Add physics component (static body)
    auto* groundPhysics = ground->addComponent<PhysicalObject>(ground);
    groundPhysics->setBodyType(BodyType::Static);
    groundPhysics->setShapeType(ShapeType::Plane);
    groundPhysics->setFriction(0.8f);
    groundPhysics->setRestitution(0.1f);
    groundPhysics->initialize();
    
    addGameObject(ground);
}

void PhysicsTestScene::createFallingCubes() {
    // Create several cubes that will fall due to gravity
    const int numCubes = 8;
    const float spacing = 2.0f;
    const float startX = -(numCubes - 1) * spacing * 0.5f;
    
    for (int i = 0; i < numCubes; i++) {
        // Create simple cube GameObject without specific geometry
        auto* cube = new GameObject();
        cube->Name = "PhysicsCube_" + std::to_string(i);
        
        // Position cubes in a line above the ground
        float xPos = startX + i * spacing;
        cube->setWorldPosition({ xPos, 5.0f + i * 0.5f, 0.0f });
        
        // Assign different materials based on index
        std::shared_ptr<Material> material;
        switch (i % 4) {
            case 0: material = renderPipeline->createMaterial(glm::vec3(0.8f, 0.2f, 0.2f), "RedMaterial"); break;
            case 1: material = renderPipeline->createMaterial(glm::vec3(0.2f, 0.2f, 0.8f), "BlueMaterial"); break;
            case 2: material = renderPipeline->createMaterial(glm::vec3(0.2f, 0.8f, 0.2f), "GreenMaterial"); break;
            case 3: material = renderPipeline->createMaterial(glm::vec3(0.8f, 0.8f, 0.2f), "YellowMaterial"); break;
        }
        cube->setMaterial(material);
        
        // Add physics component (dynamic body)
        auto* cubePhysics = cube->addComponent<PhysicalObject>(cube);
        cubePhysics->setBodyType(BodyType::Dynamic);
        cubePhysics->setShapeType(ShapeType::Box);
        cubePhysics->setBoxHalfExtents({ 0.5f, 0.5f, 0.5f });
        cubePhysics->setMass(1.0f + i * 0.5f); // Varying masses
        cubePhysics->setFriction(0.3f + i * 0.1f); // Varying friction
        cubePhysics->setRestitution(0.2f + i * 0.1f); // Varying restitution
        cubePhysics->initialize();
        
        addGameObject(cube);
    }
    
    // Create some spheres for variety
    for (int i = 0; i < 4; i++) {
        auto* sphere = new GameObject();
        sphere->Name = "PhysicsSphere_" + std::to_string(i);
        
        float xPos = startX + (i + numCubes) * spacing;
        sphere->setWorldPosition({ xPos, 8.0f, 0.0f });
        sphere->setMaterial(renderPipeline->createMaterial(glm::vec3(0.8f, 0.4f, 0.8f), "PurpleMaterial"));
        
        // Add physics component (dynamic body)
        auto* spherePhysics = sphere->addComponent<PhysicalObject>(sphere);
        spherePhysics->setBodyType(BodyType::Dynamic);
        spherePhysics->setShapeType(ShapeType::Sphere);
        spherePhysics->setSphereRadius(0.5f);
        spherePhysics->setMass(2.0f);
        spherePhysics->setFriction(0.5f);
        spherePhysics->setRestitution(0.8f); // High restitution for bouncy spheres
        spherePhysics->initialize();
        
        addGameObject(sphere);
    }
} 