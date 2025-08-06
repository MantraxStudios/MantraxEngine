#include "DefaultShaders.h"
#include <iostream>
#include "../core/FileSystem.h"

DefaultShaders::DefaultShaders() {
    std::string _Frag;
    std::string _Vert;

    FileSystem::readString("engine/shaders/StandardVertex.glsl", _Vert);
    FileSystem::readString("engine/shaders/StandardFragment.glsl", _Frag);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, _Vert);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, _Frag);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Verificar errores de enlace
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error al enlazar shaders: " << infoLog << std::endl;
    }
    

}

DefaultShaders::~DefaultShaders() {
    glDeleteProgram(program);
}

GLuint DefaultShaders::getProgram() const {
    return program;
}

GLuint DefaultShaders::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Verificar errores de compilaci�n
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error al compilar shader: " << infoLog << std::endl;
    }

    return shader;
}

// Post-processing uniform setters
void DefaultShaders::setExposure(float exposure) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uExposure");
    if (location != -1) {
        glUniform1f(location, exposure);
    }
}

void DefaultShaders::setSaturation(float saturation) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uSaturation");
    if (location != -1) {
        glUniform1f(location, saturation);
    }
}

void DefaultShaders::setSmoothness(float smoothness) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uSmoothness");
    if (location != -1) {
        glUniform1f(location, smoothness);
    }
}

// Post-processing uniform getters
float DefaultShaders::getExposure() const {
    GLint location = glGetUniformLocation(program, "uExposure");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}



float DefaultShaders::getSaturation() const {
    GLint location = glGetUniformLocation(program, "uSaturation");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}

float DefaultShaders::getSmoothness() const {
    GLint location = glGetUniformLocation(program, "uSmoothness");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}


