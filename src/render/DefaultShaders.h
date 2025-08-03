#pragma once
#include <GL/glew.h>
#include <string>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API DefaultShaders {
public:
    DefaultShaders();
    ~DefaultShaders();

    GLuint getProgram() const;
    
    // Post-processing uniform setters
    void setExposure(float exposure);
    void setSaturation(float saturation);
    void setSmoothness(float smoothness);
    
    // Post-processing uniform getters
    float getExposure() const;
    float getSaturation() const;
    float getSmoothness() const;

private:
    GLuint program;

    GLuint compileShader(GLenum type, const std::string& source);
};
