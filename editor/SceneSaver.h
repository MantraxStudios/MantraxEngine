#pragma once
#include <string>
#include "components/Scene.h"
#include "core/CoreExporter.h"
#include <components/SceneManager.h>
#include <nlohmann/json.hpp>

class SceneSaver {
public:
    static bool SaveScene(const Scene* scene, const std::string& filepath);
}; 