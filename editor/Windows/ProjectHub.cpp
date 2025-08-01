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
    
    // === DISEÑO MEJORADO DEL PROJECT HUB ===
    
    // Fondo con gradiente usando colores del tema ImGui
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Gradiente de fondo usando colores del tema ImGui
    ImVec2 gradient_start = window_pos;
    ImVec2 gradient_end = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
    draw_list->AddRectFilledMultiColor(
        gradient_start, gradient_end,
        IM_COL32(59, 61, 64, 255),   // Top-left: darkBg (0.23f, 0.24f, 0.25f)
        IM_COL32(71, 74, 77, 255),   // Top-right: mediumBg (0.28f, 0.29f, 0.30f)
        IM_COL32(82, 84, 87, 255),   // Bottom-right: lightBg (0.32f, 0.33f, 0.34f)
        IM_COL32(66, 68, 71, 255)    // Bottom-left: mezcla de colores
    );
    
    // Líneas decorativas animadas
    float time = ImGui::GetTime();
    float line_alpha = 0.3f + 0.2f * sinf(time * 2.0f);
    
    // Líneas horizontales decorativas usando accentBlue del tema
    for (int i = 0; i < 3; i++) {
        float y_pos = window_pos.y + (window_size.y * 0.1f) + (i * 120.0f);
        float line_width = window_size.x * 0.6f;
        float x_offset = sinf(time * 1.5f + i) * 30.0f;
        
        draw_list->AddLine(
            ImVec2(window_pos.x + (window_size.x - line_width) * 0.5f + x_offset, y_pos),
            ImVec2(window_pos.x + (window_size.x + line_width) * 0.5f + x_offset, y_pos),
            IM_COL32(66, 150, 250, (int)(255 * line_alpha))  // accentBlue del tema
        );
    }
    
    // Título principal con efecto de sombra
    const char* title_text = "MANTRAX ENGINE";
    ImFont* bigFont = io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : io.FontDefault;
    ImGui::PushFont(bigFont);
    
    ImVec2 title_size = ImGui::CalcTextSize(title_text);
    ImVec2 title_pos(
        (display_size.x - title_size.x) * 0.5f,
        (display_size.y - title_size.y) * 0.15f
    );
    
    // Sombra del texto
    ImGui::SetCursorPos(ImVec2(title_pos.x + 3, title_pos.y + 3));
    ImGui::TextColored(ImVec4(0, 0, 0, 0.5f), title_text);
    
    // Texto principal usando textWhite del tema
    ImGui::SetCursorPos(title_pos);
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), title_text);  // textWhite del tema
    
    ImGui::PopFont();
    
    // Subtítulo con animación
    const char* subtitle_text = "Project Hub";
    ImVec2 subtitle_size = ImGui::CalcTextSize(subtitle_text);
    ImVec2 subtitle_pos(
        (display_size.x - subtitle_size.x) * 0.5f,
        title_pos.y + title_size.y + 20.0f
    );
    
    float subtitle_alpha = 0.7f + 0.3f * sinf(time * 3.0f);
    ImGui::SetCursorPos(subtitle_pos);
    ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, subtitle_alpha), subtitle_text);  // textGray del tema
    
    // Contenedor principal para los proyectos
    float container_width = display_size.x * 0.6f;
    float container_height = display_size.y * 0.4f;
    ImVec2 container_pos(
        (display_size.x - container_width) * 0.5f,
        subtitle_pos.y + subtitle_size.y + 40.0f
    );
    
    // Fondo del contenedor de proyectos
    draw_list->AddRectFilled(
        ImVec2(container_pos.x, container_pos.y),
        ImVec2(container_pos.x + container_width, container_pos.y + container_height),
        IM_COL32(41, 43, 46, 200),  // FrameBg del tema con transparencia
        8.0f
    );
    
    // Borde del contenedor
    draw_list->AddRect(
        ImVec2(container_pos.x, container_pos.y),
        ImVec2(container_pos.x + container_width, container_pos.y + container_height),
        IM_COL32(66, 150, 250, 100),  // accentBlue del tema
        8.0f, 0, 2.0f
    );
    
    // Lista de proyectos
    ImGui::SetCursorPos(ImVec2(container_pos.x + 20, container_pos.y + 20));
    ImGui::TextColored(ImVec4(0.95f, 0.95f, 0.95f, 1.0f), "Projects in: %s", basePath.c_str());
    
    ImGui::SetCursorPos(ImVec2(container_pos.x + 20, container_pos.y + 50));
    ImGui::BeginChild("ProjectsList", ImVec2(container_width - 40, container_height - 120), true);
    
    for (size_t i = 0; i < projects.size(); ++i) {
        char label[256];
        snprintf(label, sizeof(label), "%s##%zu", projects[i].name.c_str(), i);
        
        // Estilo personalizado para los proyectos
        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(71, 74, 77, 255));  // mediumBg
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(82, 84, 87, 255));  // lightBg
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(66, 150, 250, 255));  // accentBlue
        
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
    }
    
    ImGui::EndChild();
    
    // Controles en la parte inferior
    float controls_y = container_pos.y + container_height + 20.0f;
    
    // Input para nuevo proyecto
    ImGui::SetCursorPos(ImVec2(container_pos.x, controls_y));
    ImGui::PushItemWidth(container_width * 0.4f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(41, 43, 46, 255));  // FrameBg del tema
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(51, 53, 56, 255));  // FrameBgHovered del tema
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(66, 150, 250, 100));  // accentBlue con transparencia
    ImGui::InputText("New Project", newProjectName, IM_ARRAYSIZE(newProjectName));
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();
    
    // Botones
    ImGui::SameLine();
    ImGui::SetCursorPosX(container_pos.x + container_width * 0.45f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(71, 74, 77, 255));  // mediumBg
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(82, 84, 87, 255));  // lightBg
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(66, 150, 250, 255));  // accentBlue
    
    if (ImGui::Button("Add Project", ImVec2(120, 0))) {
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
    if (ImGui::Button("Delete Selected", ImVec2(120, 0)) && selected >= 0 && selected < static_cast<int>(projects.size())) {
        fs::remove_all(projects[selected].path);
        projects.erase(projects.begin() + selected);
        selected = -1;
    }
    
    ImGui::PopStyleColor(3);
    
    // Efecto de partículas flotantes usando accentOrange del tema
    for (int i = 0; i < 10; i++) {
        float particle_x = fmodf(sinf(time * 0.5f + i * 0.7f) * window_size.x * 0.8f + window_size.x * 0.1f, window_size.x);
        float particle_y = fmodf(cosf(time * 0.3f + i * 0.5f) * window_size.y * 0.6f + window_size.y * 0.2f, window_size.y);
        float particle_alpha = 0.3f + 0.4f * sinf(time * 2.0f + i);
        
        draw_list->AddCircleFilled(
            ImVec2(window_pos.x + particle_x, window_pos.y + particle_y),
            2.0f,
            IM_COL32(255, 161, 0, (int)(255 * particle_alpha))  // accentOrange del tema
        );
    }
    
    ImGui::End();
}
