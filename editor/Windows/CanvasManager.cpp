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
            canvas->MakeNewText("Hello From CanvasManager")->setAnchor(Anchor::BottomLeft);
        } else {
            std::cerr << "No canvas available at index 0!" << std::endl;
        }
    }

    if (ImGui::Button("<")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::TopLeft);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("[]")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::TopCenter);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(">")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::TopRight);
        }
    }

    if (ImGui::Button("|<")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::CenterLeft);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("O")) { // Center
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::Center);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(">|")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::CenterRight);
        }
    }

    // Tercera fila
    if (ImGui::Button("v<")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::BottomLeft);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("v[]")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::BottomCenter);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("v>")) {
        if (UIText* textElement = dynamic_cast<UIText*>(SceneManager::getInstance().getActiveScene()->getRenderPipeline()->getCanvas(0)->RenderElements[0])) {
            textElement->setAnchor(Anchor::BottomRight);
        }
    }
    ImGui::End();
}

const std::string& CanvasManager::getName() const {
    static const std::string name = "Canvas Manager";
    return name;
}