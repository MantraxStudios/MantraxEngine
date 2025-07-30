// projecthub.cpp
#include "projecthub.h"
#include <imgui/imgui.h>
#include <filesystem>
#include <cstring>
#include "../EUI/EditorInfo.h"
#include <core/FileSystem.h>

#include "render/MaterialManager.h"

namespace fs = std::filesystem;

ProjectHub::ProjectHub(const std::string& baseDirectory)
    : basePath(baseDirectory)
{
    readProjects();
    newProjectName[0] = '\0';
}

void ProjectHub::readProjects() {
    projects.clear();
    std::cout << "[DEBUG] Reading projects in: " << basePath << std::endl;
    if (!fs::exists(basePath)) {
        std::cout << "[DEBUG] Base path does not exist. Creating: " << basePath << std::endl;
        fs::create_directories(basePath);
    }
    for (const auto& entry : fs::directory_iterator(basePath)) {
        std::cout << "[DEBUG] Found entry: " << entry.path() << std::endl;
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            std::string path = entry.path().string();
            std::cout << "[DEBUG] Adding project: " << name << " (" << path << ")" << std::endl;
            projects.emplace_back(name, path);
        }
    }
    std::cout << "[DEBUG] Total projects found: " << projects.size() << std::endl;
}


void ProjectHub::reloadProjects() {
    readProjects();
}

void ProjectHub::render() {
    ImGui::Begin("Project Hub");

    ImGui::Text("Projects in: %s", basePath.c_str());
    for (size_t i = 0; i < projects.size(); ++i) {
        char label[256];
        snprintf(label, sizeof(label), "%s##%zu", projects[i].name.c_str(), i);
        if (ImGui::Selectable(label, selected == static_cast<int>(i))) {
            selected = static_cast<int>(i);
            FileSystem::projectPath = FileSystem::workDirectory().string() + "\\" + EditorInfo::SelectedProjectPath;

            EditorInfo::SelectedProjectPath = fs::absolute(projects[i].path).string();
            FileSystem::projectPath = fs::absolute(projects[i].path).string();


            MaterialManager::getInstance().clearMaterials();
            MaterialManager::getInstance().loadMaterialsFromConfig("config/materials_config.json");

            std::cout << "Select Project: " << EditorInfo::SelectedProject << std::endl;
            std::cout << "Project Path: " << EditorInfo::SelectedProjectPath << std::endl;
        }
    }

    ImGui::Separator();

    ImGui::InputText("New Project", newProjectName, IM_ARRAYSIZE(newProjectName));
    if (ImGui::Button("Add Project")) {
        if (std::strlen(newProjectName) > 0) {
            std::string newPath = basePath + "/" + newProjectName;
            if (!fs::exists(newPath)) {
                fs::create_directory(newPath);
                projects.emplace_back(newProjectName, newPath);
            }
            newProjectName[0] = '\0';
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Selected Project") && selected >= 0 && selected < static_cast<int>(projects.size())) {
        fs::remove_all(projects[selected].path);
        projects.erase(projects.begin() + selected);
        selected = -1;
    }

    ImGui::End();
}
