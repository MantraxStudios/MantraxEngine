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

    std::map<char, GLuint> characterTextures;

    void update() override {
        // Only try to draw if we have a valid font and OpenGL context
        if (currentFont && textShaderProgram != 0 && width > 0 && height > 0) {
            std::cout << "UIText update: Text='" << Text << "', Position=(" << Position.x << ", " << Position.y 
                      << "), Canvas size=(" << width << "x" << height << "), shader=" << textShaderProgram 
                      << ", font=" << currentFont << std::endl;
            drawText(Text, Position.x, Position.y, Color);
        } else {
            std::cout << "UIText update skipped: font=" << (currentFont ? "yes" : "no") 
                      << ", shader=" << (textShaderProgram != 0 ? "yes" : "no") 
                      << ", size=(" << width << "x" << height << ")" << std::endl;
        }
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

        std::cout << "UIText rendering: '" << msg << "' at (" << x << ", " << y << ") with canvas size (" << width << "x" << height << "), font=" << currentFont << ", ortho matrix available=" << (ortho != glm::mat4(0.0f)) << std::endl;
        
        // Check if coordinates are within viewport
        if (x < 0 || y < 0 || x > width || y > height) {
            std::cout << "UIText WARNING: Text position (" << x << ", " << y << ") is outside viewport (" << width << "x" << height << ")" << std::endl;
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

        std::cout << "UIText drawCharacter: texture=" << texture << ", pos=(" << x << "," << y 
                  << "), size=(" << w << "x" << h << "), canvas size=(" << width << "x" << height << ")" << std::endl;

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
        std::cout << "UIText: Set ortho matrix, model matrix for pos=(" << x << "," << y << "), scale=(" << w << "x" << h << ")" << std::endl;
        glUniform1i(textSamplerLoc, 0);
        glUniform4fv(glGetUniformLocation(textShaderProgram, "textColor"), 1, glm::value_ptr(color));
        std::cout << "UIText: Set text color to (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        std::cout << "UIText: Drew character with texture " << texture << std::endl;

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
    }

    std::pair<int, int> calculateTextSize(const std::string& msg) {
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

    std::pair<float, float> getAnchoredPosition(float x, float y, float width, float height, Anchor anchor) {
        // Use UI size, not screen size
        float uiWidth = this->width;
        float uiHeight = this->height;

        float finalX = x;
        float finalY = y;

        switch (anchor) {
        case Anchor::TopLeft:
            finalX = x;
            finalY = y;
            break;
        case Anchor::TopCenter:
            finalX = (uiWidth - width) * 0.5f + x;
            finalY = y;
            break;
        case Anchor::TopRight:
            finalX = uiWidth - width - x;
            finalY = y;
            break;
        case Anchor::CenterLeft:
            finalX = x;
            finalY = (uiHeight - height) * 0.5f + y;
            break;
        case Anchor::Center:
            finalX = (uiWidth - width) * 0.5f + x;
            finalY = (uiHeight - height) * 0.5f + y;
            break;
        case Anchor::CenterRight:
            finalX = uiWidth - width - x;
            finalY = (uiHeight - height) * 0.5f + y;
            break;
        case Anchor::BottomLeft:
            finalX = x;
            finalY = uiHeight - height - y;
            break;
        case Anchor::BottomCenter:
            finalX = (uiWidth - width) * 0.5f + x;
            finalY = uiHeight - height - y;
            break;
        case Anchor::BottomRight:
            finalX = uiWidth - width - x;
            finalY = uiHeight - height - y;
            break;
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
};