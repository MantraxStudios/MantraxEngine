#pragma once
#include <SDL.h>
#include <string>
#include "../core/CoreExporter.h"

// Forward declaration
class MANTRAXCORE_API RenderConfig;

// Friend function declarations
void initializeRenderConfig(int width, int height, float fov);
RenderConfig& getRenderConfigInstance();
void destroyRenderConfig();

class MANTRAXCORE_API RenderConfig {
public:
    // Friend functions to access private members
    friend void initializeRenderConfig(int width, int height, float fov);
    friend RenderConfig& getRenderConfigInstance();
    friend void destroyRenderConfig();
    
    // Singleton methods (delegated to friend functions)
    static RenderConfig& getInstance() { return getRenderConfigInstance(); }
    static void initialize(int width, int height, float fov) { initializeRenderConfig(width, height, fov); }
    static void destroy() { destroyRenderConfig(); }
    
    // Delete copy constructor and assignment operator
    RenderConfig(const RenderConfig&) = delete;
    RenderConfig& operator=(const RenderConfig&) = delete;

    bool initContext(); 
    void setWindowTitle(const std::string& title);
    void resizeViewport(int width, int height);
    
    // Antialiasing methods
    void setAntialiasing(int samples);
    int getAntialiasing() const { return antialiasingSamples; }
    bool isAntialiasingEnabled() const { return antialiasingSamples > 1; }

    int getWidth() const;
    int getHeight() const;
    float getFOV() const;
    float getAspectRatio() const;

    SDL_Window* getWindow() const;
    SDL_GLContext getContext() const { return glContext; }
    SDL_Renderer* getRenderer();  // For UI system

private:
    // Private constructor for singleton
    RenderConfig(int width, int height, float fov);
    ~RenderConfig();

    int screenWidth;
    int screenHeight;
    float fov;
    int antialiasingSamples;  // Default: 8

    SDL_Window* window;
    SDL_GLContext glContext;
    SDL_Renderer* renderer;
    
    static RenderConfig* instance;
};
