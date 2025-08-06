#include <imgui/imgui.h>
#include "CanvasManager.h"
#include <iostream>
#include <filesystem>
#include <render/RenderPipeline.h>
#include <components/SceneManager.h>

void CanvasManager::OnRenderGUI() {
    ImGui::Begin("Canvas Editor", &isOpen);
    
    if (ImGui::Button("Make Canvas")) {
        Canvas2D* canvas = SceneManager::getInstance().getActiveScene()->getRenderPipeline()->addCanvas(800, 600);

        if (canvas) {
            std::cout << "Canvas created successfully!" << std::endl;
        } else {
            std::cerr << "Failed to create canvas!" << std::endl;
        }
    }

    if (ImGui::Button("Make New Text")) {
        Canvas2D* canvas = SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0);
        if (canvas) {
            canvas->MakeNewText("Hello From CanvasManager")->UIAnchor = Anchor::BottomLeft;
        } else {
            std::cerr << "No canvas available at index 0!" << std::endl;
        }
    }

    if (SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements.size() > 0) {
        if (ImGui::Button("<")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::TopLeft;
        }

        ImGui::SameLine();
        if (ImGui::Button("[]")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::TopCenter;
        }

        ImGui::SameLine();
        if (ImGui::Button(">")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::TopRight;
        }

        // Segunda fila
        if (ImGui::Button("|<")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::CenterLeft;
        }

        ImGui::SameLine();
        if (ImGui::Button("O")) { // Center
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::Center;
        }

        ImGui::SameLine();
        if (ImGui::Button(">|")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::CenterRight;
        }

        // Tercera fila
        if (ImGui::Button("v<")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::BottomLeft;
        }

        ImGui::SameLine();
        if (ImGui::Button("v[]")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::BottomCenter;
        }

        ImGui::SameLine();
        if (ImGui::Button("v>")) {
            SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0]->UIAnchor = Anchor::BottomRight;
        }
    }

    
    ImGui::End();
}

const std::string& CanvasManager::getName() const {
    static const std::string name = "Canvas Manager";
    return name;
}