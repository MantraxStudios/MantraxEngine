#pragma once

#include "UIElement.h"
#include <string>

enum class ImageScaleMode {
    STRETCH,      // Stretch to fit the element size
    KEEP_ASPECT,  // Keep aspect ratio, may leave empty space
    CROP,         // Keep aspect ratio, may crop image
    ORIGINAL      // Use original image size
};

class MANTRAXCORE_API Image : public UIElement {
public:
    Image(const glm::vec2& position, const glm::vec2& size);
    Image(const glm::vec2& position, const glm::vec2& size, const std::string& imagePath);
    virtual ~Image();

    // Core UIElement overrides
    void render(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;

    // Image-specific methods
    bool loadImage(SDL_Renderer* renderer, const std::string& imagePath);
    void unloadImage();
    bool hasImage() const { return texture != nullptr; }
    
    // Appearance
    void setTint(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void setScaleMode(ImageScaleMode mode) { scaleMode = mode; calculateRenderRect(); }
    void setOpacity(Uint8 alpha);
    
    // Information
    glm::vec2 getImageSize() const { return imageSize; }
    std::string getImagePath() const { return imagePath; }

private:
    SDL_Texture* texture = nullptr;
    std::string imagePath;
    glm::vec2 imageSize = {0, 0};
    ImageScaleMode scaleMode = ImageScaleMode::STRETCH;
    
    SDL_Color tint = {255, 255, 255, 255};
    
    // Calculated render rectangle
    SDL_Rect renderRect;
    
    // Helper methods
    void calculateRenderRect();
    SDL_Rect calculateSourceRect() const;
}; 