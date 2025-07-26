#pragma once
#include "CoreWrapper.h"
#include "../components/TestScene.h"
#include "../components/TexturedScene.h"
#include <sstream>
#include <iomanip>
#include <signal.h>

CoreWrapper::CoreWrapper() 
    : m_running(true)
    , m_mouseCaptured(false)
    , m_activeScene(nullptr)
{
}

CoreWrapper::~CoreWrapper() {
    Shutdown();
}

bool CoreWrapper::Initialize(int width, int height, float fov) {
    // Initialize render config singleton
    RenderConfig::initialize(width, height, fov);
    m_config = &RenderConfig::getInstance();
    if (!m_config->initContext()) return false;

    // Initialize shaders
    m_shaders = std::make_unique<DefaultShaders>();
    
    // Initialize SceneManager and create scenes
    auto& sceneManager = SceneManager::getInstance();
    sceneManager.addScene(std::make_unique<TestScene>());
    sceneManager.addScene(std::make_unique<TexturedScene>());

    // Get camera from active scene for render pipeline
    m_activeScene = sceneManager.getActiveScene();
    if (!m_activeScene || !m_activeScene->getCamera()) {
        std::cerr << "Failed to initialize scene or camera" << std::endl;
        return false;
    }

    // Initialize render pipeline
    m_pipeline = std::make_unique<RenderPipeline>(m_activeScene->getCamera(), m_shaders.get());
    
    // Load materials from configuration file
    if (!m_pipeline->loadMaterialsFromConfig("config/materials_config.json")) {
        std::cerr << "Warning: Failed to load materials configuration" << std::endl;
    } else {
        std::cout << "Materials loaded successfully from configuration" << std::endl;
        m_pipeline->listMaterials();
    }
    
    sceneManager.setupRenderPipeline(*m_pipeline);

    // Setup input system
    SetupInputSystem(m_activeScene);

    // Print controls
    std::cout << "\n=== CONTROLES ===" << std::endl;
    std::cout << "WASD: Mover cámara | Espacio/Shift: Subir/Bajar" << std::endl;
    std::cout << "Mouse: Mirar | Click Derecho: Capturar Mouse" << std::endl;
    std::cout << "1: Escena de Test | 2: Escena con Texturas" << std::endl;
    std::cout << "Escape: Liberar Mouse" << std::endl;
    std::cout << "=================\n" << std::endl;

    // Initialize Time system
    Time::init();

    return true;
}

void CoreWrapper::SetupInputSystem(Scene* activeScene) {
    auto& inputSystem = InputSystem::getInstance();
    
    // Limpiar acciones existentes para evitar duplicados
    inputSystem.clearActions();
    
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

    // Mouse Wheel
    auto mouseWheel = inputSystem.registerAction("MouseWheel", InputType::MouseAxis);
    mouseWheel->addMouseAxisBinding(MouseAxisType::ScrollWheel);
    mouseWheel->bindMouseAxisCallback([activeScene](float delta) {
        if (Camera* camera = activeScene->getCamera()) {
            const float zoomSpeed = 1.0f;
            camera->moveForward(delta * zoomSpeed * Time::getDeltaTime());
        }
    });

    // Mouse Buttons
    auto mouseRight = inputSystem.registerAction("MouseRight", InputType::MouseButton);
    mouseRight->addMouseButtonBinding(SDL_BUTTON_RIGHT);

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

void CoreWrapper::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
            break;
        }

        // Handle mouse capture toggle
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
            auto mouseRight = InputSystem::getInstance().getAction("MouseRight");
            if (mouseRight) {
                m_mouseCaptured = !m_mouseCaptured;
                SDL_SetRelativeMouseMode(m_mouseCaptured ? SDL_TRUE : SDL_FALSE);
            }
        }

        // Handle escape to release mouse
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            m_mouseCaptured = false;
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }

        // Process input through InputSystem only when mouse is captured
        if (m_mouseCaptured || 
            (event.type != SDL_MOUSEMOTION && 
             event.type != SDL_MOUSEWHEEL && 
             event.type != SDL_MOUSEBUTTONDOWN && 
             event.type != SDL_MOUSEBUTTONUP)) {
            InputSystem::getInstance().processInput(event);
        }
    }
}

void CoreWrapper::UpdateTitle() {
    std::stringstream title;
    title << "MantraxEngine - " << std::fixed << std::setprecision(1) << Time::getFPS() << " FPS";
    title << " | Objects: " << m_pipeline->getVisibleObjectsCount() << "/" << m_pipeline->getTotalObjectsCount();
    title << " | Scene: " << m_activeScene->getName();
    m_config->setWindowTitle(title.str());
}

void CoreWrapper::Update() {
    Time::update();
    
    // Handle events
    HandleEvents();

    // Update input system
    InputSystem::getInstance().update(Time::getDeltaTime());

    // Update scene
    auto& sceneManager = SceneManager::getInstance();
    sceneManager.update(Time::getDeltaTime());
    
    // Update active scene pointer if it changed
    Scene* newActiveScene = sceneManager.getActiveScene();
    if (newActiveScene != m_activeScene) {
        m_activeScene = newActiveScene;
        m_pipeline->setCamera(m_activeScene->getCamera());
        m_pipeline->clearGameObjects();
        m_pipeline->clearLights();
        sceneManager.setupRenderPipeline(*m_pipeline);
        SetupInputSystem(m_activeScene);
    }
    
    // Render
    m_pipeline->renderFrame();
    SDL_GL_SwapWindow(m_config->getWindow());
    
    // Update title
    UpdateTitle();
}

void CoreWrapper::Shutdown() {
    m_pipeline.reset();
    m_shaders.reset();
    // m_config is now a raw pointer to singleton, no need to reset
    m_config = nullptr;
}