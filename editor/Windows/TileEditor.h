#pragma once
#include "WindowBehaviour.h"
#include "render/Texture.h"
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class TileEditor : public WindowBehaviour {
public:
    void OnRenderGUI() override;

    const std::string& getName() const override {
        static const std::string name = "Tile Editor";
        return name;
    }
};
