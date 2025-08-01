#pragma once
#include <vector>
#include <memory>
#include "Hierarchy.h"
#include "SceneView.h"
#include "Inspector.h"
#include "Gizmos.h"
#include "MainBar.h"
#include "ContentBrowser.h"
#include "MaterialEditor.h"
#include "AnimatorEditor.h"


class RenderWindows {
private:
    std::vector<std::unique_ptr<WindowBehaviour>> m_windows;

public:
    RenderWindows() {
        m_windows.push_back(std::make_unique<Hierarchy>());
        m_windows.push_back(std::make_unique<SceneView>());
        m_windows.push_back(std::make_unique<Inspector>());
        m_windows.push_back(std::make_unique<Gizmos>());
        m_windows.push_back(std::make_unique<MainBar>());
        m_windows.push_back(std::make_unique<ContentBrowser>());
        m_windows.push_back(std::make_unique<MaterialEditor>());
        m_windows.push_back(std::make_unique<AnimatorEditor>());

    }
    
    // Delete copy constructor and assignment operator
    RenderWindows(const RenderWindows&) = delete;
    RenderWindows& operator=(const RenderWindows&) = delete;

    static RenderWindows& getInstance() {
        static RenderWindows instance;
        return instance;
    }

    void RenderUI() {
        for (auto& window : m_windows) {
            if (window && window->isOpen) {
                window->OnRenderGUI();
            }
        }
    }

    template<typename T>
    T* GetWindow() {
        for (auto& window : m_windows) {
            if (auto* typed = dynamic_cast<T*>(window.get())) {
                return typed;
            }
        }
        // Si no se encuentra, agregamos una nueva ventana del tipo solicitado
        auto newWindow = std::make_unique<T>();
        T* ptr = newWindow.get();
        m_windows.push_back(std::move(newWindow));
        return ptr;
    }
};
