#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../core/FileSystem.h"
#include "../core/CoreExporter.h"

class MANTRAXCORE_API Shader {
public:
    GLuint ID;

    // Constructor: compila y linkea
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vCode;
        std::string fCode;

        FileSystem::readString(vertexPath, vCode);
        FileSystem::readString(fragmentPath, fCode);

        GLuint vertex = compileShader(vCode.c_str(), GL_VERTEX_SHADER);
        GLuint fragment = compileShader(fCode.c_str(), GL_FRAGMENT_SHADER);

        // Crear programa
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);

        // Verificar errores de link
        checkLinkErrors(ID);

        // Borrar shaders intermedios
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Destructor: liberar memoria del programa
    ~Shader() {
        if (ID != 0) {
            glDeleteProgram(ID);
        }
    }

    // Impedir copias accidentales (evita doble delete)
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Permitir mover (transferir ownership del programa)
    Shader(Shader&& other) noexcept {
        ID = other.ID;
        other.ID = 0;
    }
    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            // Liberar el actual si existe
            if (ID != 0) {
                glDeleteProgram(ID);
            }
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

    void use() const {
        glUseProgram(ID);
    }

    GLuint getID() const {
        return ID;
    }
    

    // --------- Setters para uniforms ---------

    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    // --------- Getters para uniforms ---------

    bool getBool(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return false;
        GLint value;
        glGetUniformiv(ID, location, &value);
        return value != 0;
    }

    int getInt(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return 0;
        GLint value;
        glGetUniformiv(ID, location, &value);
        return value;
    }

    float getFloat(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return 0.0f;
        GLfloat value;
        glGetUniformfv(ID, location, &value);
        return value;
    }

    glm::vec2 getVec2(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::vec2(0.0f);
        glm::vec2 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

    glm::vec3 getVec3(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::vec3(0.0f);
        glm::vec3 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

    glm::vec4 getVec4(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::vec4(0.0f);
        glm::vec4 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

    glm::mat2 getMat2(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::mat2(1.0f);
        glm::mat2 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

    glm::mat3 getMat3(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::mat3(1.0f);
        glm::mat3 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

    glm::mat4 getMat4(const std::string &name) const {
        GLint location = glGetUniformLocation(ID, name.c_str());
        if (location == -1) return glm::mat4(1.0f);
        glm::mat4 value;
        glGetUniformfv(ID, location, glm::value_ptr(value));
        return value;
    }

private:
    GLuint compileShader(const char* source, GLenum type) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cerr << "Error compilando "
                      << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                      << " shader: " << log << std::endl;
        }
        return shader;
    }

    void checkLinkErrors(GLuint program) {
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, log);
            std::cerr << "Error linkeando programa: " << log << std::endl;
        }
    }
};
