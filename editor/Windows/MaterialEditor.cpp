#include "MaterialEditor.h"
#include "core/FileSystem.h"
#include <render/MaterialManager.h>
#include "render/RenderPipeline.h"
#include <map>
#include <memory>
#include <cstring>
#include <iostream>
#include "../EUI/UIBuilder.h"

void MaterialEditor::OnRenderGUI() {
    ImGui::Begin("Material Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    if (!materialesCargados) {
        LoadMaterials();
    }

    ImGui::Columns(2, 0, true);

    // Sidebar
    ImGui::TextColored(ImVec4(0.7f, 0.8f, 1, 1), "Materials");
    ImGui::Separator();

    ImGui::BeginChild("material_list", ImVec2(0, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < materialNames.size(); ++i) {
        ImGui::PushID(i);
        bool isSelected = (i == selectedMaterial);
        if (ImGui::Selectable(materialNames[i].c_str(), isSelected)) {
            selectedMaterial = i;
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::Spacing();
    if (ImGui::Button("Create Simple Material")) {
        CreateMaterial(MaterialType::Simple);
    }
    if (ImGui::Button("Create PBR Material")) {
        CreateMaterial(MaterialType::PBR);
    }

    ImGui::NextColumn();

    // Main: Editor
    if (!materialNames.empty()) {
        std::string matKey = materialNames[selectedMaterial];
        auto& matData = materialsConfigJson["materials"][matKey];

        // Solo cuando cambia el material
        if (lastKey != matKey) {
            strncpy_s(nameBuffer, sizeof(nameBuffer), matKey.c_str(), _TRUNCATE);

            if (matData.contains("albedo") && matData["albedo"].is_array()) {
                auto alb = matData["albedo"];
                albedo[0] = alb.size() > 0 ? alb[0].get<float>() : 1.0f;
                albedo[1] = alb.size() > 1 ? alb[1].get<float>() : 1.0f;
                albedo[2] = alb.size() > 2 ? alb[2].get<float>() : 1.0f;
            }
            else {
                albedo[0] = albedo[1] = albedo[2] = 1.0f;
            }

            metallic = matData.value("metallic", 0.0f);
            roughness = matData.value("roughness", 1.0f);

            if (matData.contains("emissive") && matData["emissive"].is_array()) {
                auto ems = matData["emissive"];
                emissive[0] = ems.size() > 0 ? ems[0].get<float>() : 0.0f;
                emissive[1] = ems.size() > 1 ? ems[1].get<float>() : 0.0f;
                emissive[2] = ems.size() > 2 ? ems[2].get<float>() : 0.0f;
            }
            else {
                emissive[0] = emissive[1] = emissive[2] = 0.0f;
            }

            if (matData.contains("tiling") && matData["tiling"].is_array()) {
                auto til = matData["tiling"];
                tiling[0] = til.size() > 0 ? til[0].get<float>() : 1.0f;
                tiling[1] = til.size() > 1 ? til[1].get<float>() : 1.0f;
            }
            else {
                tiling[0] = tiling[1] = 1.0f;
            }

            normalStrength = matData.value("normal_strength", 1.0f);

            if (matData.contains("textures") && matData["textures"].is_object()) {
                auto& tex = matData["textures"];
                strncpy_s(albedoTexture, sizeof(albedoTexture), tex.value("albedo", "").c_str(), _TRUNCATE);
                strncpy_s(normalTexture, sizeof(normalTexture), tex.value("normal", "").c_str(), _TRUNCATE);
                strncpy_s(metallicTexture, sizeof(metallicTexture), tex.value("metallic", "").c_str(), _TRUNCATE);
                strncpy_s(roughnessTexture, sizeof(roughnessTexture), tex.value("roughness", "").c_str(), _TRUNCATE);
                strncpy_s(emissiveTexture, sizeof(emissiveTexture), tex.value("emissive", "").c_str(), _TRUNCATE);
                strncpy_s(aoTexture, sizeof(aoTexture), tex.value("ao", "").c_str(), _TRUNCATE);
            }
            else {
                albedoTexture[0] = 0;
                normalTexture[0] = 0;
                metallicTexture[0] = 0;
                roughnessTexture[0] = 0;
                emissiveTexture[0] = 0;
                aoTexture[0] = 0;
            }

            previewAlbedo = GetTexturePreview(albedoTexture);
            previewNormal = GetTexturePreview(normalTexture);
            previewMetallic = GetTexturePreview(metallicTexture);
            previewRoughness = GetTexturePreview(roughnessTexture);
            previewEmissive = GetTexturePreview(emissiveTexture);
            previewAO = GetTexturePreview(aoTexture);

            lastKey = matKey;
        }

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::TextColored(ImVec4(0.7f, 1, 0.7f, 1), "Material: ");
        ImGui::SameLine();
        ImGui::InputText("##matName", nameBuffer, sizeof(nameBuffer));
        ImGui::PopFont();
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            DeleteMaterial(selectedMaterial);
            ImGui::Columns(1); ImGui::End();
            return;
        }

        if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Albedo", albedo);

            ImGui::InputText("Albedo Texture", albedoTexture, sizeof(albedoTexture));
            // Drag & drop fuera del if:
            {
                static std::string lastDroppedAlbedoTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedAlbedoTexture) {
                        strncpy_s(albedoTexture, sizeof(albedoTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewAlbedo = GetTexturePreview(albedoTexture); // recarga el preview al arrastrar
                        lastDroppedAlbedoTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedAlbedoTexture = "";
                }
            }
            if (previewAlbedo) {
                ImGui::SameLine();
                ImGui::Image(previewAlbedo, ImVec2(64, 64));
            }
        }


        if (ImGui::CollapsingHeader("PBR Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);

            ImGui::InputText("Metallic Texture", metallicTexture, sizeof(metallicTexture));
            {
                static std::string lastDroppedMetallicTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedMetallicTexture) {
                        strncpy_s(metallicTexture, sizeof(metallicTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewMetallic = GetTexturePreview(metallicTexture);
                        lastDroppedMetallicTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedMetallicTexture = "";
                }
            }
            if (previewMetallic) {
                ImGui::SameLine();
                ImGui::Image(previewMetallic, ImVec2(64, 64));
            }

            ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);

            ImGui::InputText("Roughness Texture", roughnessTexture, sizeof(roughnessTexture));
            {
                static std::string lastDroppedRoughnessTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedRoughnessTexture) {
                        strncpy_s(roughnessTexture, sizeof(roughnessTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewRoughness = GetTexturePreview(roughnessTexture);
                        lastDroppedRoughnessTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedRoughnessTexture = "";
                }
            }
            if (previewRoughness) {
                ImGui::SameLine();
                ImGui::Image(previewRoughness, ImVec2(64, 64));
            }

            ImGui::InputText("Normal Texture", normalTexture, sizeof(normalTexture));
            {
                static std::string lastDroppedNormalTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedNormalTexture) {
                        strncpy_s(normalTexture, sizeof(normalTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewNormal = GetTexturePreview(normalTexture);
                        lastDroppedNormalTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedNormalTexture = "";
                }
            }
            if (previewNormal) {
                ImGui::SameLine();
                ImGui::Image(previewNormal, ImVec2(64, 64));
            }

            ImGui::SliderFloat("Normal Strength", &normalStrength, 0.0f, 8.0f);
        }

        if (ImGui::CollapsingHeader("Advanced", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::ColorEdit3("Emissive", emissive);
            ImGui::InputText("Emissive Texture", emissiveTexture, sizeof(emissiveTexture));
            {
                static std::string lastDroppedEmissiveTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedEmissiveTexture) {
                        strncpy_s(emissiveTexture, sizeof(emissiveTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewEmissive = GetTexturePreview(emissiveTexture);
                        lastDroppedEmissiveTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedEmissiveTexture = "";
                }
            }
            if (previewEmissive) {
                ImGui::SameLine();
                ImGui::Image(previewEmissive, ImVec2(64, 64));
            }
            ImGui::InputFloat2("Tiling", tiling);
            ImGui::InputText("AO Texture", aoTexture, sizeof(aoTexture));
            {
                static std::string lastDroppedAOTexture = "";
                auto result = UIBuilder::Drag_Objetive("TextureClass");
                if (result.has_value()) {
                    std::string droppedTexturePath = result.value();
                    // Only process if this is a new texture (not the same as last frame)
                    if (droppedTexturePath != lastDroppedAOTexture) {
                        strncpy_s(aoTexture, sizeof(aoTexture), droppedTexturePath.c_str(), _TRUNCATE);
                        previewAO = GetTexturePreview(aoTexture);
                        lastDroppedAOTexture = droppedTexturePath;
                    }
                } else {
                    // Reset the last dropped texture when no texture is being dragged
                    lastDroppedAOTexture = "";
                }
            }
            if (previewAO) {
                ImGui::SameLine();
                ImGui::Image(previewAO, ImVec2(64, 64));
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Save")) {
            std::string newKey(nameBuffer);
            if (newKey != matKey && !newKey.empty() && !materialsConfigJson["materials"].contains(newKey)) {
                materialsConfigJson["materials"][newKey] = matData;
                materialsConfigJson["materials"].erase(matKey);
                matKey = newKey;
                materialNames[selectedMaterial] = newKey;
            }

            materialsConfigJson["materials"][matKey]["albedo"] = { albedo[0], albedo[1], albedo[2] };
            materialsConfigJson["materials"][matKey]["metallic"] = metallic;
            materialsConfigJson["materials"][matKey]["roughness"] = roughness;
            materialsConfigJson["materials"][matKey]["emissive"] = { emissive[0], emissive[1], emissive[2] };
            materialsConfigJson["materials"][matKey]["tiling"] = { tiling[0], tiling[1] };
            materialsConfigJson["materials"][matKey]["normal_strength"] = normalStrength;

            if (!materialsConfigJson["materials"][matKey].contains("textures") ||
                !materialsConfigJson["materials"][matKey]["textures"].is_object()) {
                materialsConfigJson["materials"][matKey]["textures"] = nlohmann::json::object();
            }
            auto& tex = materialsConfigJson["materials"][matKey]["textures"];
            tex["albedo"] = std::string(albedoTexture);
            tex["normal"] = std::string(normalTexture);
            tex["metallic"] = std::string(metallicTexture);
            tex["roughness"] = std::string(roughnessTexture);
            tex["emissive"] = std::string(emissiveTexture);
            tex["ao"] = std::string(aoTexture);

            SaveMaterials();
            feedbackMsg = "Material saved!";
            feedbackTime = ImGui::GetTime();
            materialesCargados = false;

            MaterialManager::getInstance().clearMaterials();
            MaterialManager::getInstance().loadMaterialsFromConfig("config/materials_config.json");
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            materialesCargados = false;
            feedbackMsg = "Reloaded materials";
            feedbackTime = ImGui::GetTime();

            MaterialManager::getInstance().clearMaterials();
            MaterialManager::getInstance().loadMaterialsFromConfig("config/materials_config.json");
        }
    }

    ImGui::Columns(1);

    if (!feedbackMsg.empty() && ImGui::GetTime() - feedbackTime < 2.0f) {
        ImGui::SetNextWindowBgAlpha(0.8f);
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1, 1, 0.3f, 1), "%s", feedbackMsg.c_str());
        ImGui::EndTooltip();
    }

    ImGui::End();
}

void MaterialEditor::CreateMaterial(MaterialType type) {
    int counter = 1;
    std::string baseName = (type == MaterialType::Simple) ? "Material_Simple" : "Material_PBR";
    std::string newName = baseName;
    while (materialsConfigJson["materials"].contains(newName)) {
        newName = baseName + std::to_string(counter++);
    }

    nlohmann::json newMat;
    newMat["albedo"] = { 1.0f, 1.0f, 1.0f };
    if (type == MaterialType::PBR) {
        newMat["metallic"] = 0.0f;
        newMat["roughness"] = 1.0f;
        newMat["emissive"] = { 0.0f, 0.0f, 0.0f };
        newMat["tiling"] = { 1.0f, 1.0f };
        newMat["normal_strength"] = 1.0f;
        newMat["textures"] = {
            {"albedo", ""},
            {"normal", ""},
            {"metallic", ""},
            {"roughness", ""},
            {"emissive", ""},
            {"ao", ""}
        };
    }
    else {
        newMat["textures"] = { {"albedo", ""} };
    }

    materialsConfigJson["materials"][newName] = newMat;
    SaveMaterials();
    feedbackMsg = (type == MaterialType::Simple ? "Simple material created!" : "PBR material created!");
    feedbackTime = ImGui::GetTime();
    materialesCargados = false;
    LoadMaterials();
    for (int i = 0; i < materialNames.size(); ++i) {
        if (materialNames[i] == newName) {
            selectedMaterial = i;
            break;
        }
    }
}

void MaterialEditor::DeleteMaterial(int index) {
    if (index >= 0 && index < materialNames.size()) {
        std::string matKey = materialNames[index];
        materialsConfigJson["materials"].erase(matKey);
        SaveMaterials();
        feedbackMsg = "Material deleted!";
        feedbackTime = ImGui::GetTime();
        materialesCargados = false;
        LoadMaterials();
        if (selectedMaterial >= materialNames.size())
            selectedMaterial = materialNames.size() - 1;
    }
}

void MaterialEditor::LoadMaterials() {
    RenderPipeline& manager = RenderPipeline::getInstance();
    if (!manager.loadMaterialsFromConfig("config/materials_config.json")) {
        feedbackMsg = "Error loading materials";
        feedbackTime = ImGui::GetTime();
    }
    else {
        std::string fileContent;
        if (FileSystem::readString("config/materials_config.json", fileContent)) {
            try {
                materialsConfigJson = nlohmann::json::parse(fileContent);
            }
            catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                materialsConfigJson.clear();
            }
        }
        materialNames.clear();
        if (materialsConfigJson.contains("materials") && materialsConfigJson["materials"].is_object()) {
            for (auto& el : materialsConfigJson["materials"].items()) {
                materialNames.push_back(el.key());
            }
        }
        selectedMaterial = std::min(selectedMaterial, (int)materialNames.size() - 1);
        if (selectedMaterial < 0) selectedMaterial = 0;
        lastKey = "";
    }
    materialesCargados = true;
}

void MaterialEditor::SaveMaterials() {
    std::string dump = materialsConfigJson.dump(4);
    FileSystem::writeString("config/materials_config.json", dump);
    // RenderPipeline::getInstance().loadMaterialsFromConfig("config/materials_config.json");
}
