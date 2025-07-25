#pragma once
#include <GL/glew.h>  
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API NativeGeometry {
public:
    NativeGeometry();
    virtual ~NativeGeometry();

    virtual void draw() const;
    virtual void drawInstanced(const std::vector<glm::mat4>& modelMatrices) const;
    virtual void updateInstanceBuffer(const std::vector<glm::mat4>& modelMatrices);
    
    // Para soporte de modelos 3D
    virtual bool usesModelNormals() const { return false; }
    virtual glm::vec3 getBoundingBoxMin() const { return glm::vec3(-0.5f); }
    virtual glm::vec3 getBoundingBoxMax() const { return glm::vec3(0.5f); }

protected:
    GLuint vao, vbo, instanceVBO;
    std::vector<glm::mat4> currentInstances;
};
