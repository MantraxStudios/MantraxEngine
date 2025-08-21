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
#include "TileEditor.h"
#include "InputEditor.h"
#include "CanvasManager.h"
#include <MantraxNodes/MNodeEditor.h>

class MNodeEditor;

class RenderWindows
{
private:
    std::vector<std::unique_ptr<WindowBehaviour>> m_windows;
    MNodeEditor editor;

public:
    RenderWindows()
    {
        m_windows.push_back(std::make_unique<Hierarchy>());
        m_windows.push_back(std::make_unique<SceneView>());
        m_windows.push_back(std::make_unique<Inspector>());
        m_windows.push_back(std::make_unique<MainBar>());
        m_windows.push_back(std::make_unique<ContentBrowser>());
        m_windows.push_back(std::make_unique<MaterialEditor>());
        m_windows.push_back(std::make_unique<AnimatorEditor>());
        m_windows.push_back(std::make_unique<TileEditor>());
        m_windows.push_back(std::make_unique<InputEditor>());
        m_windows.push_back(std::make_unique<CanvasManager>());
    }

    // Delete copy constructor and assignment operator
    RenderWindows(const RenderWindows &) = delete;
    RenderWindows &operator=(const RenderWindows &) = delete;

    static RenderWindows &getInstance()
    {
        static RenderWindows instance;
        return instance;
    }

    void RenderUI()
    {
        ImGui::Begin("Node Editor");
        editor.Draw();
        ImGui::End();

        for (auto &window : m_windows)
        {
            if (window && window->isOpen)
            {
                window->OnRenderGUI();
            }
        }
    }

    template <typename T>
    T *GetWindow()
    {
        for (auto &window : m_windows)
        {
            if (auto *typed = dynamic_cast<T *>(window.get()))
            {
                return typed;
            }
        }
        // Si no se encuentra, agregamos una nueva ventana del tipo solicitado
        auto newWindow = std::make_unique<T>();
        T *ptr = newWindow.get();
        m_windows.push_back(std::move(newWindow));
        return ptr;
    }
};
