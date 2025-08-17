#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <iostream>
#include "../UIBehaviour.h"
#include "../../core/CoreExporter.h"

class MANTRAXCORE_API UIText : public UIBehaviour
{
public:
	std::string Text = "New Text";

	glm::vec2 Position;
    glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::mat4 ortho;

    FT_Face currentFont;

    float width;
    float height;
    
    // Store original reference dimensions and position for proper scaling
    float originalWidth = 0.0f;
    float originalHeight = 0.0f;
    glm::vec2 relativePosition; // Position as percentage of canvas size (0.0-1.0)
    bool useRelativePositioning = false;

    std::map<char, GLuint> characterTextures;

    void update() override {
        // Only try to draw if we have a valid font and OpenGL context
        if (currentFont && textShaderProgram != 0 && width > 0 && height > 0) {
            // Calculate text size for anchoring
            auto [textWidth, textHeight] = calculateTextSize(Text);
            
            // Get anchored position using text dimensions and current anchor
            auto [finalX, finalY] = getAnchoredPosition(Position.x, Position.y, static_cast<float>(textWidth), static_cast<float>(textHeight), UIAnchor);
            
            // Draw text at the calculated position
            drawTextInternal(Text, finalX, finalY, Color);
        }
    }

    void setAnchor(Anchor newAnchor) {
        UIAnchor = newAnchor;
        std::cout << "UIText: Anchor changed to " << static_cast<int>(newAnchor) << std::endl;
        // Force immediate update to reposition text
        update();
    }
    
    // Method to handle canvas resize and maintain relative positioning
    void onCanvasResize(float newWidth, float newHeight) {
        // If this is the first time setting size, store as original reference
        if (originalWidth == 0.0f && originalHeight == 0.0f) {
            originalWidth = newWidth;
            originalHeight = newHeight;
            // Convert absolute position to relative position (percentage)
            if (newWidth > 0 && newHeight > 0) {
                relativePosition.x = Position.x / newWidth;
                relativePosition.y = Position.y / newHeight;
                useRelativePositioning = true;
            }
        } else if (useRelativePositioning && newWidth > 0 && newHeight > 0) {
            // Update position based on new canvas size and relative position
            Position.x = relativePosition.x * newWidth;
            Position.y = relativePosition.y * newHeight;
        }
        
        width = newWidth;
        height = newHeight;
        
        std::cout << "UIText: Canvas resized to " << newWidth << "x" << newHeight 
                  << ", updated position to (" << Position.x << ", " << Position.y << ")" << std::endl;
    }
    
    // Method to set position and enable relative positioning
    void setPosition(float x, float y) {
        Position.x = x;
        Position.y = y;
        
        // Update relative position if we have valid canvas dimensions
        if (width > 0 && height > 0) {
            relativePosition.x = x / width;
            relativePosition.y = y / height;
            useRelativePositioning = true;
            
            // Store current dimensions as reference if not set
            if (originalWidth == 0.0f && originalHeight == 0.0f) {
                originalWidth = width;
                originalHeight = height;
            }
        }
    }
    
    // Safe position setting for different anchors
    void setPositionForAnchor(float x, float y, Anchor anchor) {
        // Adjust position based on anchor to prevent off-screen issues
        float safeX = x;
        float safeY = y;
        
        // For top anchors, ensure Y is not too small
        if (anchor == Anchor::TopLeft || anchor == Anchor::TopCenter || anchor == Anchor::TopRight) {
            safeY = std::max(5.0f, y);
        }
        
        // For right anchors, ensure there's enough space
        if (anchor == Anchor::TopRight || anchor == Anchor::CenterRight || anchor == Anchor::BottomRight) {
            safeX = std::max(5.0f, x);
        }
        
        setPosition(safeX, safeY);
        UIAnchor = anchor;
        
        std::cout << "UIText: Set position (" << safeX << ", " << safeY << ") for anchor " << static_cast<int>(anchor) << std::endl;
    }

    std::pair<int, int> drawText(const std::string& msg, float x, float y, glm::vec4 color) {
        // Calculate text size for all anchors that need it
        auto [textWidth, textHeight] = calculateTextSize(msg);

        // Get anchored position using text dimensions
        auto [finalX, finalY] = getAnchoredPosition(x, y, static_cast<float>(textWidth), static_cast<float>(textHeight), UIAnchor);
        return drawTextInternal(msg, finalX, finalY, color);
    }

    std::pair<int, int> drawTextInternal(const std::string& msg, float x, float y, glm::vec4 color) {
        if (!currentFont) {
            std::cerr << "UIText: No font loaded!" << std::endl;
            return { 0,0 };
        }

        float currentX = x;
        float maxHeight = 0;

        for (char c : msg) {
            // Load character glyph
            if (FT_Load_Char(currentFont, c, FT_LOAD_RENDER)) {
                std::cerr << "Could not load character: " << c << std::endl;
                continue;
            }

            FT_GlyphSlot glyph = currentFont->glyph;

            // Generate texture for this character if not already cached
            GLuint texture = getCharacterTexture(c);
            if (texture == 0) {
                texture = generateCharacterTexture(glyph);
                characterTextures[c] = texture;
            }

            // Calculate position
            float xpos = currentX + glyph->bitmap_left;
            float ypos = y - glyph->bitmap_top;

            float w = glyph->bitmap.width;
            float h = glyph->bitmap.rows;

            // Draw character
            drawCharacter(texture, xpos, ypos, w, h, color);

            // Advance cursor
            currentX += glyph->advance.x >> 6; // Bitshift by 6 to get value in pixels

            // Track max height
            if (h > maxHeight) maxHeight = h;
        }

        return { static_cast<int>(currentX - x), static_cast<int>(maxHeight) };
    }

    void drawCharacter(GLuint texture, float x, float y, float w, float h, glm::vec4 color) {
        if (w <= 0 || h <= 0) return;

        glUseProgram(textShaderProgram);
        
        // Don't change viewport - let the canvas handle it
        // glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        // Configure blending for transparent text
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Disable depth testing for 2D text
        glDisable(GL_DEPTH_TEST);

        // Use the ortho matrix from the canvas
        glUniformMatrix4fv(textOrthoLoc, 1, GL_FALSE, &ortho[0][0]);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
        model = glm::scale(model, glm::vec3(w, h, 1.0f));
        glUniformMatrix4fv(textModelLoc, 1, GL_FALSE, &model[0][0]);
        glUniform1i(textSamplerLoc, 0);
        glUniform4fv(glGetUniformLocation(textShaderProgram, "textColor"), 1, glm::value_ptr(color));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
    }

    std::pair<int, int> calculateTextSize(const std::string& msg) const {
        if (!currentFont) {
            return { 0, 0 };
        }

        int totalWidth = 0;
        int maxHeight = 0;

        for (char c : msg) {
            if (FT_Load_Char(currentFont, c, FT_LOAD_RENDER)) {
                continue;
            }

            FT_GlyphSlot glyph = currentFont->glyph;
            totalWidth += glyph->advance.x >> 6; // Bitshift by 6 to get value in pixels

            if (glyph->bitmap.rows > maxHeight) {
                maxHeight = glyph->bitmap.rows;
            }
        }

        return { totalWidth, maxHeight };
    }

    std::pair<float, float> getAnchoredPosition(float x, float y, float textWidth, float textHeight, Anchor anchor) {
        // Use current UI size
        float uiWidth = this->width;
        float uiHeight = this->height;
        
        // Safety check for valid dimensions
        if (uiWidth <= 0 || uiHeight <= 0) {
            std::cerr << "UIText: Invalid canvas dimensions: " << uiWidth << "x" << uiHeight << std::endl;
            return { x, y };
        }

        float finalX = x;
        float finalY = y;
        
        // Use the current Position for offset calculations, which is now maintained proportionally
        float offsetX = Position.x;
        float offsetY = Position.y;

        switch (anchor) {
        case Anchor::TopLeft:
            finalX = offsetX;
            finalY = offsetY;
            break;
        case Anchor::TopCenter:
            finalX = (uiWidth - textWidth) * 0.5f + offsetX;
            finalY = offsetY;
            break;
        case Anchor::TopRight:
            finalX = uiWidth - textWidth - offsetX;
            finalY = offsetY;
            break;
        case Anchor::CenterLeft:
            finalX = offsetX;
            finalY = (uiHeight - textHeight) * 0.5f + offsetY;
            break;
        case Anchor::Center:
            finalX = (uiWidth - textWidth) * 0.5f + offsetX;
            finalY = (uiHeight - textHeight) * 0.5f + offsetY;
            break;
        case Anchor::CenterRight:
            finalX = uiWidth - textWidth - offsetX;
            finalY = (uiHeight - textHeight) * 0.5f + offsetY;
            break;
        case Anchor::BottomLeft:
            finalX = offsetX;
            finalY = uiHeight - textHeight - offsetY;
            break;
        case Anchor::BottomCenter:
            finalX = (uiWidth - textWidth) * 0.5f + offsetX;
            finalY = uiHeight - textHeight - offsetY;
            break;
        case Anchor::BottomRight:
            finalX = uiWidth - textWidth - offsetX;
            finalY = uiHeight - textHeight - offsetY;
            break;
        }

        // Ensure the element stays within bounds with proper clamping
        // For X axis: keep text within horizontal bounds
        if (textWidth > 0) {
            finalX = std::max(0.0f, std::min(finalX, uiWidth - textWidth));
        } else {
            finalX = std::max(0.0f, std::min(finalX, uiWidth));
        }
        
        // For Y axis: keep text within vertical bounds 
        // Special handling for top anchors to prevent going above screen
        if (textHeight > 0) {
            finalY = std::max(0.0f, std::min(finalY, uiHeight - textHeight));
        } else {
            finalY = std::max(0.0f, std::min(finalY, uiHeight));
        }
        
        // Additional safety for top anchors - ensure minimum distance from top
        // Since Y=0 is top of screen in our coordinate system, ensure we don't go negative
        if (anchor == Anchor::TopLeft || anchor == Anchor::TopCenter || anchor == Anchor::TopRight) {
            // For top anchors, ensure we have at least textHeight distance from top edge
            finalY = std::max(textHeight > 0 ? textHeight : 20.0f, finalY);
        }

        return { finalX, finalY };
    }

    GLuint generateCharacterTexture(FT_GlyphSlot glyph) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Convert bitmap to RGBA
        int width = glyph->bitmap.width;
        int height = glyph->bitmap.rows;

        if (width > 0 && height > 0) {
            std::vector<unsigned char> rgbaData(width * height * 4);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int srcIndex = y * width + x;
                    int dstIndex = (y * width + x) * 4;

                    unsigned char alpha = glyph->bitmap.buffer[srcIndex];
                    rgbaData[dstIndex] = 255;     // R
                    rgbaData[dstIndex + 1] = 255; // G
                    rgbaData[dstIndex + 2] = 255; // B
                    rgbaData[dstIndex + 3] = alpha; // A
                }
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
        }
        else {
            // Create 1x1 transparent texture for empty characters
            unsigned char transparentPixel[4] = { 0, 0, 0, 0 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, transparentPixel);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return texture;
    }

    GLuint getCharacterTexture(char c) {
        auto it = characterTextures.find(c);
        return (it != characterTextures.end()) ? it->second : 0;
    }
    
    // Override drag methods from UIBehaviour for better text drag support
    glm::vec4 getBounds() const override;
    glm::vec2 getPosition() const override;
    void setPosition(const glm::vec2& position) override;
    
protected:
    // Override drag event handlers for custom text drag behavior
    void onDragStarted(const UIDragEvent& event) override;
    void onDragUpdated(const UIDragEvent& event) override;
    void onDragEnded(const UIDragEvent& event) override;
};