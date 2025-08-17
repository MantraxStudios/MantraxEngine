#pragma once
#include <GL/glew.h>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind();
    void unbind();
    void resize(int width, int height);
    
    GLuint getColorTexture() const { return colorTexture; }
    GLuint getFramebuffer() const { return framebuffer; }
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    bool isValid() const { return framebuffer != 0; }

private:
    GLuint framebuffer;
    GLuint colorTexture;
    GLuint depthRenderbuffer;
    int width;
    int height;
    
    void createFramebuffer();
    void cleanup();
}; 