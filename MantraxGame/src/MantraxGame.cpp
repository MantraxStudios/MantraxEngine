#define SDL_MAIN_HANDLED
#define GLM_ENABLE_EXPERIMENTAL

#include <SDL.h>
#include <GL/glew.h>

#include "render/RenderConfig.h"
#include "render/DefaultShaders.h"
#include "render/Camera.h"
#include "core/AudioManager.h"

#include "render/Material.h"
#include "render/Texture.h"
#include "render/Light.h"
#include "components/GameObject.h"
#include "components/Component.h"
#include "render/RenderPipeline.h"
#include "components/SceneManager.h"
#include "components/TestScene.h"
#include "components/ScriptExecutor.h"
#include "components/TexturedScene.h"
#include "components/ModelScene.h"
#include "input/InputSystem.h"
#include "core/Time.h"
#include "ui/UIManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <map>
#include <signal.h>
#include <string>
#include <functional>
#include <exception>
#include "core/InputConfigLoader.h"

// Variable global para manejo seguro del cierre
volatile bool g_running = true;

void signalHandler(int signal) {
    std::cout << "\nSeal de cierre recibida (" << signal << "). Cerrando de forma segura..." << std::endl;
    g_running = false;
}

void setupInputSystem(Scene* activeScene) {
    InputConfigLoader::loadInputConfigFromJSON();

    // Initialize DLL Loader for C# bridge
    auto& inputSystem = InputSystem::getInstance();

    auto moveAction = inputSystem.getAction("Move");
    if (moveAction) {
        moveAction->bindVector2DCallback([activeScene](const glm::vec2& movement) {
            if (Camera* camera = activeScene->getCamera()) {
                const float speed = 1.0f;
                if (movement.y != 0.0f) {
                    camera->moveForward(movement.y * speed * Time::getDeltaTime());
                }
                if (movement.x != 0.0f) {
                    camera->moveRight(movement.x * speed * Time::getDeltaTime());
                }
            }
            });
    }

    auto verticalMoveAction = inputSystem.getAction("VerticalMove");
    if (verticalMoveAction) {
        verticalMoveAction->bindValueCallback([activeScene](float value) {
            if (Camera* camera = activeScene->getCamera()) {
                const float speed = 1.0f;
                camera->moveUp(value * speed * Time::getDeltaTime());
            }
            });
    }

    auto mouseLookX = inputSystem.getAction("MouseLookX");
    if (mouseLookX) {
        mouseLookX->bindMouseAxisCallback([activeScene](float delta) {
            if (Camera* camera = activeScene->getCamera()) {
                const float sensitivity = 0.3f;
                camera->rotate(delta * sensitivity, 0.0f);
            }
            });
    }

    auto mouseLookY = inputSystem.getAction("MouseLookY");
    if (mouseLookY) {
        mouseLookY->bindMouseAxisCallback([activeScene](float delta) {
            if (Camera* camera = activeScene->getCamera()) {
                const float sensitivity = 0.3f;
                camera->rotate(0.0f, -delta * sensitivity);
            }
            });
    }

    auto mouseWheel = inputSystem.getAction("MouseWheel");
    if (mouseWheel) {
        mouseWheel->bindMouseAxisCallback([activeScene](float delta) {
            if (Camera* camera = activeScene->getCamera()) {
                const float zoomSpeed = 1.0f;
                camera->moveForward(delta * zoomSpeed * Time::getDeltaTime());
            }
            });
    }

    //auto switchToScene1 = inputSystem.getAction("SwitchToScene1");
    //if (switchToScene1) {
    //    switchToScene1->bindButtonCallback([](bool pressed) {
    //        if (pressed) {
    //            Selection::GameObjectSelect = nullptr;
    //            auto& sceneManager = SceneManager::getInstance();
    //            //sceneManager.setActiveScene("TestScene");
    //        }
    //        });
    //}

    //auto switchToScene2 = inputSystem.getAction("SwitchToScene2");
    //if (switchToScene2) {
    //    switchToScene2->bindButtonCallback([](bool pressed) {
    //        if (pressed) {
    //            Selection::GameObjectSelect = nullptr;
    //            auto& sceneManager = SceneManager::getInstance();
    //            sceneManager.setActiveScene("TexturedScene");
    //        }
    //        });
    //}

    //auto switchToScene3 = inputSystem.getAction("SwitchToScene3");
    //if (switchToScene3) {
    //    switchToScene3->bindButtonCallback([](bool pressed) {
    //        if (pressed) {
    //            Selection::GameObjectSelect = nullptr;
    //            auto& sceneManager = SceneManager::getInstance();
    //            sceneManager.setActiveScene("ModelScene");
    //        }
    //        });
    //}

    //auto switchToScene4 = inputSystem.getAction("SwitchToScene4");
    //if (switchToScene4) {
    //    switchToScene4->bindButtonCallback([](bool pressed) {
    //        if (pressed) {
    //            Selection::GameObjectSelect = nullptr;
    //            auto& sceneManager = SceneManager::getInstance();
    //            sceneManager.setActiveScene("Physics Test Scene");
    //        }
    //        });
    //}
}

int main() {
    // Registrar manejador de senales para cierre seguro
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Initialize RenderConfig singleton
    RenderConfig::initialize(1200, 800, 65.0f);
    RenderConfig& config = RenderConfig::getInstance();
    config.setAntialiasing(16);

    if (!config.initContext()) return -1;

    SDL_Window* window = config.getWindow();
    SDL_GLContext gl_context = SDL_GL_GetCurrentContext();


    // Inicializar FMOD
    auto& audioManager = AudioManager::getInstance();
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize FMOD" << std::endl;
        return -1;
    }

    DefaultShaders shaders;

    // Initialize SceneManager and create scenes
    auto& sceneManager = SceneManager::getInstance();

    // Get active scene (but don't check camera yet - scenes not initialized)
    Scene* activeScene = sceneManager.getActiveScene();
    if (!activeScene) {
        std::cerr << "Failed to get active scene from SceneManager" << std::endl;
        return -1;
    }

    std::cout << "Active scene: " << activeScene->getName() << " (not initialized yet)" << std::endl;

    // Create a temporary camera for RenderPipeline initialization
    auto tempCamera = std::make_unique<Camera>(45.0f, 1200.0f / 800.0f, 0.1f, 1000.0f);
    tempCamera->setPosition({ 0.0f, 5.0f, 10.0f });
    tempCamera->setTarget({ 0.0f, 0.0f, 0.0f });

    // Initialize RenderPipeline with temporary camera
    RenderPipeline pipeline(tempCamera.get(), &shaders);

    // Load materials from configuration file
    if (!pipeline.loadMaterialsFromConfig("config/materials_config.json")) {
        std::cerr << "Warning: Failed to load materials configurations" << std::endl;
    }
    else {
        std::cout << "Materials loaded successfully from configurations" << std::endl;
        pipeline.listMaterials();
    }

    // Setup resource management for scenes
    sceneManager.setRenderPipeline(&pipeline);

    // Initialize all scenes now that they have RenderPipeline access
    sceneManager.initializeAllScenes();

    // Now check if active scene has camera and update pipeline
    std::cout << "After initialization - Active scene has camera: " << (activeScene->getCamera() ? "YES" : "NO") << std::endl;

    if (!activeScene->getCamera()) {
        std::cerr << "Active scene '" << activeScene->getName() << "' has no camera after initialization" << std::endl;
        return -1;
    }

    // Replace temporary camera with scene's camera
    pipeline.setCamera(activeScene->getCamera());

    // Setup render pipeline with current scene objects
    sceneManager.setupRenderPipeline(pipeline);

    // Setup input system
    setupInputSystem(activeScene);

    activeScene->getCamera()->enableFramebuffer(true);
    activeScene->getCamera()->setFramebufferSize(800, 600);

    // Camera control variables
    bool mouseCaptured = false;

    // UI Demo control variable
    bool showUIDemo = true;

    // Banner de inicio
    bool showStartupBanner = true;
    float bannerStartTime = 0.0f;
    const float bannerDuration = 3.0f; // Duración del banner en segundos

    // Initialize Time system
    Time::init();

    // Event handling
    SDL_Event event;

    while (g_running) {
        Time::update();

        // Actualizar FMOD
        audioManager.update();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                g_running = false;
                break;
            }

            InputSystem::getInstance().processInput(event);
        }
        
        InputSystem::getInstance().update(Time::getDeltaTime());

        // Update scene
        sceneManager.update(Time::getDeltaTime());

        // Update active scene pointer if it changed
        Scene* newActiveScene = sceneManager.getActiveScene();
        if (newActiveScene != activeScene) {
            // Disable framebuffer on old camera
            if (activeScene && activeScene->getCamera()) {
                activeScene->getCamera()->enableFramebuffer(false);
                std::cout << "Framebuffer disabled for old scene: " << activeScene->getName() << std::endl;
            }

            // Update active scene
            activeScene = newActiveScene;

            // Update pipeline with new scene
            pipeline.setCamera(activeScene->getCamera());
            pipeline.clearGameObjects();
            pipeline.clearLights();
            sceneManager.setupRenderPipeline(pipeline);
            setupInputSystem(activeScene);

            // Enable framebuffer for new camera
            if (activeScene->getCamera()) {
                activeScene->getCamera()->enableFramebuffer(true);
                activeScene->getCamera()->setFramebufferSize(800, 600);
                std::cout << "Framebuffer enabled for scene: " << activeScene->getName() << std::endl;
            }
        }

        // Render scene to framebuffer (for ImGui viewport) and to main window
        pipeline.renderFrame();
        //pipeline.renderToScreen();

        SDL_GL_SwapWindow(window);

        // Update title
        std::stringstream title;
        title << "MantraxEngine - " << std::fixed << std::setprecision(1) << Time::getFPS() << " FPS";
        title << " | Objects: " << pipeline.getVisibleObjectsCount() << "/" << pipeline.getTotalObjectsCount();
        title << " | Scene: " << activeScene->getName();
        config.setWindowTitle(title.str());
    }

    // Cleanup
    try
    {
        SceneManager::getInstance().cleanupPhysics();
        RenderConfig::destroy();          // 2. Luego la ventana y OpenGL/SDL
        audioManager.destroy();           // 3. Luego audio y recursos propios
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during cleanup: " << e.what() << std::endl;
    }

    return 0;
}
