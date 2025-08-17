#pragma once
#include <chrono>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API Time {
public:
    static void init() {
        startTime = std::chrono::steady_clock::now();
        lastFrameTime = startTime;
        deltaTime = 0.0f;
        time = 0.0f;
        frameCount = 0;
        fps = 0.0f;
    }

    static void update() {
        auto currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        time = std::chrono::duration<float>(currentTime - startTime).count();
        lastFrameTime = currentTime;

        // Update FPS
        frameCount++;
        fpsAccumulator += deltaTime;
        if (fpsAccumulator >= 1.0f) {
            fps = static_cast<float>(frameCount) / fpsAccumulator;
            frameCount = 0;
            fpsAccumulator = 0.0f;
        }

        // Clamp deltaTime to prevent huge jumps
        if (deltaTime > maxDeltaTime) {
            deltaTime = maxDeltaTime;
        }
    }

    // Getters
    static float getDeltaTime() { return deltaTime; }
    static float getTime() { return time; }
    static float getUnscaledDeltaTime() { return deltaTime / timeScale; }
    static float getFPS() { return fps; }
    static float getTimeScale() { return timeScale; }
    static void setTimeScale(float scale) { timeScale = scale; }
    static float getFixedDeltaTime() { return fixedDeltaTime; }
    static void setFixedDeltaTime(float dt) { fixedDeltaTime = dt; }
    static float getMaxDeltaTime() { return maxDeltaTime; }
    static void setMaxDeltaTime(float max) { maxDeltaTime = max; }

private:
    static inline std::chrono::steady_clock::time_point startTime;
    static inline std::chrono::steady_clock::time_point lastFrameTime;
    static inline float deltaTime = 0.0f;      // Tiempo entre frames
    static inline float time = 0.0f;           // Tiempo total desde el inicio
    static inline float timeScale = 1.0f;      // Escala de tiempo (1.0 = normal)
    static inline float fixedDeltaTime = 0.02f; // Para física (50 Hz por defecto)
    static inline float maxDeltaTime = 0.1f;   // Límite para prevenir saltos grandes
    static inline uint32_t frameCount = 0;     // Contador de frames para FPS
    static inline float fps = 0.0f;            // Frames por segundo actual
    static inline float fpsAccumulator = 0.0f; // Acumulador para cálculo de FPS
}; 