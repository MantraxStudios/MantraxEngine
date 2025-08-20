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
#include "../SimpleNodeEditor.h"

class RenderWindows
{
private:
    std::vector<std::unique_ptr<WindowBehaviour>> m_windows;
    SimpleNodeEditor editor;

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

        // editor.AddStartNode(1, ImVec2(50, 100));
        // editor.AddStartNode(2, ImVec2(10, 100));
        // editor.AddStringNode(3, ImVec2(50, 200), "Mi mensaje!");
        // editor.AddStringNode(4, ImVec2(50, 200), "Mi mensajeabc!");
        // editor.AddPrintNode(5, ImVec2(250, 150));

        // editor.AddCustomNode(25, ImVec2(100, 100), ImVec2(160, 80), "Persona",
        //                      [&](CustomNode &cn)
        //                      {
        //                          cn.RegisterPin("Edad", edad);
        //                          cn.RegisterPin("Velocidad", velocidad);
        //                      });

        // Crear nodos con valores editables
        auto startNode = editor.CreateStartNode(ImVec2(50, 100));
        auto stringNode = editor.CreateStringNode("Texto inicial", ImVec2(50, 200));
        auto intNode = editor.CreateIntNode(42, ImVec2(200, 150));
        auto floatNode = editor.CreateFloatNode(3.14f, ImVec2(350, 150));
        auto boolNode = editor.CreateBoolNode(true, ImVec2(500, 150));

        auto customNode = editor.CreateNode(
            "Mi Nodo",
            [](CustomNode *node)
            {
                std::string input = node->GetInputValue<std::string>(0);
                node->SetOutputValue<std::string>(0, "Processed: " + input);
                std::cout << "Procesando: " << input << std::endl;
            },
            true,                                // tiene pin de ejecución de entrada
            true,                                // tiene pin de ejecución de salida
            {{"Texto", std::string("default")}}, // pins de entrada
            {{"Resultado", std::string("")}},    // pins de salida
            ImVec2(100, 100)                     // posición
        );

        NodeConfig config("Mi Nodo");
        config.inputPins.push_back(PinConfig("Entrada1", Input, std::string("default")));
        config.outputPins.push_back(PinConfig("Salida1", Output));
        config.executeFunction = [](CustomNode *node)
        {
            std::string input = node->GetInputValue<std::string>(0);
            node->SetOutputValue<std::string>(0, "Processed: " + input);
        };

        auto miNodo = editor.CreateCustomNode(config, ImVec2(100, 100));

        CustomNode NewNodeTest = CustomNode();
        NewNodeTest.SetupNode(&editor);

        CustomNode NewNodeTest2 = CustomNode();
        NewNodeTest2.SetupNode(&editor);

        editor.customNodes.push_back(NewNodeTest);
        editor.customNodes.push_back(NewNodeTest2);
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

        // Ejecutar desde nodo de ejecución
        if (ImGui::Button("Ejecutar"))
        {
            editor.ExecuteGraph();
        }

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
