#include "DefaultShaders.h"
#include <iostream>
#include "../core/FileSystem.h"

DefaultShaders::DefaultShaders() {
    shaderGraphic = new Shader("engine/shaders/StandardVertex.glsl", "engine/shaders/StandardFragment.glsl");
}

Shader* DefaultShaders::getProgram() const {
    return shaderGraphic;
}

// Post-processing uniform setters
void DefaultShaders::setExposure(float exposure) {
    shaderGraphic->setFloat("uExposure", exposure);
}

void DefaultShaders::setSaturation(float saturation) {
    shaderGraphic->setFloat("uSaturation", saturation);
}

void DefaultShaders::setSmoothness(float smoothness) {
    shaderGraphic->setFloat("uSmoothness", smoothness);
}

// Post-processing uniform getters
float DefaultShaders::getExposure() const {
    return shaderGraphic->getFloat("uExposure");
}

float DefaultShaders::getSaturation() const {
    return shaderGraphic->getFloat("uSaturation");
}

float DefaultShaders::getSmoothness() const {
    return shaderGraphic->getFloat("uSmoothness");
}