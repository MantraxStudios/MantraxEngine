#define SDL_MAIN_HANDLED
#define GLM_ENABLE_EXPERIMENTAL

#include "EUI/ImGuiLoader.h"
#include <SDL.h>
#include <GL/glew.h>

#include "render/RenderConfig.h"
#include "render/DefaultShaders.h"
#include "render/Camera.h"
#include "render/NativeGeometry.h"
#include "render/Material.h"
#include "render/Texture.h"
#include "render/Light.h"
#include "components/GameObject.h"
#include "components/Component.h"
#include "render/RenderPipeline.h"
#include "components/SceneManager.h"
#include "components/TestScene.h"
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
#include <exception>
#include "Windows/RenderWindows.h"

// ==== ImGui ====
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include "Windows/Selection.h"

// Variable global para manejo seguro del cierre
volatile bool g_running = true;

void signalHandler(int signal) {
    std::cout << "\nSe�al de cierre recibida (" << signal << "). Cerrando de forma segura..." << std::endl;
    g_running = false;
}

void setupInputSystem(Scene* activeScene) {
    auto& inputSystem = InputSystem::getInstance();
    
    // Movimiento de la c�mara (WASD)
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

    // Movimiento vertical de la c�mara (Space/Shift)
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
            Selection::GameObjectSelect = nullptr;
            auto& sceneManager = SceneManager::getInstance();
            sceneManager.setActiveScene("TestScene");
        }
    });

    auto switchToScene2 = inputSystem.registerAction("SwitchToScene2", InputType::Button);
    switchToScene2->addKeyBinding(SDLK_2);
    switchToScene2->bindButtonCallback([](bool pressed) {
        if (pressed) {
            Selection::GameObjectSelect = nullptr;
            auto& sceneManager = SceneManager::getInstance();
            sceneManager.setActiveScene("TexturedScene");
        }
    });

    auto switchToScene3 = inputSystem.registerAction("SwitchToScene3", InputType::Button);
    switchToScene3->addKeyBinding(SDLK_3);
    switchToScene3->bindButtonCallback([](bool pressed) {
        if (pressed) {
            Selection::GameObjectSelect = nullptr;
            auto& sceneManager = SceneManager::getInstance();
            sceneManager.setActiveScene("ModelScene");
        }
    });
}

int main() {
    // Registrar manejador de se�ales para cierre seguro
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Initialize RenderConfig singleton
    RenderConfig::initialize(1200, 800, 65.0f);
    RenderConfig& config = RenderConfig::getInstance();
    config.setAntialiasing(16);

    RenderWindows* _WindowsRender = new RenderWindows();
    if (!config.initContext()) return -1;

    SDL_Window* window = config.getWindow();
    SDL_GLContext gl_context = SDL_GL_GetCurrentContext();

    ImGuiLoader::StartContext(window, gl_context);

    // UI Manager disabled due to SDL_Renderer conflicts
    // Will use ImGui-based UI instead
    std::cout << "Using ImGui-based UI system (SDL_Renderer disabled)" << std::endl;

    DefaultShaders shaders;
    
    // Initialize SceneManager and create scenes
    auto& sceneManager = SceneManager::getInstance();
    
    sceneManager.addScene(std::make_unique<TestScene>());
    sceneManager.addScene(std::make_unique<TexturedScene>());
    sceneManager.addScene(std::make_unique<ModelScene>());

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

    std::cout << "\n=== CONTROLES ===" << std::endl;
    std::cout << "WASD: Mover c�mara | Espacio/Shift: Subir/Bajar" << std::endl;
    std::cout << "Mouse: Mirar | Click Derecho: Capturar Mouse" << std::endl;
    std::cout << "1: Escena de Test | 2: Escena con Texturas | 3: ⚡ ESCENA MASIVA (5000 objetos)" << std::endl;
    std::cout << "U: Toggle UI Demo Window (ImGui-based)" << std::endl;
    std::cout << "UI: Ventana 'UI System Demo' - Funciona en viewport y pantalla" << std::endl;
    std::cout << "Escape: Liberar Mouse" << std::endl;
    std::cout << "⚠️  ADVERTENCIA: Escena 3 = Prueba de estrés con 5000 objetos" << std::endl;
    std::cout << "=================\n" << std::endl;

    // Enable camera framebuffer for viewport rendering
    activeScene->getCamera()->enableFramebuffer(true);
    activeScene->getCamera()->setFramebufferSize(800, 600);
    std::cout << "Initial framebuffer enabled for scene: " << activeScene->getName() << std::endl;

    // Camera control variables
    bool mouseCaptured = false;

    // UI Demo control variable
    bool showUIDemo = true;

    // Initialize Time system
    Time::init();

    // Event handling
    SDL_Event event;

    while (g_running) {
        Time::update();

        while (SDL_PollEvent(&event)) {
            ImGuiLoader::ImGuiEventPoll(&event);

            if (event.type == SDL_QUIT) {
                g_running = false;
                break;
            }

            // Toggle UI demo window with 'U' key  
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_u) {
                showUIDemo = !showUIDemo;
                std::cout << "UI Demo toggled - Visible: " << (showUIDemo ? "YES" : "NO") << std::endl;
                continue;
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
        
        // ==== ImGui Frame Begin ====
        ImGuiLoader::MakeFrame();

        //ImGui::Begin("Scene Info");
        //ImGui::Text("Objects: %zu", SceneManager::getInstance().getActiveScene()->getGameObjects().size());
        //ImGui::Text("Visible: %d/%d", pipeline.getVisibleObjectsCount(), pipeline.getTotalObjectsCount());
        //ImGui::Text("Scene: %s", activeScene->getName().c_str());
        //ImGui::Text("FPS: %.1f", Time::getFPS());
        //ImGui::End();

        _WindowsRender->RenderUI();

        ImGuiLoader::SendToRender();
        SDL_GL_SwapWindow(window);
        
        // Update title
        std::stringstream title;
        title << "MantraxEngine - " << std::fixed << std::setprecision(1) << Time::getFPS() << " FPS";
        title << " | Objects: " << pipeline.getVisibleObjectsCount() << "/" << pipeline.getTotalObjectsCount();
        title << " | Scene: " << activeScene->getName();
        config.setWindowTitle(title.str());
    }

    ImGuiLoader::CleanEUI();
    
    // Clean up RenderConfig singleton
    RenderConfig::destroy();
    
    return 0;
}
