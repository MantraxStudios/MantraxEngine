#pragma once
#include <GL/glew.h>
#include <string>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API DefaultShaders {
public:
    DefaultShaders();
    ~DefaultShaders();

    GLuint getProgram() const;

private:
    GLuint program;

    GLuint compileShader(GLenum type, const std::string& source);
};
