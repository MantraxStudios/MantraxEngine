// SceneSaver.cpp - Version con debug para heap corruption
#include "SceneSaver.h"
#include <iostream>

bool SceneSaver::SaveScene(const Scene* scene, const std::string& filepath) {
    if (!scene) {
        std::cerr << "Error: Null scene pointer" << std::endl;
        return false;
    }

    // TODO: Implementar guardado de escena
    std::cout << "SaveScene not implemented yet" << std::endl;
    return false;
}