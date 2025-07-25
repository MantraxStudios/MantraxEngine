#include "Image.h"
#include <iostream>
#include <algorithm>

Image::Image(const glm::vec2& position, const glm::vec2& size)
    : UIElement(UIElementType::IMAGE, position, size) {
    calculateRenderRect();
}

Image::Image(const glm::vec2& position, const glm::vec2& size, const std::string& imagePath)
    : UIElement(UIElementType::IMAGE, position, size), imagePath(imagePath) {
    calculateRenderRect();
}

Image::~Image() {
    unloadImage();
}

void Image::render(SDL_Renderer* renderer) {
    if (!isVisible) return;
    
    if (texture) {
        // Set texture color modulation (tint)
        SDL_SetTextureColorMod(texture, tint.r, tint.g, tint.b);
        SDL_SetTextureAlphaMod(texture, tint.a);
        
        // Render the texture
        SDL_Rect sourceRect = calculateSourceRect();
        SDL_RenderCopy(renderer, texture, &sourceRect, &renderRect);
    } else {
        // Render placeholder if no image loaded
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_Rect rect = getRect();
        SDL_RenderFillRect(renderer, &rect);
        
        // Draw an X to indicate missing image
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
        SDL_RenderDrawLine(renderer, rect.x + rect.w, rect.y, rect.x, rect.y + rect.h);
    }
}

void Image::update(float deltaTime) {
    // Update any image animations here
}

bool Image::loadImage(SDL_Renderer* renderer, const std::string& path) {
    // Unload existing image
    unloadImage();
    
    // Load surface from file (BMP format for simplicity)
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "Image::loadImage() - Failed to load image: " << path << " - " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Create texture from surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Image::loadImage() - Failed to create texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }
    
    // Store image information
    imageSize.x = static_cast<float>(surface->w);
    imageSize.y = static_cast<float>(surface->h);
    imagePath = path;
    
    SDL_FreeSurface(surface);
    
    // Recalculate render rectangle
    calculateRenderRect();
    
    std::cout << "Image::loadImage() - Loaded image: " << path << " (" << imageSize.x << "x" << imageSize.y << ")" << std::endl;
    return true;
}

void Image::unloadImage() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    imageSize = {0, 0};
    imagePath.clear();
}

void Image::setTint(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    tint = {r, g, b, a};
}

void Image::setOpacity(Uint8 alpha) {
    tint.a = alpha;
}

void Image::calculateRenderRect() {
    SDL_Rect elementRect = getRect();
    
    switch (scaleMode) {
        case ImageScaleMode::STRETCH:
            renderRect = elementRect;
            break;
            
        case ImageScaleMode::KEEP_ASPECT: {
            if (imageSize.x == 0 || imageSize.y == 0) {
                renderRect = elementRect;
                break;
            }
            
            float imageAspect = imageSize.x / imageSize.y;
            float elementAspect = size.x / size.y;
            
            if (imageAspect > elementAspect) {
                // Image is wider - fit width
                renderRect.w = elementRect.w;
                renderRect.h = static_cast<int>(elementRect.w / imageAspect);
                renderRect.x = elementRect.x;
                renderRect.y = elementRect.y + (elementRect.h - renderRect.h) / 2;
            } else {
                // Image is taller - fit height
                renderRect.h = elementRect.h;
                renderRect.w = static_cast<int>(elementRect.h * imageAspect);
                renderRect.y = elementRect.y;
                renderRect.x = elementRect.x + (elementRect.w - renderRect.w) / 2;
            }
            break;
        }
        
        case ImageScaleMode::CROP: {
            if (imageSize.x == 0 || imageSize.y == 0) {
                renderRect = elementRect;
                break;
            }
            
            float imageAspect = imageSize.x / imageSize.y;
            float elementAspect = size.x / size.y;
            
            if (imageAspect > elementAspect) {
                // Image is wider - fit height, crop width
                renderRect.h = elementRect.h;
                renderRect.w = static_cast<int>(elementRect.h * imageAspect);
                renderRect.y = elementRect.y;
                renderRect.x = elementRect.x + (elementRect.w - renderRect.w) / 2;
            } else {
                // Image is taller - fit width, crop height
                renderRect.w = elementRect.w;
                renderRect.h = static_cast<int>(elementRect.w / imageAspect);
                renderRect.x = elementRect.x;
                renderRect.y = elementRect.y + (elementRect.h - renderRect.h) / 2;
            }
            break;
        }
        
        case ImageScaleMode::ORIGINAL:
            renderRect.x = elementRect.x;
            renderRect.y = elementRect.y;
            renderRect.w = static_cast<int>(imageSize.x);
            renderRect.h = static_cast<int>(imageSize.y);
            break;
    }
}

SDL_Rect Image::calculateSourceRect() const {
    if (scaleMode == ImageScaleMode::CROP && texture) {
        // For crop mode, we need to calculate which part of the source image to use
        float imageAspect = imageSize.x / imageSize.y;
        float elementAspect = size.x / size.y;
        
        SDL_Rect sourceRect;
        
        if (imageAspect > elementAspect) {
            // Image is wider - use full height, crop width
            sourceRect.h = static_cast<int>(imageSize.y);
            sourceRect.w = static_cast<int>(imageSize.y * elementAspect);
            sourceRect.y = 0;
            sourceRect.x = static_cast<int>((imageSize.x - sourceRect.w) / 2);
        } else {
            // Image is taller - use full width, crop height
            sourceRect.w = static_cast<int>(imageSize.x);
            sourceRect.h = static_cast<int>(imageSize.x / elementAspect);
            sourceRect.x = 0;
            sourceRect.y = static_cast<int>((imageSize.y - sourceRect.h) / 2);
        }
        
        return sourceRect;
    } else {
        // Use entire source image
        return SDL_Rect{0, 0, static_cast<int>(imageSize.x), static_cast<int>(imageSize.y)};
    }
} 