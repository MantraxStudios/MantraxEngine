#pragma once
#include <string>
#include "components/Scene.h"
#include "core/CoreExporter.h"

class MANTRAXCORE_API SceneSaver {
public:
    static bool SaveScene(const Scene* scene, const std::string& filepath);
}; 