#include "Canvas.h"
#include <SDL.h>
#include <stdexcept>
#include "../components/SceneManager.h"
#include "../render/Camera.h"
#include "../render/Framebuffer.h"
#include "../render/AspectRatioManager.h"

// Constructor
Canvas2D::Canvas2D(int w, int h)
    : width(w), height(h), ftLibrary(nullptr) {
    
    // Check if OpenGL context is current
    if (!ensureOpenGLContext()) {
        std::cerr << "ERROR: No OpenGL context is current when creating Canvas2D!" << std::endl;
        throw std::runtime_error("OpenGL context not bound");
    }
    
    // Initialize member variables to safe defaults
    shaderProgram = 0;
    textShaderProgram = 0;
    quadVAO = 0;
    quadVBO = 0;
    orthoLoc = -1;
    modelLoc = -1;
    colorLoc = -1;
    textOrthoLoc = -1;
    textModelLoc = -1;
    textSamplerLoc = -1;
    currentFont = nullptr;
    currentFontSize = 0;
    
    try {
        setupShader();
        setupTextShader();
        setupQuad();
        updateOrtho();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Initialize FreeType
        if (FT_Init_FreeType(&ftLibrary)) {
            std::cerr << "Could not init FreeType Library" << std::endl;
            ftLibrary = nullptr;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error initializing Canvas2D: " << e.what() << std::endl;
        // Clean up any partially initialized resources
        if (shaderProgram) glDeleteProgram(shaderProgram);
        if (textShaderProgram) glDeleteProgram(textShaderProgram);
        if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
        if (quadVBO) glDeleteBuffers(1, &quadVBO);
        throw;
    }
}

// Destructor
Canvas2D::~Canvas2D() {
    // Check if OpenGL context is current before deleting resources
    if (SDL_GL_GetCurrentContext()) {
        if (shaderProgram) {
            glDeleteProgram(shaderProgram);
            shaderProgram = 0;
        }
        if (textShaderProgram) {
            glDeleteProgram(textShaderProgram);
            textShaderProgram = 0;
        }
        if (quadVAO) {
            glDeleteVertexArrays(1, &quadVAO);
            quadVAO = 0;
        }
        if (quadVBO) {
            glDeleteBuffers(1, &quadVBO);
            quadVBO = 0;
        }
    }
    
    // Clean up loaded fonts FIRST
    for (auto& font : loadedFonts) {
        if (font.second) {
            FT_Done_Face(font.second);
        }
    }
    loadedFonts.clear();
    
    // Clean up FreeType library LAST
    if (ftLibrary) {
        FT_Done_FreeType(ftLibrary);
        ftLibrary = nullptr;
    }
}

// Load a font with FreeType
bool Canvas2D::loadFont(const std::string& path, int size) {
    if (!ftLibrary) {
        std::cerr << "FreeType library not initialized!" << std::endl;
        return false;
    }
    
    std::cout << "Attempting to load font: " << path << " with size: " << size << std::endl;
    
    // Check if font is already loaded
    auto it = loadedFonts.find(path);
    if (it != loadedFonts.end()) {
        // Font already loaded, just set size
        FT_Face face = it->second;
        if (FT_Set_Pixel_Sizes(face, 0, size)) {
            std::cerr << "Could not set font size" << std::endl;
            return false;
        }
        currentFont = face;
        currentFontSize = size;
        std::cout << "Font size updated successfully" << std::endl;
        return true;
    }
    
    // Load new font
    FT_Face face;
    if (FT_New_Face(ftLibrary, path.c_str(), 0, &face)) {
        std::cerr << "Failed to load font: " << path << std::endl;
        return false;
    }
    
    // Set font size
    if (FT_Set_Pixel_Sizes(face, 0, size)) {
        std::cerr << "Could not set font size" << std::endl;
        FT_Done_Face(face);
        return false;
    }
    
    // Store font
    loadedFonts[path] = face;
    currentFont = face;
    currentFontSize = size;
    
    std::cout << "Font loaded successfully: " << path << " (Size: " << size << ")" << std::endl;
    return true;
}

void Canvas2D::setSize(int w, int h) {
    width = w;
    height = h;
    updateOrtho();
}

// Get current UI size
int Canvas2D::getWidth() const { return width; }
int Canvas2D::getHeight() const { return height; }

// Set UI size independently of screen size
void Canvas2D::setUISize(int w, int h) {
    width = w;
    height = h;
    updateOrtho();
    
    // Update all UI elements with new dimensions
    for (UIBehaviour* element : RenderElements) {
        element->width = static_cast<float>(w);
        element->height = static_cast<float>(h);
        
        // Update UIText elements specifically with new resize handler
        if (UIText* textElement = dynamic_cast<UIText*>(element)) {
            textElement->onCanvasResize(static_cast<float>(w), static_cast<float>(h));
            textElement->ortho = this->ortho;
            // Don't call update() here - it will be called in DrawElements()
        }
    }
    
    std::cout << "Canvas2D: Updated UI size to " << w << "x" << h << " and notified " << RenderElements.size() << " UI elements" << std::endl;
}

void Canvas2D::clear(glm::vec3 color) {
    glClearColor(color.r, color.g, color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// Internal drawQuad method (without anchor)
void Canvas2D::drawQuadInternal(float x, float y, float w, float h, glm::vec3 color) {
    glUseProgram(shaderProgram);
    glViewport(0, 0, width, height);
    
    // Disable depth testing for 2D quads
    glDisable(GL_DEPTH_TEST);
    
    glUniformMatrix4fv(orthoLoc, 1, GL_FALSE, &ortho[0][0]);
    glUniform3fv(colorLoc, 1, &color[0]);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    
    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

// Internal drawText method (without anchor)
std::pair<int, int> Canvas2D::drawTextInternal(const std::string& msg, float x, float y, glm::vec4 color) {
    if (!currentFont) {
        std::cerr << "Canvas2D: No font loaded!" << std::endl;
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


void Canvas2D::DrawElements() {
    std::cout << "[Canvas2D] DrawElements called with " << RenderElements.size() << " elements" << std::endl;
    
    // Use camera framebuffer size instead of window size
    int canvasWidth = width;
    int canvasHeight = height;
    
    // Get camera framebuffer size from active scene
    Scene* activeScene = SceneManager::getInstance().getActiveScene();
    if (activeScene && activeScene->getCamera()) {
        Camera* camera = activeScene->getCamera();
        int newWidth = camera->getBufferWidth();
        int newHeight = camera->getBufferHeight();
        
        std::cout << "Canvas2D: Camera framebuffer size: " << newWidth << "x" << newHeight << std::endl;
        std::cout << "Canvas2D: Current canvas size: " << canvasWidth << "x" << canvasHeight << std::endl;
        
        canvasWidth = newWidth;
        canvasHeight = newHeight;
        
        // Update ortho matrix for new size
        if (canvasWidth != width || canvasHeight != height) {
            std::cout << "Canvas2D: Size changed! Updating from " << width << "x" << height << " to " << canvasWidth << "x" << canvasHeight << std::endl;
            width = canvasWidth;
            height = canvasHeight;
            updateOrtho();
            
            // Notify all UI elements of the size change (but don't call update() here)
            for (UIBehaviour* element : RenderElements) {
                element->width = static_cast<float>(width);
                element->height = static_cast<float>(height);
                
                // Update UIText elements specifically with new resize handler
                if (UIText* textElement = dynamic_cast<UIText*>(element)) {
                    textElement->onCanvasResize(static_cast<float>(width), static_cast<float>(height));
                    textElement->ortho = this->ortho;
                    // Don't call update() here - it will be called in the loop below
                }
            }
        }
    }
    
    // Set viewport for all UI elements
    glViewport(0, 0, canvasWidth, canvasHeight);
    
    for (UIBehaviour* Behaviour : RenderElements)
    {
        Behaviour->quadVAO = quadVAO;
        Behaviour->quadVBO = quadVBO;
        Behaviour->orthoLoc = orthoLoc;
        Behaviour->modelLoc = modelLoc;
        Behaviour->colorLoc = colorLoc;
        Behaviour->textOrthoLoc = textOrthoLoc;
        Behaviour->textModelLoc = textModelLoc;
        Behaviour->textSamplerLoc = textSamplerLoc;
        Behaviour->textShaderProgram = textShaderProgram;
        Behaviour->width = static_cast<float>(canvasWidth);
        Behaviour->height = static_cast<float>(canvasHeight);
        
        // Cast to UIText to set additional properties
        if (UIText* textElement = dynamic_cast<UIText*>(Behaviour)) {
            textElement->ortho = this->ortho;
            textElement->currentFont = this->currentFont;
            textElement->characterTextures = this->characterTextures;
        }
        
        // Always call update() here (only once per frame)
        std::cout << "[Canvas2D] Updating UI element" << std::endl;
        Behaviour->update();
    }
}

GLuint Canvas2D::getShader() const { return shaderProgram; }

// Anchor-based positioning methods
std::pair<float, float> Canvas2D::getAnchoredPosition(float x, float y, float width, float height, Anchor anchor) {
    // Use UI size, not screen size
    float uiWidth = static_cast<float>(this->width);
    float uiHeight = static_cast<float>(this->height);
    
    std::cout << "Canvas2D::getAnchoredPosition - Input: x=" << x << ", y=" << y << ", w=" << width << ", h=" << height << std::endl;
    std::cout << "Canvas2D::getAnchoredPosition - UI size: " << uiWidth << "x" << uiHeight << std::endl;
    
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
    
    // Ensure the element stays within bounds
    finalX = std::max(0.0f, std::min(finalX, uiWidth - width));
    finalY = std::max(0.0f, std::min(finalY, uiHeight - height));
    
    std::cout << "Canvas2D::getAnchoredPosition - Output: x=" << finalX << ", y=" << finalY << " (Anchor: " << static_cast<int>(anchor) << ")" << std::endl;
    
    return { finalX, finalY };
}

// Overloaded drawQuad with anchor support
void Canvas2D::drawQuad(float x, float y, float w, float h, glm::vec3 color, Anchor anchor) {
    auto [finalX, finalY] = getAnchoredPosition(x, y, w, h, anchor);
    drawQuadInternal(finalX, finalY, w, h, color);
}

// Overloaded drawText with anchor support
std::pair<int, int> Canvas2D::drawText(const std::string& msg, float x, float y, glm::vec4 color, Anchor anchor) {
    // Calculate text size for all anchors that need it
    auto [textWidth, textHeight] = calculateTextSize(msg);
    
    // Get anchored position using text dimensions
    auto [finalX, finalY] = getAnchoredPosition(x, y, static_cast<float>(textWidth), static_cast<float>(textHeight), anchor);
    return drawTextInternal(msg, finalX, finalY, color);
}

// Calculate text size without rendering
std::pair<int, int> Canvas2D::calculateTextSize(const std::string& msg) {
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

void Canvas2D::updateOrtho() {
    ortho = glm::ortho(0.0f, float(width), float(height), 0.0f, -1.0f, 1.0f);
}

void Canvas2D::setupShader() {
    const char* vtx = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 ortho;
        uniform mat4 model;
        void main() {
            gl_Position = ortho * model * vec4(aPos, 0.0, 1.0);
        }
    )";
    const char* frag = R"(
        #version 330 core
        uniform vec3 color;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // Check for OpenGL errors before starting
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error before shader setup: " << error << std::endl;
    }
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (!vs || !fs) {
        throw std::runtime_error("Failed to create shader objects");
    }
    
    glShaderSource(vs, 1, &vtx, nullptr);
    glShaderSource(fs, 1, &frag, nullptr);
    glCompileShader(vs);
    glCompileShader(fs);
    checkCompile(vs, "VERTEX");
    checkCompile(fs, "FRAGMENT");

    shaderProgram = glCreateProgram();
    if (!shaderProgram) {
        throw std::runtime_error("Failed to create shader program");
    }
    
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    checkLink(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    orthoLoc = glGetUniformLocation(shaderProgram, "ortho");
    modelLoc = glGetUniformLocation(shaderProgram, "model");
    colorLoc = glGetUniformLocation(shaderProgram, "color");
    
    // Check for OpenGL errors after setup
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after shader setup: " << error << std::endl;
    }
}

void Canvas2D::setupTextShader() {
    const char* vtx = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 ortho;
        uniform mat4 model;
        out vec2 uv;
        void main() {
            gl_Position = ortho * model * vec4(aPos, 0.0, 1.0);
            uv = aPos;
        }
    )";
    const char* frag = R"(
        #version 330 core
        in vec2 uv;
        uniform sampler2D tex;
        uniform vec4 textColor;
        out vec4 FragColor;
        void main() {
            vec4 texColor = texture(tex, uv);
            // Only render if there's content (alpha > 0.01)
            if (texColor.a < 0.01) {
                discard;
            }
            // Render only the letters with their original alpha
            FragColor = vec4(textColor.rgb, texColor.a * textColor.a);
        }
    )";
    
    // Check for OpenGL errors before starting
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error before text shader setup: " << error << std::endl;
    }
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (!vs || !fs) {
        throw std::runtime_error("Failed to create text shader objects");
    }
    
    glShaderSource(vs, 1, &vtx, nullptr);
    glShaderSource(fs, 1, &frag, nullptr);
    glCompileShader(vs);
    glCompileShader(fs);
    checkCompile(vs, "TEXT_VERTEX");
    checkCompile(fs, "TEXT_FRAGMENT");

    textShaderProgram = glCreateProgram();
    if (!textShaderProgram) {
        throw std::runtime_error("Failed to create text shader program");
    }
    
    glAttachShader(textShaderProgram, vs);
    glAttachShader(textShaderProgram, fs);
    glLinkProgram(textShaderProgram);
    checkLink(textShaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    textOrthoLoc = glGetUniformLocation(textShaderProgram, "ortho");
    textModelLoc = glGetUniformLocation(textShaderProgram, "model");
    textSamplerLoc = glGetUniformLocation(textShaderProgram, "tex");
    
    // Check for OpenGL errors after setup
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after text shader setup: " << error << std::endl;
    }
}

void Canvas2D::setupQuad() {
    float quadVerts[] = {
        0.f, 0.f,
        1.f, 0.f,
        0.f, 1.f,
        1.f, 1.f
    };
    
    // Check for OpenGL errors before starting
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error before quad setup: " << error << std::endl;
    }
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    if (!quadVAO || !quadVBO) {
        throw std::runtime_error("Failed to create VAO/VBO for quad");
    }
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    
    // Check for OpenGL errors after setup
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error after quad setup: " << error << std::endl;
    }
}

GLuint Canvas2D::getCharacterTexture(char c) {
    auto it = characterTextures.find(c);
    return (it != characterTextures.end()) ? it->second : 0;
}

GLuint Canvas2D::generateCharacterTexture(FT_GlyphSlot glyph) {
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
    } else {
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

void Canvas2D::drawCharacter(GLuint texture, float x, float y, float w, float h, glm::vec4 color) {
    if (w <= 0 || h <= 0) return;
    
    glUseProgram(textShaderProgram);
    glViewport(0, 0, width, height);
    
    // Configure blending for transparent text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth testing for 2D text
    glDisable(GL_DEPTH_TEST);
    
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

void Canvas2D::checkCompile(GLuint shader, const char* type) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << type << " SHADER COMPILATION ERROR:\n" << infoLog << std::endl;
    }
}

void Canvas2D::checkLink(GLuint prog) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        std::cerr << "SHADER PROGRAM LINKING ERROR:\n" << infoLog << std::endl;
    }
}

bool Canvas2D::ensureOpenGLContext() {
    // Check if we have a valid OpenGL context (SDL2 specific)
    if (!SDL_GL_GetCurrentContext()) {
        std::cerr << "WARNING: No OpenGL context is current!" << std::endl;
        return false;
    }
    
    // Check if GLEW is initialized and OpenGL 3.3+ is available
    if (!GLEW_VERSION_3_3) {
        std::cerr << "WARNING: OpenGL 3.3+ not available! GLEW may not be initialized properly." << std::endl;
        return false;
    }
    
    // Additional check: verify that we can actually create OpenGL objects
    GLuint testVAO;
    glGenVertexArrays(1, &testVAO);
    if (glGetError() != GL_NO_ERROR) {
        std::cerr << "WARNING: Cannot create OpenGL objects - context may not be properly bound!" << std::endl;
        return false;
    }
    glDeleteVertexArrays(1, &testVAO);
    
    return true;
}

UIText* Canvas2D::MakeNewText(std::string Text) {
    UIText* _Text = new UIText();
    _Text->Text = Text;
    
    // Initialize basic properties with a position that works well for all anchors
    // Use a smaller offset to prevent going off-screen, especially for top anchors
    _Text->Position = glm::vec2(5.0f, 5.0f); // Smaller offset to prevent off-screen issues
    _Text->Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    _Text->UIAnchor = Anchor::TopLeft;
    
    // Initialize canvas size with current canvas dimensions and set up relative positioning
    _Text->width = static_cast<float>(this->width);
    _Text->height = static_cast<float>(this->height);
    _Text->originalWidth = static_cast<float>(this->width);
    _Text->originalHeight = static_cast<float>(this->height);
    _Text->ortho = this->ortho;
    
    // Set up relative positioning
    if (this->width > 0 && this->height > 0) {
        _Text->relativePosition.x = _Text->Position.x / static_cast<float>(this->width);
        _Text->relativePosition.y = _Text->Position.y / static_cast<float>(this->height);
        _Text->useRelativePositioning = true;
    }
    
    // Initialize font and textures from canvas
    _Text->currentFont = this->currentFont;
    _Text->characterTextures = this->characterTextures;
    
    // OpenGL variables will be set in DrawElements()
    // This avoids duplication and ensures they're set correctly each frame

    RenderElements.push_back(_Text);
    std::cout << "Canvas2D: Created new UIText with canvas size " << this->width << "x" << this->height 
              << " and relative position (" << _Text->relativePosition.x << ", " << _Text->relativePosition.y << ")" << std::endl;
    return _Text;
}

// Aspect ratio management methods
void Canvas2D::setAspectRatioMode(AspectRatioMode mode) {
    auto& aspectManager = AspectRatioManager::getInstance();
    AspectRatioSettings settings = aspectManager.getSettings();
    settings.mode = mode;
    aspectManager.setSettings(settings);
    
    std::cout << "[Canvas2D] Aspect ratio mode set to: " << static_cast<int>(mode) << std::endl;
}

void Canvas2D::updateAspectRatio(int screenWidth, int screenHeight) {
    auto& aspectManager = AspectRatioManager::getInstance();
    aspectManager.updateScreenSize(screenWidth, screenHeight);
    
    // Calcular nuevo tamaño de UI basado en aspect ratio
    glm::vec2 uiScale = aspectManager.getCurrentUIScale();
    glm::ivec4 viewport = aspectManager.getCurrentViewport();
    
    // Actualizar tamaño del canvas basado en el viewport efectivo
    int newWidth = viewport.z;
    int newHeight = viewport.w;
    
    if (newWidth != width || newHeight != height) {
        setUISize(newWidth, newHeight);
        std::cout << "[Canvas2D] Updated canvas size to " << newWidth << "x" << newHeight 
                  << " based on aspect ratio (scale: " << uiScale.x << "," << uiScale.y << ")" << std::endl;
    }
}

glm::vec2 Canvas2D::getUIScale() const {
    return AspectRatioManager::getInstance().getCurrentUIScale();
}