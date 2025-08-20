#pragma once
#include "Shader.h"
#include <string>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API DefaultShaders {
public:
    Shader* shaderGraphic;
    DefaultShaders();

    Shader* getProgram() const;
    
    // Post-processing uniform setters
    void setExposure(float exposure);
    void setSaturation(float saturation);
    void setSmoothness(float smoothness);
    
    // Post-processing uniform getters
    float getExposure() const;
    float getSaturation() const;
    float getSmoothness() const;
};
