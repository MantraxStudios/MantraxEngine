#pragma once
#include <GL/glew.h>
#include <string>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API Texture {
public:
    Texture();
    Texture(const std::string& filePath);
    ~Texture();

    bool loadFromFile(const std::string& filePath);
    bool loadIconFromFile(const std::string& filePath); // New method for loading icons
    void bind(unsigned int slot = 0) const;
    void unbind() const;
    
    inline int getWidth() const { return width; }
    inline int getHeight() const { return height; }
    inline GLuint getID() const { return rendererID; }
    inline std::string getFilePath() const { return filePath; }

private:
    GLuint rendererID;
    std::string filePath;
    unsigned char* localBuffer;
    int width, height, BPP;
}; 