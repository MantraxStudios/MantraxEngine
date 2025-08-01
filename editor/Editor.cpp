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
#include "ui/UIManager.h"
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
#include "SceneSaver.h"


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

    SceneSaver::MakeNewScene("Empty Scene");

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

            //// Show startup banner with 'B' key
            //if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_b) {
            //    showStartupBanner = true;
            //    bannerStartTime = 0.0f; // Reset timer
            //    std::cout << "Startup banner triggered" << std::endl;
            //    continue;
            //}

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
            }
            else if (event.type != SDL_MOUSEMOTION &&
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
                    ImGuiIO& io = ImGui::GetIO();
                    ImVec2 display_size = io.DisplaySize;

                    // Forzar sin padding
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

                    ImGui::SetNextWindowBgAlpha(1.0f); // Fondo opaco
                    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(display_size, ImGuiCond_Always);

                    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;

                    ImGui::Begin("MantraxEngine", nullptr, flags);

                    // === DISEÑO MEJORADO DEL BANNER ===
                    
                    // Fondo con gradiente
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 window_pos = ImGui::GetWindowPos();
                    ImVec2 window_size = ImGui::GetWindowSize();
                    
                                         // Gradiente de fondo usando colores del tema ImGui
                     ImVec2 gradient_start = window_pos;
                     ImVec2 gradient_end = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
                     draw_list->AddRectFilledMultiColor(
                         gradient_start, gradient_end,
                         IM_COL32(59, 61, 64, 255),   // Top-left: darkBg (0.23f, 0.24f, 0.25f)
                         IM_COL32(71, 74, 77, 255),   // Top-right: mediumBg (0.28f, 0.29f, 0.30f)
                         IM_COL32(82, 84, 87, 255),   // Bottom-right: lightBg (0.32f, 0.33f, 0.34f)
                         IM_COL32(66, 68, 71, 255)    // Bottom-left: mezcla de colores
                     );

                    // Líneas decorativas animadas
                    float time = Time::getTime();
                    float line_alpha = 0.3f + 0.2f * sinf(time * 2.0f);
                    
                                         // Líneas horizontales decorativas usando accentBlue del tema
                     for (int i = 0; i < 5; i++) {
                         float y_pos = window_pos.y + (window_size.y * 0.2f) + (i * 80.0f);
                         float line_width = window_size.x * 0.8f;
                         float x_offset = sinf(time * 1.5f + i) * 50.0f;
                         
                         draw_list->AddLine(
                             ImVec2(window_pos.x + (window_size.x - line_width) * 0.5f + x_offset, y_pos),
                             ImVec2(window_pos.x + (window_size.x + line_width) * 0.5f + x_offset, y_pos),
                             IM_COL32(66, 150, 250, (int)(255 * line_alpha))  // accentBlue del tema
                         );
                     }

                                         // Círculos decorativos usando accentOrange del tema
                     for (int i = 0; i < 8; i++) {
                         float angle = time * 0.5f + i * 0.785f;
                         float radius = 30.0f + 20.0f * sinf(time * 1.2f + i);
                         float x = window_pos.x + 100.0f + cosf(angle) * radius;
                         float y = window_pos.y + 100.0f + sinf(angle) * radius;
                         
                         draw_list->AddCircleFilled(
                             ImVec2(x, y), 3.0f,
                             IM_COL32(255, 161, 0, (int)(255 * (0.3f + 0.4f * sinf(time * 2.0f + i))))  // accentOrange del tema
                         );
                     }

                    // Logo/Icono (círculo con engranaje)
                    float logo_size = 120.0f;
                    ImVec2 logo_pos(
                        window_pos.x + window_size.x * 0.5f - logo_size * 0.5f,
                        window_pos.y + window_size.y * 0.25f - logo_size * 0.5f
                    );
                    
                                         // Círculo exterior del logo usando accentBlue del tema
                     draw_list->AddCircle(
                         ImVec2(logo_pos.x + logo_size * 0.5f, logo_pos.y + logo_size * 0.5f),
                         logo_size * 0.5f,
                         IM_COL32(66, 150, 250, 255), 0, 3.0f  // accentBlue del tema
                     );
                    
                    // Engranaje interior
                    for (int i = 0; i < 8; i++) {
                        float angle = i * 0.785f + time * 0.3f;
                        float inner_radius = logo_size * 0.3f;
                        float outer_radius = logo_size * 0.45f;
                        
                        ImVec2 inner_point(
                            logo_pos.x + logo_size * 0.5f + cosf(angle) * inner_radius,
                            logo_pos.y + logo_size * 0.5f + sinf(angle) * inner_radius
                        );
                        ImVec2 outer_point(
                            logo_pos.x + logo_size * 0.5f + cosf(angle) * outer_radius,
                            logo_pos.y + logo_size * 0.5f + sinf(angle) * outer_radius
                        );
                        
                                                 draw_list->AddLine(inner_point, outer_point, IM_COL32(255, 161, 0, 255), 2.0f);  // accentOrange del tema
                    }

                    // Título principal con efecto de sombra
                    const char* title_text = "MANTRAX ENGINE";
                    ImFont* bigFont = io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : io.FontDefault;
                    ImGui::PushFont(bigFont);

                    ImVec2 title_size = ImGui::CalcTextSize(title_text);
                    ImVec2 title_pos(
                        (display_size.x - title_size.x) * 0.5f,
                        (display_size.y - title_size.y) * 0.45f
                    );
                    
                                         // Sombra del texto usando textWhite del tema
                     ImGui::SetCursorPos(ImVec2(title_pos.x + 3, title_pos.y + 3));
                     ImGui::TextColored(ImVec4(0, 0, 0, 0.5f), title_text);
                     
                     // Texto principal usando textWhite del tema
                     ImGui::SetCursorPos(title_pos);
                     ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), title_text);  // textWhite del tema

                    ImGui::PopFont();

                    // Subtítulo con animación
                    const char* subtitle_text = "Game Development Engine";
                    ImVec2 subtitle_size = ImGui::CalcTextSize(subtitle_text);
                    ImVec2 subtitle_pos(
                        (display_size.x - subtitle_size.x) * 0.5f,
                        title_pos.y + title_size.y + 20.0f
                    );
                    
                                         float subtitle_alpha = 0.7f + 0.3f * sinf(time * 3.0f);
                     ImGui::SetCursorPos(subtitle_pos);
                     ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, subtitle_alpha), subtitle_text);  // textGray del tema

                    // Texto de carga con animación de puntos
                    const char* loading_base = "Loading Project";
                    std::string loading_text = loading_base;
                    int dots_count = (int)(time * 2.0f) % 4;
                    for (int i = 0; i < dots_count; i++) {
                        loading_text += ".";
                    }
                    
                    ImVec2 loading_size = ImGui::CalcTextSize(loading_text.c_str());
                    ImVec2 loading_pos(
                        (display_size.x - loading_size.x) * 0.5f,
                        subtitle_pos.y + subtitle_size.y + 40.0f
                    );
                                         ImGui::SetCursorPos(loading_pos);
                     ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), loading_text.c_str());  // textGray del tema

                    // Barra de progreso mejorada
                    float progress = elapsedTime / bannerDuration;
                    if (progress > 1.0f) progress = 1.0f;
                    if (progress < 0.0f) progress = 0.0f;

                    float progress_width = display_size.x * 0.4f;
                    float progress_height = 8.0f;
                    ImVec2 progress_pos(
                        (display_size.x - progress_width) * 0.5f,
                        loading_pos.y + loading_size.y + 30.0f
                    );
                    
                                         // Fondo de la barra de progreso usando colores del tema
                     draw_list->AddRectFilled(
                         ImVec2(progress_pos.x, progress_pos.y),
                         ImVec2(progress_pos.x + progress_width, progress_pos.y + progress_height),
                         IM_COL32(41, 43, 46, 255),  // FrameBg del tema
                         4.0f
                     );
                    
                    // Barra de progreso con gradiente
                    if (progress > 0.0f) {
                        ImVec2 progress_end(
                            progress_pos.x + progress_width * progress,
                            progress_pos.y + progress_height
                        );
                        
                                                 draw_list->AddRectFilledMultiColor(
                             ImVec2(progress_pos.x, progress_pos.y),
                             progress_end,
                             IM_COL32(66, 150, 250, 255),   // accentBlue del tema
                             IM_COL32(255, 161, 0, 255),     // accentOrange del tema
                             IM_COL32(255, 161, 0, 255),     // accentOrange del tema
                             IM_COL32(66, 150, 250, 255)     // accentBlue del tema
                         );
                    }

                    // Información adicional en la parte inferior
                    const char* version_text = "Version 1.0.0";
                    ImVec2 version_size = ImGui::CalcTextSize(version_text);
                    ImVec2 version_pos(
                        (display_size.x - version_size.x) * 0.5f,
                        progress_pos.y + progress_height + 5.0f
                    );
                                         ImGui::SetCursorPos(version_pos);
                     ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 0.8f), version_text);  // textGray del tema

                                         // Efecto de partículas flotantes usando accentOrange del tema
                     for (int i = 0; i < 15; i++) {
                         float particle_x = fmodf(sinf(time * 0.5f + i * 0.7f) * window_size.x * 0.8f + window_size.x * 0.1f, window_size.x);
                         float particle_y = fmodf(cosf(time * 0.3f + i * 0.5f) * window_size.y * 0.6f + window_size.y * 0.2f, window_size.y);
                         float particle_alpha = 0.3f + 0.4f * sinf(time * 2.0f + i);
                         
                         draw_list->AddCircleFilled(
                             ImVec2(window_pos.x + particle_x, window_pos.y + particle_y),
                             2.0f,
                             IM_COL32(255, 161, 0, (int)(255 * particle_alpha))  // accentOrange del tema
                         );
                     }

                    ImGui::End();
                    ImGui::PopStyleVar();
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
    try
    {
        SceneManager::getInstance().cleanupPhysics();
        ImGuiLoader::CleanEUI();          // 1. Primero ImGui (debe tener el contexto GL activo)
        RenderConfig::destroy();          // 2. Luego la ventana y OpenGL/SDL
        audioManager.destroy();           // 3. Luego audio y recursos propios
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception during cleanup: " << e.what() << std::endl;
    }

    return 0;
}
