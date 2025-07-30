#include "FileExplorer.h"
#include <filesystem>

namespace fs = std::filesystem;

bool FileExplorer::ShowPopup(const std::string& root, std::string& outSelected, const std::string& extension)
{
    // Static vars keep state between frames and popups
    static std::string currentDir;
    static bool initialized = false;

    // This is key: Reset to root when popup opens!
    if (ImGui::IsPopupOpen("File Explorer") && !initialized) {
        currentDir = root;
        initialized = true;
    }

    bool selected = false;

    if (ImGui::BeginPopupModal("File Explorer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Safety: Check folder exists
        if (!fs::exists(currentDir) || !fs::is_directory(currentDir)) {
            ImGui::Text("Directory does not exist:\n%s", currentDir.c_str());
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                initialized = false;
            }
            ImGui::EndPopup();
            return false;
        }

        // Helper function para obtener ruta relativa desde Content (INCLUYENDO Content)
        auto getRelativePath = [&](const std::string& fullPath) -> std::string {
            fs::path full(fullPath);
            fs::path rootPath(root);

            try {
                // Obtener la ruta relativa desde el padre de Content
                fs::path relative = fs::relative(full, rootPath.parent_path());
                return relative.string();
            }
            catch (...) {
                return full.filename().string();
            }
            };

        // Back button - no permitir salir del directorio raíz
        if (currentDir != root) {
            if (ImGui::Button(".. (Back)")) {
                fs::path current(currentDir);
                if (current.parent_path().string().length() >= root.length()) {
                    currentDir = current.parent_path().string();
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Current: %s", getRelativePath(currentDir).c_str());
        ImGui::Separator();

        // Scroll region para el explorador
        ImGui::BeginChild("FileList", ImVec2(400, 300), true);

        // List directories first
        for (const auto& entry : fs::directory_iterator(currentDir)) {
            if (entry.is_directory()) {
                std::string name = entry.path().filename().string();
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
                if (ImGui::Selectable((name + "/").c_str(), false)) {
                    currentDir = entry.path().string();
                }
                ImGui::PopStyleColor();
            }
        }

        // List files matching extension
        for (const auto& entry : fs::directory_iterator(currentDir)) {
            if (entry.is_regular_file() && (extension.empty() || entry.path().extension() == extension)) {
                std::string name = entry.path().filename().string();
                if (ImGui::Selectable(name.c_str(), false)) {
                    // Devolver ruta relativa DESDE Content (incluyendo Content)
                    outSelected = getRelativePath(entry.path().string());
                    selected = true;
                    ImGui::CloseCurrentPopup();
                    initialized = false;
                }
            }
        }

        ImGui::EndChild();

        if (!outSelected.empty()) {
            ImGui::Text("Selected: %s", outSelected.c_str());
        }

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            initialized = false;
        }

        ImGui::EndPopup();
    }
    else {
        initialized = false;
    }

    return selected;
}

bool FileExplorer::ShowInline(const std::string& root, std::string& outSelected, const std::string& extension)
{
    static std::string currentDir;
    static bool initialized = false;

    // Inicializar directorio
    if (!initialized) {
        currentDir = root;
        initialized = true;
    }

    bool selected = false;

    // Safety: Check folder exists
    if (!fs::exists(currentDir) || !fs::is_directory(currentDir)) {
        ImGui::Text("Directory does not exist:\n%s", currentDir.c_str());
        if (ImGui::Button("Reset to Root")) {
            currentDir = root;
        }
        return false;
    }

    // Back button
    if (currentDir != fs::path(currentDir).root_path().string()) {
        if (ImGui::Button(".. (Back)")) {
            currentDir = fs::path(currentDir).parent_path().string();
        }
    }

    ImGui::Separator();
    ImGui::Text("Current: %s", currentDir.c_str());
    ImGui::Separator();

    // Scroll region para el explorador
    ImGui::BeginChild("FileList", ImVec2(400, 300), true);

    // List directories first
    for (const auto& entry : fs::directory_iterator(currentDir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 200, 255, 255));
            if (ImGui::Selectable((name + "/").c_str(), false)) {
                currentDir = entry.path().string();
            }
            ImGui::PopStyleColor();
        }
    }

    // List files matching extension
    for (const auto& entry : fs::directory_iterator(currentDir)) {
        if (entry.is_regular_file() && (extension.empty() || entry.path().extension() == extension)) {
            std::string name = entry.path().filename().string();
            if (ImGui::Selectable(name.c_str(), false)) {
                outSelected = entry.path().string();
                selected = true;
                initialized = false; // Reset para próxima vez
            }
        }
    }

    ImGui::EndChild();

    if (!outSelected.empty()) {
        ImGui::Text("Selected: %s", outSelected.c_str());
    }

    return selected;
}
