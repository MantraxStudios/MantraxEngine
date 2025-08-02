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
    // Obtener el tamaño de la pantalla
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 display_size = io.DisplaySize;
    
    // Configurar la ventana para ocupar toda la pantalla
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(display_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    
    ImGui::Begin("Project Hub", nullptr, flags);
    
    // === DISEÑO LIMPIO Y PROFESIONAL ===
    
    // Fondo simple y limpio
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Fondo principal
    draw_list->AddRectFilled(
        window_pos,
        ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y),
        IM_COL32(30, 30, 30, 255)  // Fondo oscuro simple
    );
    
    // Header simple
    float header_height = 60.0f;
    draw_list->AddRectFilled(
        window_pos,
        ImVec2(window_pos.x + window_size.x, window_pos.y + header_height),
        IM_COL32(45, 45, 45, 255)  // Header más claro
    );
    
    // Línea separadora
    draw_list->AddLine(
        ImVec2(window_pos.x, window_pos.y + header_height),
        ImVec2(window_pos.x + window_size.x, window_pos.y + header_height),
        IM_COL32(60, 60, 60, 255),
        1.0f
    );
    
    // Título simple
    const char* title_text = "Mantrax Engine";
    ImVec2 title_size = ImGui::CalcTextSize(title_text);
    ImVec2 title_pos(
        window_pos.x + 20.0f,
        window_pos.y + (header_height - title_size.y) * 0.5f
    );
    
    draw_list->AddText(
        title_pos,
        IM_COL32(255, 255, 255, 255),
        title_text
    );
    
    // Área principal
    float content_y = window_pos.y + header_height + 20.0f;
    float content_height = window_size.y - header_height - 40.0f;
    
    // === PANEL IZQUIERDO - Proyectos ===
    float left_panel_width = 350.0f;
    ImGui::SetCursorPos(ImVec2(20.0f, header_height + 20.0f));
    ImGui::BeginChild("ProjectsPanel", ImVec2(left_panel_width, content_height), true);
    
    // Título del panel
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Projects");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Lista de proyectos
    for (size_t i = 0; i < projects.size(); ++i) {
        char label[256];
        snprintf(label, sizeof(label), "%s##%zu", projects[i].name.c_str(), i);
        
        // Estilo simple para los proyectos
        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(50, 50, 50, 255));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(70, 70, 70, 255));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 120, 215, 255));
        
        if (ImGui::Selectable(label, selected == static_cast<int>(i), ImGuiSelectableFlags_AllowDoubleClick)) {
            selected = static_cast<int>(i);
            FileSystem::projectPath = FileSystem::workDirectory().string() + "\\" + EditorInfo::SelectedProjectPath;

            EditorInfo::SelectedProjectPath = fs::absolute(projects[i].path).string();
            FileSystem::projectPath = fs::absolute(projects[i].path).string();

            MaterialManager::getInstance().clearMaterials();
            MaterialManager::getInstance().loadMaterialsFromConfig("config/materials_config.json");

            std::cout << "Select Project: " << EditorInfo::SelectedProject << std::endl;
            std::cout << "Project Path: " << EditorInfo::SelectedProjectPath << std::endl;
        }
        
        ImGui::PopStyleColor(3);
        
        // Mostrar ruta si está seleccionado
        if (selected == static_cast<int>(i)) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "  %s", projects[i].path.c_str());
        }
    }
    
    ImGui::EndChild();
    
    // === PANEL DERECHO - Crear Proyecto ===
    float right_panel_x = left_panel_width + 40.0f;
    float right_panel_width = window_size.x - right_panel_x - 20.0f;
    
    ImGui::SetCursorPos(ImVec2(right_panel_x, header_height + 20.0f));
    ImGui::BeginChild("CreatePanel", ImVec2(right_panel_width, content_height), true);
    
    // Título del panel derecho
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Create New Project");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Formulario simple
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Project Name:");
    ImGui::PushItemWidth(right_panel_width - 40.0f);
    ImGui::InputText("##ProjectName", newProjectName, IM_ARRAYSIZE(newProjectName));
    ImGui::PopItemWidth();
    
    ImGui::Spacing();
    
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Location:");
    ImGui::PushItemWidth(right_panel_width - 40.0f);
    ImGui::InputText("##ProjectLocation", (char*)basePath.c_str(), basePath.length(), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Botones
    if (ImGui::Button("Create Project", ImVec2(120, 30))) {
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
    if (ImGui::Button("Browse", ImVec2(80, 30))) {
        // TODO: Implementar diálogo de selección de carpeta
    }
    
    // Botón de eliminar
    if (selected >= 0 && selected < static_cast<int>(projects.size())) {
        ImGui::SameLine();
        if (ImGui::Button("Delete Selected", ImVec2(100, 30))) {
            fs::remove_all(projects[selected].path);
            projects.erase(projects.begin() + selected);
            selected = -1;
        }
    }
    
    ImGui::EndChild();
    
    ImGui::End();
}
