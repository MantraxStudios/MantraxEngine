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
    if (glContext) SDL_GL_DeleteContext(glContext);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

bool RenderConfig::initContext() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // Configure antialiasing
    if (antialiasingSamples > 1) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasingSamples);
        std::cout << "Antialiasing enabled with " << antialiasingSamples << " samples" << std::endl;
    } else {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        std::cout << "Antialiasing disabled" << std::endl;
    }

    window = SDL_CreateWindow("MantraxEngine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        return false;
    }

    // SDL_Renderer disabled - causes conflicts with OpenGL
    // renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    renderer = nullptr;

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Make the OpenGL context current
    if (SDL_GL_MakeCurrent(window, glContext) != 0) {
        std::cerr << "SDL_GL_MakeCurrent error: " << SDL_GetError() << std::endl;
        return false;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init error" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable/disable antialiasing in OpenGL
    if (antialiasingSamples > 1) {
        glEnable(GL_MULTISAMPLE);
        std::cout << "OpenGL multisample antialiasing enabled" << std::endl;
    } else {
        glDisable(GL_MULTISAMPLE);
        std::cout << "OpenGL multisample antialiasing disabled" << std::endl;
    }
    
    return true;
}

void RenderConfig::setWindowTitle(const std::string& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
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
        SDL_SetWindowSize(window, width, height);
    }

    // Update OpenGL viewport
    glViewport(0, 0, width, height);
}

void RenderConfig::setAntialiasing(int samples) {
    if (samples < 0 || samples > 16) {
        std::cerr << "Invalid antialiasing samples: " << samples << ". Must be between 0 and 16." << std::endl;
        return;
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
