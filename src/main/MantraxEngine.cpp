#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>

#include "../render/RenderConfig.h"
#include "../render/DefaultShaders.h"
#include "../render/Camera.h"

#include "../render/Material.h"
#include "../render/Texture.h"
#include "../render/Light.h"
#include "../components/GameObject.h"
#include "../components/Component.h"
#include "../render/RenderPipeline.h"
#include "../components/SceneManager.h"
#include "../components/TestScene.h"
#include "../components/TexturedScene.h"
#include "../input/InputSystem.h"
#include "../core/Time.h"
#include "../core/AudioManager.h"

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
#include <exception>

// Variable global para manejo seguro del cierre
volatile bool g_running = true;

void signalHandler(int signal) {
    std::cout << "\nSignal de cierre recibida (" << signal << "). Cerrando de forma segura..." << std::endl;
    g_running = false;
}

void setupInputSystem(Scene* activeScene) {
    auto& inputSystem = InputSystem::getInstance();
    
    // Movimiento de la cámara (WASD)
    auto moveAction = inputSystem.registerAction("Move", InputType::Vector2D);
    moveAction->addKeyBinding(SDLK_w, true, 1);  // Forward
    moveAction->addKeyBinding(SDLK_s, false, 1); // Backward
    moveAction->addKeyBinding(SDLK_d, true, 0);  // Right
    moveAction->addKeyBinding(SDLK_a, false, 0); // Left
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

    // Movimiento vertical de la cámara (Space/Shift)
    auto verticalMoveAction = inputSystem.registerAction("VerticalMove", InputType::Value);
    verticalMoveAction->addKeyBinding(SDLK_SPACE, true);
    verticalMoveAction->addKeyBinding(SDLK_LSHIFT, false);
    verticalMoveAction->bindValueCallback([activeScene](float value) {
        if (Camera* camera = activeScene->getCamera()) {
            const float speed = 1.0f;
            camera->moveUp(value * speed * Time::getDeltaTime());
        }
    });

    // Mouse Look
    auto mouseLookX = inputSystem.registerAction("MouseLookX", InputType::MouseAxis);
    mouseLookX->addMouseAxisBinding(MouseAxisType::X);
    mouseLookX->bindMouseAxisCallback([activeScene](float delta) {
        if (Camera* camera = activeScene->getCamera()) {
            const float sensitivity = 0.1f;
            camera->rotate(delta * sensitivity, 0.0f);
        }
    });

    auto mouseLookY = inputSystem.registerAction("MouseLookY", InputType::MouseAxis);
    mouseLookY->addMouseAxisBinding(MouseAxisType::Y);
    mouseLookY->bindMouseAxisCallback([activeScene](float delta) {
        if (Camera* camera = activeScene->getCamera()) {
            const float sensitivity = 0.1f;
            camera->rotate(0.0f, -delta * sensitivity);
        }
    });

    // Mouse Buttons
    auto mouseRight = inputSystem.registerAction("MouseRight", InputType::MouseButton);
    mouseRight->addMouseButtonBinding(SDL_BUTTON_RIGHT);

    // Mouse Wheel
    auto mouseWheel = inputSystem.registerAction("MouseWheel", InputType::MouseAxis);
    mouseWheel->addMouseAxisBinding(MouseAxisType::ScrollWheel);
    mouseWheel->bindMouseAxisCallback([activeScene](float delta) {
        if (Camera* camera = activeScene->getCamera()) {
            const float zoomSpeed = 1.0f;
            camera->moveForward(delta * zoomSpeed * Time::getDeltaTime());
        }
    });

    // Cambio de escenas
    auto switchToScene1 = inputSystem.registerAction("SwitchToScene1", InputType::Button);
    switchToScene1->addKeyBinding(SDLK_1);
    switchToScene1->bindButtonCallback([](bool pressed) {
        if (pressed) {
            auto& sceneManager = SceneManager::getInstance();
            sceneManager.setActiveScene("TestScene");
        }
    });

    auto switchToScene2 = inputSystem.registerAction("SwitchToScene2", InputType::Button);
    switchToScene2->addKeyBinding(SDLK_2);
    switchToScene2->bindButtonCallback([](bool pressed) {
        if (pressed) {
            auto& sceneManager = SceneManager::getInstance();
            sceneManager.setActiveScene("TexturedScene");
        }
    });
}

int main() {
    // Registrar manejador de señales para cierre seguro
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Inicializar RenderConfig usando los métodos estáticos
    RenderConfig::initialize(1200, 800, 45.0f);
    auto& config = RenderConfig::getInstance();
    if (!config.initContext()) return -1;

    DefaultShaders shaders;
    
    // Initialize SceneManager and create scenes
    auto& sceneManager = SceneManager::getInstance();
    sceneManager.addScene(std::make_unique<TestScene>());
    sceneManager.addScene(std::make_unique<TexturedScene>());

    // Get camera from active scene for render pipeline
    Scene* activeScene = sceneManager.getActiveScene();
    if (!activeScene || !activeScene->getCamera()) {
        std::cerr << "Failed to initialize scene or camera" << std::endl;
        return -1;
    }

    RenderPipeline pipeline(activeScene->getCamera(), &shaders);
    sceneManager.setupRenderPipeline(pipeline);

    // Setup input system
    setupInputSystem(activeScene);

    // Inicializar FMOD
    auto& audioManager = AudioManager::getInstance();
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize FMOD" << std::endl;
        return -1;
    }

    std::cout << "\n=== CONTROLES ===" << std::endl;
    std::cout << "WASD: Mover camara | Espacio/Shift: Subir/Bajar" << std::endl;
    std::cout << "Mouse: Mirar | Click Derecho: Capturar Mouse" << std::endl;
    std::cout << "1: Escena de Test | 2: Escena con Texturas" << std::endl;
    std::cout << "Escape: Liberar Mouse" << std::endl;
    std::cout << "=================\n" << std::endl;

    // Camera control variables
    bool mouseCaptured = false;

    // Initialize Time system
    Time::init();

    // Event handling
    SDL_Event event;

    while (g_running) {
        Time::update();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                g_running = false;
                break;
            }

            // Handle mouse capture toggle
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                auto mouseRight = InputSystem::getInstance().getAction("MouseRight");
                if (mouseRight) {
                    mouseCaptured = !mouseCaptured;
                    SDL_SetRelativeMouseMode(mouseCaptured ? SDL_TRUE : SDL_FALSE);
                }
            }

            // Handle escape to release mouse
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                mouseCaptured = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }

            // Process input through InputSystem only when mouse is captured
            if (mouseCaptured || 
                (event.type != SDL_MOUSEMOTION && 
                 event.type != SDL_MOUSEWHEEL && 
                 event.type != SDL_MOUSEBUTTONDOWN && 
                 event.type != SDL_MOUSEBUTTONUP)) {
                InputSystem::getInstance().processInput(event);
            }
        }

        // Update input system
        InputSystem::getInstance().update(Time::getDeltaTime());

        // Update scene
        sceneManager.update(Time::getDeltaTime());
        
        // Update active scene pointer if it changed
        Scene* newActiveScene = sceneManager.getActiveScene();
        if (newActiveScene != activeScene) {
            activeScene = newActiveScene;
            pipeline.setCamera(activeScene->getCamera());
            pipeline.clearGameObjects();
            pipeline.clearLights();
            sceneManager.setupRenderPipeline(pipeline);
            setupInputSystem(activeScene);
        }
        
        // Render
        pipeline.renderFrame();
        SDL_GL_SwapWindow(config.getWindow());
        
        // Update title
        std::stringstream title;
        title << "MantraxEngine - " << std::fixed << std::setprecision(1) << Time::getFPS() << " FPS";
        title << " | Objects: " << pipeline.getVisibleObjectsCount() << "/" << pipeline.getTotalObjectsCount();
        title << " | Scene: " << activeScene->getName();
        config.setWindowTitle(title.str());

        // Actualizar FMOD
        audioManager.update();
    }

    // Cleanup
    audioManager.destroy();
    RenderConfig::destroy();
    return 0;
}
