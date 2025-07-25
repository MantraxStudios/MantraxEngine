#pragma once
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <memory>
#include "../core/CoreExporter.h"
#include "../render/RenderConfig.h"
#include "../render/DefaultShaders.h"
#include "../components/Scene.h"
#include "../components/SceneManager.h"
#include "../render/RenderPipeline.h"
#include "../input/InputSystem.h"
#include "../core/Time.h"

extern "C" {
    class MANTRAXCORE_API CoreWrapper {
    public:
        CoreWrapper();
        ~CoreWrapper();
        
        bool Initialize(int width = 1200, int height = 800, float fov = 45.0f);
        void Update();
        void Shutdown();
        bool IsRunning() const { return m_running; }
        void SetRunning(bool running) { m_running = running; }
        
    private:
        void SetupInputSystem(Scene* activeScene);
        void HandleEvents();
        void UpdateTitle();
        
        bool m_running;
        bool m_mouseCaptured;
        std::unique_ptr<RenderConfig> m_config;
        std::unique_ptr<DefaultShaders> m_shaders;
        std::unique_ptr<RenderPipeline> m_pipeline;
        Scene* m_activeScene;
    };
}