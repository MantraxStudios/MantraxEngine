#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "UIBehaviour.h"
#include "UIElements/UIText.h"
#include "../core/CoreExporter.h"
#include "../render/AspectRatioManager.h"

class MANTRAXCORE_API Canvas2D {
public:
    Canvas2D(int w, int h);
    ~Canvas2D();

    // Load a font with FreeType
    bool loadFont(const std::string& path, int size);
    void setSize(int w, int h);
    
    // Get current UI size
    int getWidth() const;
    int getHeight() const;
    
    // Set UI size independently of screen size
    void setUISize(int w, int h);
    void clear(glm::vec3 color = glm::vec3(0.2f, 0.2f, 0.2f));
    
    // Aspect ratio management
    void setAspectRatioMode(AspectRatioMode mode);
    void updateAspectRatio(int screenWidth, int screenHeight);
    glm::vec2 getUIScale() const;

    // Internal drawQuad method (without anchor)
    void drawQuadInternal(float x, float y, float w, float h, glm::vec3 color);

    // Internal drawText method (without anchor)
    std::pair<int, int> drawTextInternal(const std::string& msg, float x, float y, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    GLuint getShader() const;

    // Anchor-based positioning methods
    std::pair<float, float> getAnchoredPosition(float x, float y, float width, float height, Anchor anchor);

    // Overloaded drawQuad with anchor support
    void drawQuad(float x, float y, float w, float h, glm::vec3 color, Anchor anchor = Anchor::TopLeft);

    // Overloaded drawText with anchor support
    std::pair<int, int> drawText(const std::string& msg, float x, float y, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), Anchor anchor = Anchor::TopLeft);
    
    // Calculate text size without rendering
    std::pair<int, int> calculateTextSize(const std::string& msg);

    // Ensure OpenGL context is bound (for safety)
    static bool ensureOpenGLContext();

    std::vector<UIBehaviour*> RenderElements = std::vector<UIBehaviour*>();
    void DrawElements();


    UIText* MakeNewText(std::string Text);

private:
    int width, height;
    glm::mat4 ortho;
    
    // FreeType
    FT_Library ftLibrary;
    FT_Face currentFont;
    int currentFontSize;
    std::map<std::string, FT_Face> loadedFonts;
    std::map<char, GLuint> characterTextures;

    GLuint shaderProgram;
    GLuint quadVAO, quadVBO;
    GLint orthoLoc, modelLoc, colorLoc;

    GLuint textShaderProgram;
    GLint textOrthoLoc, textModelLoc, textSamplerLoc;

    void updateOrtho();
    void setupShader();
    void setupTextShader();
    void setupQuad();
    GLuint getCharacterTexture(char c);
    GLuint generateCharacterTexture(FT_GlyphSlot glyph);
    void drawCharacter(GLuint texture, float x, float y, float w, float h, glm::vec4 color);
    void checkCompile(GLuint shader, const char* type);
    void checkLink(GLuint prog);
};