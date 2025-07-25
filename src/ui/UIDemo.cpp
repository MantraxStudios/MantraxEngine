#include "UIDemo.h"
#include <iostream>

UIDemo::UIDemo() {
}

UIDemo::~UIDemo() {
    cleanup();
}

bool UIDemo::initialize(SDL_Renderer* renderer) {
    if (!renderer) {
        std::cerr << "UIDemo::initialize() - Renderer is null!" << std::endl;
        return false;
    }

    // Create canvas
    canvas = std::make_unique<Canvas>(renderer);
    if (!canvas) {
        std::cerr << "UIDemo::initialize() - Failed to create Canvas!" << std::endl;
        return false;
    }

    // Setup UI elements
    setupUI();

    std::cout << "UIDemo::initialize() - UI Demo initialized successfully" << std::endl;
    return true;
}

void UIDemo::cleanup() {
    if (canvas) {
        canvas->clearAllElements();
        canvas.reset();
    }
}

void UIDemo::render() {
    if (showDemo && canvas) {
        canvas->render();
    }
}

void UIDemo::update(float deltaTime) {
    if (showDemo && canvas) {
        canvas->update(deltaTime);
    }
}

bool UIDemo::handleEvent(const SDL_Event& event) {
    if (!showDemo || !canvas) {
        return false;
    }

    // Let canvas handle the event first
    return canvas->handleEvent(event);
}

void UIDemo::setupUI() {
    setupTexts();
    setupButtons();
    setupImages();
}

void UIDemo::setupButtons() {
    // Test Button 1
    testButton1 = canvas->addButton(glm::vec2(50, 100), glm::vec2(150, 40), "Click Me!");
    testButton1->setBackgroundColor(70, 130, 180);
    testButton1->setHoverColor(100, 149, 237);
    testButton1->setPressedColor(50, 100, 150);
    testButton1->setOnClick([this]() { onButton1Click(); });

    // Test Button 2
    testButton2 = canvas->addButton(glm::vec2(220, 100), glm::vec2(150, 40), "Button 2");
    testButton2->setBackgroundColor(220, 20, 60);
    testButton2->setHoverColor(255, 69, 104);
    testButton2->setPressedColor(180, 15, 45);
    testButton2->setOnClick([this]() { onButton2Click(); });

    // Toggle Button
    toggleButton = canvas->addButton(glm::vec2(400, 100), glm::vec2(120, 40), "Hide UI");
    toggleButton->setBackgroundColor(34, 139, 34);
    toggleButton->setHoverColor(50, 205, 50);
    toggleButton->setPressedColor(20, 100, 20);
    toggleButton->setOnClick([this]() { onToggleClick(); });
}

void UIDemo::setupTexts() {
    // Title
    titleText = canvas->addText(glm::vec2(50, 30), "MantraxEngine UI Canvas Demo");
    titleText->setTextColor(255, 255, 255);
    titleText->setFontSize(24);

    // Info text
    infoText = canvas->addText(glm::vec2(50, 60), "Click the buttons to test UI functionality");
    infoText->setTextColor(200, 200, 200);
    infoText->setFontSize(16);
}

void UIDemo::setupImages() {
    // Add a placeholder image
    testImage = canvas->addImage(glm::vec2(50, 160), glm::vec2(100, 100));
    testImage->setTint(100, 100, 255, 180); // Blue tint with transparency
    
    // Try to load an actual image (will show placeholder if not found)
    // testImage->loadImage(renderer, "path/to/image.bmp");
}

void UIDemo::onButton1Click() {
    clickCount++;
    std::cout << "UIDemo::onButton1Click() - Button 1 clicked! Count: " << clickCount << std::endl;
    
    // Update info text
    std::string newText = "Button 1 clicked " + std::to_string(clickCount) + " times";
    infoText->setText(newText);
    
    // Change button color based on click count
    if (clickCount % 3 == 0) {
        testButton1->setBackgroundColor(255, 140, 0); // Orange
    } else if (clickCount % 3 == 1) {
        testButton1->setBackgroundColor(70, 130, 180); // Steel blue
    } else {
        testButton1->setBackgroundColor(138, 43, 226); // Blue violet
    }
}

void UIDemo::onButton2Click() {
    std::cout << "UIDemo::onButton2Click() - Button 2 clicked!" << std::endl;
    
    // Toggle image visibility
    if (testImage) {
        testImage->setVisible(!testImage->getVisible());
        testButton2->setText(testImage->getVisible() ? "Hide Image" : "Show Image");
    }
}

void UIDemo::onToggleClick() {
    std::cout << "UIDemo::onToggleClick() - Toggle clicked!" << std::endl;
    
    // This will be handled externally - just update button text
    toggleButton->setText(showDemo ? "Show UI" : "Hide UI");
} 