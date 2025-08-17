#include "RenderConfig.h"
#include <GL/glew.h>
#include <iostream>
#include <stdexcept>

// Static instance pointer
RenderConfig* RenderConfig::instance = nullptr;

// Friend function implementations
RenderConfig& getRenderConfigInstance() {
    if (RenderConfig::instance == nullptr) {
        throw std::runtime_error("RenderConfig not initialized. Call initialize() first.");
    }
    return *RenderConfig::instance;
}

void initializeRenderConfig(int width, int height, float fov) {
    if (RenderConfig::instance != nullptr) {
        delete RenderConfig::instance;
    }
    RenderConfig::instance = new RenderConfig(width, height, fov);
}

void destroyRenderConfig() {
    if (RenderConfig::instance != nullptr) {
        delete RenderConfig::instance;
        RenderConfig::instance = nullptr;
    }
}

RenderConfig::RenderConfig(int width, int height, float fov)
    : screenWidth(width), screenHeight(height), fov(fov), antialiasingSamples(8),
    window(nullptr), glContext(nullptr), renderer(nullptr) {
}

RenderConfig::~RenderConfig() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (glContext) SDL_GL_DestroyContext(glContext);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

bool RenderConfig::initContext() {
    // Inicializar SDL3 - sintaxis correcta para SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Atributos de OpenGL - sintaxis SDL3
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) {
        std::cerr << "Error setting OpenGL major version: " << SDL_GetError() << std::endl;
        return false;
    }
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) {
        std::cerr << "Error setting OpenGL minor version: " << SDL_GetError() << std::endl;
        return false;
    }
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
        std::cerr << "Error setting OpenGL profile: " << SDL_GetError() << std::endl;
        return false;
    }

    // Configurar buffer de profundidad
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Antialiasing
    if (antialiasingSamples > 1) {
        if (!SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1)) {
            std::cerr << "Warning: Could not set multisample buffers: " << SDL_GetError() << std::endl;
        }
        if (!SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasingSamples)) {
            std::cerr << "Warning: Could not set multisample samples: " << SDL_GetError() << std::endl;
        }
        std::cout << "Antialiasing enabled with " << antialiasingSamples << " samples\n";
    }
    else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        std::cout << "Antialiasing disabled\n";
    }

    // Crear ventana SDL3 - sintaxis corregida
    window = SDL_CreateWindow("MantraxEngine",
        screenWidth,
        screenHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Crear contexto OpenGL
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    // Activar contexto - en SDL3 devuelve bool
    if (!SDL_GL_MakeCurrent(window, glContext)) {
        std::cerr << "SDL_GL_MakeCurrent error: " << SDL_GetError() << std::endl;
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
        return false;
    }

    // Sincronizar con VSync - en SDL3 devuelve bool
    if (!SDL_GL_SetSwapInterval(1)) {
        std::cerr << "Warning: VSync not supported: " << SDL_GetError() << std::endl;
    }

    // Inicializar GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "GLEW init error: " << glewGetErrorString(glewError) << std::endl;
        return false;
    }

    // Limpiar cualquier error de OpenGL generado por GLEW
    glGetError();

    // Verificar versión de OpenGL
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Configuración OpenGL
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (antialiasingSamples > 1) {
        glEnable(GL_MULTISAMPLE);
    }
    else {
        glDisable(GL_MULTISAMPLE);
    }

    // Establecer viewport inicial
    glViewport(0, 0, screenWidth, screenHeight);

    // Verificar errores de OpenGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error during initialization: " << error << std::endl;
        return false;
    }

    std::cout << "OpenGL context created successfully!" << std::endl;
    return true;
}

void RenderConfig::setWindowTitle(const std::string& title) {
    if (window) {
        if (!SDL_SetWindowTitle(window, title.c_str())) {
            std::cerr << "Error setting window title: " << SDL_GetError() << std::endl;
        }
    }
}

void RenderConfig::resizeViewport(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid viewport dimensions: " << width << "x" << height << std::endl;
        return;
    }

    // Update internal dimensions
    screenWidth = width;
    screenHeight = height;

    // Resize SDL window if it exists
    if (window) {
        if (!SDL_SetWindowSize(window, width, height)) {
            std::cerr << "Error resizing window: " << SDL_GetError() << std::endl;
        }
    }

    // Update OpenGL viewport
    glViewport(0, 0, width, height);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error setting viewport: " << error << std::endl;
    }
}

void RenderConfig::setAntialiasing(int samples) {
    if (samples < 0 || samples > 16) {
        std::cerr << "Invalid antialiasing samples: " << samples << ". Must be between 0 and 16." << std::endl;
        return;
    }

    // Validar que sea potencia de 2
    if (samples > 1 && (samples & (samples - 1)) != 0) {
        std::cerr << "Warning: Antialiasing samples should be a power of 2 (2, 4, 8, 16)" << std::endl;
    }

    antialiasingSamples = samples;

    // If context is already created, we need to recreate it with new antialiasing settings
    if (glContext) {
        std::cout << "Antialiasing changed to " << samples << " samples. Context will be recreated on next initialization." << std::endl;
    }
}

int RenderConfig::getWidth() const { return screenWidth; }
int RenderConfig::getHeight() const { return screenHeight; }
float RenderConfig::getFOV() const { return fov; }
float RenderConfig::getAspectRatio() const { return static_cast<float>(screenWidth) / screenHeight; }
SDL_Window* RenderConfig::getWindow() const { return window; }
SDL_Renderer* RenderConfig::getRenderer() { return renderer; }