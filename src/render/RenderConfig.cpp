#include "RenderConfig.h"
#include <GL/glew.h>
#include <iostream>

RenderConfig::RenderConfig(int width, int height, float fov)
    : screenWidth(width), screenHeight(height), fov(fov),
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

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init error" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

void RenderConfig::setWindowTitle(const std::string& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

int RenderConfig::getWidth() const { return screenWidth; }
int RenderConfig::getHeight() const { return screenHeight; }
float RenderConfig::getFOV() const { return fov; }
float RenderConfig::getAspectRatio() const { return static_cast<float>(screenWidth) / screenHeight; }
SDL_Window* RenderConfig::getWindow() const { return window; }
SDL_Renderer* RenderConfig::getRenderer() { return renderer; }
