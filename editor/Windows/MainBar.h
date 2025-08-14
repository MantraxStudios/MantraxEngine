#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "WindowBehaviour.h"
#include <imgui/imgui.h>
#include <iostream>
#include <nlohmann/json.hpp>

class MainBar : public WindowBehaviour
{
public:
	void OnRenderGUI() override;
    const std::string& getName() const override {
        static const std::string name = "Main Bar";
        return name;
    }

    bool materialesCargados = false;
    std::vector<std::string> materialNames;
    int selectedMaterial = 0;
    nlohmann::json materialsConfigJson;
    std::string lastKey = "";

    // Buffers para propiedades editables
    char nameBuffer[64] = "";
    float albedo[3] = { 1.0f, 1.0f, 1.0f };
    float metallic = 0.0f;
    float roughness = 1.0f;
    float emissive[3] = { 0.0f, 0.0f, 0.0f };
    float tiling[2] = { 1.0f, 1.0f };
    float normalStrength = 1.0f;

    char albedoTexture[128] = "";
    char normalTexture[128] = "";
    char metallicTexture[128] = "";
    char roughnessTexture[128] = "";
    char emissiveTexture[128] = "";
    char aoTexture[128] = "";
};