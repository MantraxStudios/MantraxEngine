#define SDL_MAIN_HANDLED
#define GLM_ENABLE_EXPERIMENTAL

#include "EUI/ImGuiLoader.h"
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

#include "EUI/EditorInfo.h"

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
#include "core/InputConfigLoader.h"

// ==== ImGui ====
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include "Windows/Selection.h"
#include "Windows/ProjectHub.h"


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
                const float sensitivity = 0.1f;
                camera->rotate(delta * sensitivity, 0.0f);
            }
        });
    }

    auto mouseLookY = inputSystem.getAction("MouseLookY");
    if (mouseLookY) {
        mouseLookY->bindMouseAxisCallback([activeScene](float delta) {
            if (Camera* camera = activeScene->getCamera()) {
                const float sensitivity = 0.1f;
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

    auto switchToScene1 = inputSystem.getAction("SwitchToScene1");
    if (switchToScene1) {
        switchToScene1->bindButtonCallback([](bool pressed) {
            if (pressed) {
                Selection::GameObjectSelect = nullptr;
                auto& sceneManager = SceneManager::getInstance();
                //sceneManager.setActiveScene("TestScene");
            }
        });
    }

    auto switchToScene2 = inputSystem.getAction("SwitchToScene2");
    if (switchToScene2) {
        switchToScene2->bindButtonCallback([](bool pressed) {
            if (pressed) {
                Selection::GameObjectSelect = nullptr;
                auto& sceneManager = SceneManager::getInstance();
                sceneManager.setActiveScene("TexturedScene");
            }
        });
    }

    auto switchToScene3 = inputSystem.getAction("SwitchToScene3");
    if (switchToScene3) {
        switchToScene3->bindButtonCallback([](bool pressed) {
            if (pressed) {
                Selection::GameObjectSelect = nullptr;
                auto& sceneManager = SceneManager::getInstance();
                sceneManager.setActiveScene("ModelScene");
            }
        });
    }

    auto switchToScene4 = inputSystem.getAction("SwitchToScene4");
    if (switchToScene4) {
        switchToScene4->bindButtonCallback([](bool pressed) {
            if (pressed) {
                Selection::GameObjectSelect = nullptr;
                auto& sceneManager = SceneManager::getInstance();
                sceneManager.setActiveScene("Physics Test Scene");
            }
        });
    }
}

int main() {
    // Registrar manejador de senales para cierre seguro
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    ProjectHub hub("projects");
    
    // Initialize RenderConfig singleton
    RenderConfig::initialize(1200, 800, 65.0f);
    RenderConfig& config = RenderConfig::getInstance();
    config.setAntialiasing(0);

    if (!config.initContext()) return -1;

    SDL_Window* window = config.getWindow();
    SDL_GLContext gl_context = SDL_GL_GetCurrentContext();

    ImGuiLoader::StartContext(window, gl_context);

    // Inicializar FMOD
    auto& audioManager = AudioManager::getInstance();
    if (!audioManager.initialize()) {
        std::cerr << "Failed to initialize FMOD" << std::endl;
        return -1;
    }

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

    EditorInfo::pipeline = &pipeline;
    
    // Load materials from configuration file
    if (!pipeline.loadMaterialsFromConfig("config/materials_config.json")) {
        std::cerr << "Warning: Failed to load materials configurations" << std::endl;
    } else {
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

    std::cout << "\n=== CONTROLES ===" << std::endl;
    std::cout << "WASD: Mover camara | Espacio/Shift: Subir/Bajar" << std::endl;
    std::cout << "Mouse: Mirar | Click Derecho: Capturar Mouse" << std::endl;
    std::cout << "1: Escena de Test | 2: Escena con Texturas | 3: ESCENA MASIVA (5000 objetos) | 4: Physics Test Scene" << std::endl;
    std::cout << "U: Toggle UI Demo Window (ImGui-based)" << std::endl;
    std::cout << "B: Mostrar Banner de Inicio" << std::endl;
    std::cout << "UI: Ventana 'UI System Demo' - Funciona en viewport y pantalla" << std::endl;
    std::cout << "Escape: Liberar Mouse" << std::endl;
    std::cout << "ADVERTENCIA: Escena 3 = Prueba de estrés con 5000 objetos" << std::endl;
    std::cout << "=================\n" << std::endl;

    // Enable camera framebuffer for viewport rendering
    activeScene->getCamera()->enableFramebuffer(true);
    activeScene->getCamera()->setFramebufferSize(800, 600);
    std::cout << "Initial framebuffer enabled for scene: " << activeScene->getName() << std::endl;

    // Camera control variables
    bool mouseCaptured = false;



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
            ImGuiLoader::ImGuiEventPoll(&event);

            if (event.type == SDL_QUIT) {
                g_running = false;
                break;
            }



            // Show startup banner with 'B' key
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_b) {
                showStartupBanner = true;
                bannerStartTime = 0.0f; // Reset timer
                std::cout << "Startup banner triggered" << std::endl;
                continue;
            }

            // Handle mouse capture toggle
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT && EditorInfo::IsHoveringScene) {
                mouseCaptured = true;
                SDL_SetRelativeMouseMode(SDL_TRUE);
                // Obtener el centro de la ventana
                int windowWidth, windowHeight;
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);
                SDL_WarpMouseInWindow(window, windowWidth / 2, windowHeight / 2);
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
                mouseCaptured = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_ENABLE);
            }

            // Handle escape to release mouse
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE && mouseCaptured) {
                mouseCaptured = false;
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_ENABLE);
            }

            // Process input through InputSystem only when mouse is captured
            if (mouseCaptured) {
                // Mantener el mouse en el centro después de cada movimiento
                if (event.type == SDL_MOUSEMOTION) {
                    int windowWidth, windowHeight;
                    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
                    SDL_WarpMouseInWindow(window, windowWidth / 2, windowHeight / 2);
                }
                InputSystem::getInstance().processInput(event);
            } else if (event.type != SDL_MOUSEMOTION && 
                      event.type != SDL_MOUSEWHEEL && 
                      event.type != SDL_MOUSEBUTTONDOWN && 
                      event.type != SDL_MOUSEBUTTONUP) {
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

        if (EditorInfo::SelectedProjectPath.empty()) {
            hub.render();
        }

        if (!EditorInfo::SelectedProjectPath.empty()) {
            // Banner de inicio
            if (showStartupBanner) {
                // Inicializar tiempo de inicio si es la primera vez
                if (bannerStartTime == 0.0f) {
                    bannerStartTime = Time::getTime();
                }

                // Calcular tiempo transcurrido
                float elapsedTime = Time::getTime() - bannerStartTime;

                // Si ha pasado el tiempo, ocultar el banner
                if (elapsedTime >= bannerDuration) {
                    showStartupBanner = false;
                }
                else {
                    // Configurar estilo para el banner
                    ImGui::SetNextWindowBgAlpha(0.9f);
                    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.3f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Always);

                    ImGui::Begin("MantraxEngine Banner", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

                    // Centrar el texto
                    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.3f);

                    // Título principal
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Usar la fuente más grande disponible
                    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("MANTRAX ENGINE").x) * 0.5f);
                    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "MANTRAX ENGINE");
                    ImGui::PopFont();

                    // Subtítulo
                    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.6f);
                    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Game Engine & Editor").x) * 0.5f);
                    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Game Engine & Editor");

                    // Barra de progreso
                    ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.8f);
                    ImGui::SetCursorPosX(50);
                    ImGui::ProgressBar(elapsedTime / bannerDuration, ImVec2(300, 20), "");

                    ImGui::End();
                }
            }

            //ImGui::Begin("Scene Info");
            //ImGui::Text("Objects: %zu", SceneManager::getInstance().getActiveScene()->getGameObjects().size());
            //ImGui::Text("Visible: %d/%d", pipeline.getVisibleObjectsCount(), pipeline.getTotalObjectsCount());
            //ImGui::Text("Scene: %s", activeScene->getName().c_str());
            //ImGui::Text("FPS: %.1f", Time::getFPS());
            //ImGui::End();

            RenderWindows::getInstance().RenderUI();
        }

        ImGuiLoader::SendToRender();
        SDL_GL_SwapWindow(window);
        
        // Update title
        std::stringstream title;
        title << "MantraxEngine - " << std::fixed << std::setprecision(1) << Time::getFPS() << " FPS";
        title << " | Objects: " << pipeline.getVisibleObjectsCount() << "/" << pipeline.getTotalObjectsCount();
        title << " | Scene: " << activeScene->getName();
        config.setWindowTitle(title.str());
    }

    // Cleanup
    audioManager.destroy();
    ImGuiLoader::CleanEUI();
    RenderConfig::destroy();
    
    return 0;
}
