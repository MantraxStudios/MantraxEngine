#include "ModelScene.h"
#include "../render/AssimpGeometry.h"
#include "../render/NativeGeometry.h"
#include "../render/Material.h"
#include "../render/RenderPipeline.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cstdlib>

void ModelScene::initialize() {
    std::cout << "🚀 ModelScene::initialize() called - Creating massive scene with 5000+ objects" << std::endl;
    
    // Create camera
    auto newCamera = std::make_unique<Camera>(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    newCamera->setPosition({ 0.0f, 5.0f, 10.0f });
    newCamera->setTarget({ 0.0f, 0.0f, 0.0f });
    setCamera(std::move(newCamera));

    // Ensure we have RenderPipeline access
    if (!renderPipeline) {
        std::cerr << "ERROR: ModelScene initialized without RenderPipeline!" << std::endl;
        return;
    }

    // Load 3D model using RenderPipeline
    std::vector<std::string> modelPaths = {
        "Cube.fbx",
        "x64/Debug/Cube.fbx", 
        "../Cube.fbx",
        "models/Cube.fbx"
    };
    
    for (const auto& path : modelPaths) {
        modelGeometry = renderPipeline->loadModel(path);
        if (modelGeometry) {
            if (modelGeometry->isLoaded()) {
                std::cout << "Successfully loaded model from: " << path << std::endl;
                break;
            }
        }
    }

            // Crear modelo de carro usando el nuevo sistema de carga automática
            auto* carModel = new GameObject("x64/Debug/oldcar.fbx");
            carModel->Name = "OldCar";
            carModel->setLocalPosition({ 0.0f, 5.0f, 0.0f });
            carModel->setLocalScale({ 0.01f, 0.01f, 0.01f });
            carModel->setLocalRotationEuler({45.0f, 15.0f, 30.0f});
            
            // Solo agregar si se cargó correctamente
            if (carModel->hasGeometry()) {
                addGameObject(carModel);
                std::cout << "✅ Car model loaded and added to scene" << std::endl;
            } else {
                std::cout << "⚠️ Car model failed to load, not adding to scene" << std::endl;
                delete carModel; // Limpiar memoria si no se cargó
            }
    
    if (modelGeometry && modelGeometry->isLoaded()) {
        std::cout << "Model loaded successfully!" << std::endl;
        
        // Get materials by name from RenderPipeline
        auto redMaterial = renderPipeline->getMaterial("red_material");
        auto blueMaterial = renderPipeline->getMaterial("blue_material");
        auto greenMaterial = renderPipeline->getMaterial("green_material");
        auto goldMaterial = renderPipeline->getMaterial("gold_material");
        
        // Verify materials were loaded
        if (!redMaterial || !blueMaterial || !greenMaterial || !goldMaterial) {
            std::cerr << "ERROR: Failed to load required materials in ModelScene" << std::endl;
            return;
        }

        // Create multiple instances of the same model with different materials and positions
        auto* redModel = new GameObject(modelGeometry);
        redModel->setLocalPosition({ -3.0f, 0.0f, 0.0f });
        redModel->setMaterial(redMaterial);
        addGameObject(redModel);

        auto* blueModel = new GameObject(modelGeometry);
        blueModel->setLocalPosition({ -1.0f, 0.0f, 0.0f });
        blueModel->setMaterial(blueMaterial);
        addGameObject(blueModel);

        auto* greenModel = new GameObject(modelGeometry);
        greenModel->setLocalPosition({ 1.0f, 0.0f, 0.0f });
        greenModel->setMaterial(greenMaterial);
        addGameObject(greenModel);

        auto* goldModel = new GameObject(modelGeometry);
        goldModel->setLocalPosition({ 3.0f, 0.0f, 0.0f });
        goldModel->setLocalScale({ 1.2f, 1.2f, 1.2f });
        goldModel->setMaterial(goldMaterial);
        addGameObject(goldModel);

        // Create 5000 instances for massive stress testing
        std::cout << "Creating 5000 objects for performance testing..." << std::endl;
        
        const int gridSize = 100; // 71x71 = 5041, close to 5000
        const float spacing = 2.0f;
        const float gridOffset = (gridSize - 1) * spacing * 0.5f;
        
        int objectCount = 0;
        for (int i = 0; i < gridSize && objectCount < 5000; i++) {
            for (int j = 0; j < gridSize && objectCount < 5000; j++) {
                auto* instance = new GameObject(modelGeometry);
                
                // Position in grid with some random variation
                float randomX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
                float randomZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
                float randomY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f;
                
                instance->setLocalPosition({ 
                    -gridOffset + i * spacing + randomX, 
                    randomY, 
                    -gridOffset + j * spacing + randomZ 
                });
                
                // Random rotation
                instance->setLocalRotationEuler({ 
                    static_cast<float>(rand() % 360), 
                    static_cast<float>(rand() % 360), 
                    static_cast<float>(rand() % 360)
                });
                
                // Random scale variation
                float scaleVariation = 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f; // 0.7 to 1.3
                instance->setLocalScale({ scaleVariation, scaleVariation, scaleVariation });
                
                // Choose material based on pattern
                int materialChoice = (i * gridSize + j) % 4;
                if (materialChoice == 0) instance->setMaterial(redMaterial);
                else if (materialChoice == 1) instance->setMaterial(blueMaterial);
                else if (materialChoice == 2) instance->setMaterial(greenMaterial);
                else instance->setMaterial(goldMaterial);
                
                addGameObject(instance);
                objectCount++;
            }
        }
        
        std::cout << "✅ Created exactly " << objectCount << " objects in " << gridSize << "x" << gridSize << " grid" << std::endl;
        std::cout << "📊 Total GameObjects: " << getGameObjects().size() << std::endl;
        std::cout << "🎯 Grid area: " << (gridSize * spacing) << "x" << (gridSize * spacing) << " units" << std::endl;
        std::cout << "⚡ Performance test ready - use frustum culling!" << std::endl;
    } else {
        std::cerr << "Failed to load any 3D model from the attempted paths" << std::endl;
        std::cerr << "Falling back to basic cube scene..." << std::endl;
        
        // Fallback: Create 5000 basic cubes if model loading fails
        std::cout << "Creating 5000 fallback cubes for performance testing..." << std::endl;
        
        auto fallbackGeometry = renderPipeline->createNativeGeometry();
        auto basicMaterial = renderPipeline->getMaterial("basic_material");
        auto redMaterial = renderPipeline->getMaterial("red_material");
        auto blueMaterial = renderPipeline->getMaterial("blue_material");
        auto greenMaterial = renderPipeline->getMaterial("green_material");
        
        // Verify fallback materials were loaded
        if (!basicMaterial || !redMaterial || !blueMaterial || !greenMaterial) {
            std::cerr << "ERROR: Failed to load fallback materials in ModelScene" << std::endl;
            return;
        }
        
        // Create 5000 cubes in the same grid pattern
        const int gridSize = 71;
        const float spacing = 2.0f;
        const float gridOffset = (gridSize - 1) * spacing * 0.5f;
        
        int objectCount = 0;
        for (int i = 0; i < gridSize && objectCount < 5000; i++) {
            for (int j = 0; j < gridSize && objectCount < 5000; j++) {
                auto* cube = new GameObject(fallbackGeometry);
                
                // Position in grid with some random variation
                float randomX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
                float randomZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
                float randomY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f;
                
                cube->setLocalPosition({ 
                    -gridOffset + i * spacing + randomX, 
                    randomY, 
                    -gridOffset + j * spacing + randomZ 
                });
                
                // Random rotation
                cube->setLocalRotationEuler({ 
                    static_cast<float>(rand() % 360), 
                    static_cast<float>(rand() % 360), 
                    static_cast<float>(rand() % 360)
                });
                
                // Random scale variation
                float scaleVariation = 0.7f + (static_cast<float>(rand()) / RAND_MAX) * 0.6f;
                cube->setLocalScale({ scaleVariation, scaleVariation, scaleVariation });
                
                // Choose material based on pattern
                int materialChoice = (i * gridSize + j) % 4;
                if (materialChoice == 0) cube->setMaterial(redMaterial);
                else if (materialChoice == 1) cube->setMaterial(blueMaterial);
                else if (materialChoice == 2) cube->setMaterial(greenMaterial);
                else cube->setMaterial(basicMaterial);
                
                addGameObject(cube);
                objectCount++;
            }
        }
        
        std::cout << "✅ Created exactly " << objectCount << " fallback cubes" << std::endl;
        std::cout << "📊 Total GameObjects: " << getGameObjects().size() << std::endl;
        std::cout << "🎯 Fallback grid area: " << (gridSize * spacing) << "x" << (gridSize * spacing) << " units" << std::endl;
        std::cout << "⚡ Performance test ready with fallback geometry!" << std::endl;
    }

    // Add lighting
    auto directionalLight = std::make_shared<Light>(LightType::Directional);
    directionalLight->setDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
    directionalLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    directionalLight->setIntensity(1.0f);
    addLight(directionalLight);

    // Add point lights for better illumination
    auto pointLight1 = std::make_shared<Light>(LightType::Point);
    pointLight1->setPosition(glm::vec3(5.0f, 5.0f, 5.0f));
    pointLight1->setColor(glm::vec3(1.0f, 0.8f, 0.6f));
    pointLight1->setIntensity(2.0f);
    addLight(pointLight1);

    auto pointLight2 = std::make_shared<Light>(LightType::Point);
    pointLight2->setPosition(glm::vec3(-5.0f, 5.0f, -5.0f));
    pointLight2->setColor(glm::vec3(0.6f, 0.8f, 1.0f));
    pointLight2->setIntensity(2.0f);
    addLight(pointLight2);
    
    std::cout << "✅ ModelScene initialization complete - Total objects: " << getGameObjects().size() << std::endl;
}

void ModelScene::update(float deltaTime) {
    // Update logic optimized for 5000 objects
    static float time = 0.0f;
    time += deltaTime;
    
    auto& gameObjects = getGameObjects();
    
    // Only animate the first few hero objects to maintain performance
    if (gameObjects.size() > 4) {
        // Rotate the gold model (hero object)
        gameObjects[3]->setLocalRotationEuler({ 0.0f, time * 30.0f, 0.0f });
        
        // Animate only a small subset of the 5000 objects (every 100th object)
        for (size_t i = 4; i < gameObjects.size(); i += 100) {
            float offset = (i / 100) * 0.1f;
            gameObjects[i]->setLocalRotationEuler({ 
                time * 10.0f + offset, 
                time * 8.0f + offset, 
                time * 6.0f + offset 
            });
        }
    }
    
    // Performance tracking
    static float lastPrintTime = 0.0f;
    if (time - lastPrintTime > 5.0f) { // Print every 5 seconds
        lastPrintTime = time;
        std::cout << "📊 Performance: Managing " << gameObjects.size() << " objects" << std::endl;
    }
} 

