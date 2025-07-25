#pragma once
#include <SDL.h>
#include <string>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API RenderConfig {
public:
    RenderConfig(int width, int height, float fov);
    ~RenderConfig();

    bool initContext(); 
    void setWindowTitle(const std::string& title);

    int getWidth() const;
    int getHeight() const;
    float getFOV() const;
    float getAspectRatio() const;

    SDL_Window* getWindow() const;
    SDL_GLContext getContext() const { return glContext; }
    SDL_Renderer* getRenderer();  // For UI system

private:
    int screenWidth;
    int screenHeight;
    float fov;

    SDL_Window* window;
    SDL_GLContext glContext;
    SDL_Renderer* renderer;
};
