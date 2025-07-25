#include "Canvas.h"
#include <iostream>
#include <algorithm>
#include <utility>

Canvas::Canvas(SDL_Renderer* renderer) : renderer(renderer) {
    if (!renderer) {
        std::cerr << "Canvas::Canvas() - Warning: Renderer is null!" << std::endl;
    }
}

Canvas::~Canvas() {
    clearAllElements();
}

void Canvas::render() {
    if (!renderer) return;
    
    // Clear background if needed
    if (shouldClearBackground) {
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);
    }
    
    // Render all elements in order
    for (const auto& element : elements) {
        if (element && element->getVisible()) {
            element->render(renderer);
        }
    }
}

void Canvas::update(float deltaTime) {
    // Update all elements
    for (const auto& element : elements) {
        if (element) {
            element->update(deltaTime);
        }
    }
}

bool Canvas::handleEvent(const SDL_Event& event) {
    // Handle events in reverse order (top elements first)
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        const auto& element = *it;
        if (element && element->getVisible() && element->getEnabled()) {
            if (element->handleEvent(event)) {
                return true; // Event was handled, stop propagation
            }
        }
    }
    return false; // Event not handled
}

Button* Canvas::addButton(const glm::vec2& position, const glm::vec2& size, const std::string& text) {
    auto button = std::make_unique<Button>(position, size, text);
    button->setID(generateElementId());
    
    Button* buttonPtr = button.get();
    elements.emplace_back(std::move(button));
    
    std::cout << "Canvas::addButton() - Added button '" << text << "' with ID: " << buttonPtr->getID() << std::endl;
    return buttonPtr;
}

Text* Canvas::addText(const glm::vec2& position, const std::string& text) {
    auto textElement = std::make_unique<Text>(position, text);
    textElement->setID(generateElementId());
    
    Text* textPtr = textElement.get();
    elements.emplace_back(std::move(textElement));
    
    std::cout << "Canvas::addText() - Added text '" << text << "' with ID: " << textPtr->getID() << std::endl;
    return textPtr;
}

Image* Canvas::addImage(const glm::vec2& position, const glm::vec2& size, const std::string& imagePath) {
    auto image = std::make_unique<Image>(position, size);
    image->setID(generateElementId());
    
    // Load image if path provided
    if (!imagePath.empty() && renderer) {
        image->loadImage(renderer, imagePath);
    }
    
    Image* imagePtr = image.get();
    elements.emplace_back(std::move(image));
    
    std::cout << "Canvas::addImage() - Added image with ID: " << imagePtr->getID();
    if (!imagePath.empty()) {
        std::cout << " (Path: " << imagePath << ")";
    }
    std::cout << std::endl;
    
    return imagePtr;
}

UIElement* Canvas::getElementById(const std::string& id) {
    for (const auto& element : elements) {
        if (element && element->getID() == id) {
            return element.get();
        }
    }
    return nullptr;
}

Button* Canvas::getButtonById(const std::string& id) {
    return findElementByType<Button>(id);
}

Text* Canvas::getTextById(const std::string& id) {
    return findElementByType<Text>(id);
}

Image* Canvas::getImageById(const std::string& id) {
    return findElementByType<Image>(id);
}

void Canvas::removeElement(const std::string& id) {
    auto it = std::find_if(elements.begin(), elements.end(),
        [&id](const std::unique_ptr<UIElement>& element) {
            return element && element->getID() == id;
        });
    
    if (it != elements.end()) {
        std::cout << "Canvas::removeElement() - Removed element with ID: " << id << std::endl;
        elements.erase(it);
    }
}

void Canvas::removeElement(UIElement* element) {
    if (!element) return;
    
    auto it = std::find_if(elements.begin(), elements.end(),
        [element](const std::unique_ptr<UIElement>& elem) {
            return elem.get() == element;
        });
    
    if (it != elements.end()) {
        std::cout << "Canvas::removeElement() - Removed element with ID: " << element->getID() << std::endl;
        elements.erase(it);
    }
}

void Canvas::clearAllElements() {
    size_t count = elements.size();
    elements.clear();
    
    if (count > 0) {
        std::cout << "Canvas::clearAllElements() - Removed " << count << " elements" << std::endl;
    }
}

void Canvas::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    backgroundColor = {r, g, b, a};
}

std::string Canvas::generateElementId() {
    return "element_" + std::to_string(nextElementId++);
}

 