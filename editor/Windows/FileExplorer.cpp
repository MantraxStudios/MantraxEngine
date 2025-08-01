#include "FileExplorer.h"
#include <core/FileSystem.h>
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
            ImGui::Text("Directory doesnot exist:\n%s", currentDir.c_str());
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

bool FileExplorer::ShowSavePopup(const std::string& root, std::string& outSelected, const std::string& extension, const std::string& defaultWindowName)
{
    namespace fs = std::filesystem;
    static std::string currentDir;
    static bool initialized = false;
    static char fileNameBuffer[256] = "";
    static char newFolderBuffer[128] = "";
    static std::string folderErrorMsg;

    // Centrar y reiniciar al abrir
    if (ImGui::IsPopupOpen(defaultWindowName.c_str()) && !initialized) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(800, 480), ImGuiCond_Appearing);
        initialized = true;
        fileNameBuffer[0] = '\0';
        newFolderBuffer[0] = '\0';
        currentDir = root;
        outSelected = root;
        fs::path(root).parent_path();
        folderErrorMsg.clear();
    }

    bool selected = false;

    if (ImGui::BeginPopupModal(defaultWindowName.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 24.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float padding = ImGui::GetStyle().WindowPadding.x;

        // --- HEADER: Crear carpeta ---
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
        if (ImGui::BeginChild("CreateFolderSection", ImVec2(0, 100), true, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::Text("Create New Folder:");

            float buttonWidth = 80.0f;
            float inputWidth = windowWidth - buttonWidth - spacing - padding * 2;

            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputText("##NewFolder", newFolderBuffer, sizeof(newFolderBuffer));

            ImGui::SameLine();
            bool canCreate = (newFolderBuffer[0] != '\0');
            if (!canCreate) ImGui::BeginDisabled();

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0)) && canCreate) {
                fs::path newFolderPath = fs::path(currentDir) / std::string(newFolderBuffer);
                if (fs::exists(newFolderPath)) {
                    folderErrorMsg = "Folder already exists!";
                }
                else {
                    std::error_code ec;
                    if (fs::create_directory(newFolderPath, ec)) {
                        folderErrorMsg.clear();
                        newFolderBuffer[0] = '\0';
                    }
                    else {
                        folderErrorMsg = "Error creating folder!";
                    }
                }
            }

            if (!canCreate) ImGui::EndDisabled();

            // Error message
            if (!folderErrorMsg.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", folderErrorMsg.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // --- NAVIGATION: Botón de volver ---
        if (currentDir != root) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.8f));
            if (ImGui::Button("<", ImVec2(80, buttonHeight))) {
                fs::path currentPath(currentDir);
                fs::path parent = currentPath.parent_path();
                // Solo permite volver si no pasamos de root
                if (parent.string().length() >= root.length() && parent.string().find(root) == 0) {
                    currentDir = parent.string();
                }
                else {
                    currentDir = root; // Nunca dejes que pase de root
                }
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        }


        // Current directory path
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Path: %s",
            fs::relative(currentDir, fs::path(root).parent_path()).string().c_str());

        ImGui::Separator();

        // --- DIRECTORY LISTING ---
        ImGui::Text("Folders:");

        // Table with proper sizing
        float actionColumnWidth = 230.0f;
        float availableWidth = windowWidth - padding;
        float nameColumnWidth = availableWidth - actionColumnWidth;

        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
        if (ImGui::BeginTable("FolderTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 120))) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, nameColumnWidth);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, actionColumnWidth);
            ImGui::TableHeadersRow();

            // Directory entries
            std::vector<fs::directory_entry> folders;
            for (const auto& entry : fs::directory_iterator(currentDir)) {
                if (entry.is_directory()) {
                    folders.push_back(entry);
                }
            }

            // Sort folders alphabetically
            std::sort(folders.begin(), folders.end(),
                [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    return a.path().filename().string() < b.path().filename().string();
                });

            for (const auto& entry : folders) {
                std::string name = entry.path().filename().string();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Folder icon and name
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui::Text("[DIR] %s", name.c_str());
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();

                // Action buttons
                ImGui::PushID(name.c_str());

                if (ImGui::Button("Open", ImVec2(100, 0))) {
                    currentDir = entry.path().string();
                    folderErrorMsg.clear(); // Clear any previous errors
                }

                ImGui::SameLine();

                // Check if folder is empty for delete button
                bool isEmpty = true;
                try {
                    isEmpty = fs::directory_iterator(entry.path()) == fs::end(fs::directory_iterator());
                }
                catch (...) {
                    isEmpty = false;
                }

                if (!isEmpty) ImGui::BeginDisabled();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 0.8f));

                if (ImGui::Button("Delete", ImVec2(100, 0))) {
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (ec) {
                        folderErrorMsg = "Cannot delete folder!";
                    }
                    else {
                        folderErrorMsg.clear();
                    }
                }

                ImGui::PopStyleColor(2);
                if (!isEmpty) ImGui::EndDisabled();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleColor();

        ImGui::Separator();

        // --- FILE NAME INPUT ---
        ImGui::Text("File Name:");

        float extensionWidth = ImGui::CalcTextSize(extension.c_str()).x + 10.0f;
        float fileInputWidth = windowWidth - extensionWidth - spacing - padding;

        ImGui::SetNextItemWidth(fileInputWidth);
        ImGui::InputText("##SaveFileName", fileNameBuffer, sizeof(fileNameBuffer));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", extension.c_str());

        // --- PREVIEW PATH ---
        std::string fullPath;
        if (fileNameBuffer[0] != '\0') {
            fs::path savePath = fs::path(currentDir) / (std::string(fileNameBuffer) + extension);
            fullPath = savePath.string();
            ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Save as: %s",
                fs::relative(fullPath, fs::path(root).parent_path()).string().c_str());
        }

        ImGui::Separator();

        // --- ACTION BUTTONS ---
        bool canSave = (fileNameBuffer[0] != '\0');
        float buttonWidth = 80.0f;
        float totalButtonWidth = buttonWidth * 2 + spacing;
        float buttonStartX = (windowWidth - totalButtonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonStartX);

        if (!canSave) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));

        if (ImGui::Button("Save", ImVec2(buttonWidth, buttonHeight * 1.2f)) && canSave) {
            outSelected = FileSystem::GetPathAfterContent(fullPath);
            selected = true;
            ImGui::CloseCurrentPopup();
            initialized = false;
        }

        ImGui::PopStyleColor(2);
        if (!canSave) ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, buttonHeight * 1.2f))) {
            ImGui::CloseCurrentPopup();
            initialized = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::EndPopup();
    }
    else {
        initialized = false;
    }

    return selected;
}


bool FileExplorer::ShowSavePopupAnimator(const std::string& root, std::string& outSelected, const std::string& extension, const std::string& defaultWindowName)
{
    namespace fs = std::filesystem;
    static std::string currentDir;
    static bool initialized = false;
    static char fileNameBuffer[256] = "";
    static char newFolderBuffer[128] = "";
    static std::string folderErrorMsg;

    // Centrar y reiniciar al abrir
    if (ImGui::IsPopupOpen(defaultWindowName.c_str()) && !initialized) {
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(800, 480), ImGuiCond_Appearing);
        initialized = true;
        fileNameBuffer[0] = '\0';
        newFolderBuffer[0] = '\0';
        currentDir = root;
        outSelected = root;
        fs::path(root).parent_path();
        folderErrorMsg.clear();
    }

    bool selected = false;

    if (ImGui::BeginPopupModal(defaultWindowName.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        float windowWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = 24.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float padding = ImGui::GetStyle().WindowPadding.x;

        // --- HEADER: Crear carpeta ---
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.3f));
        if (ImGui::BeginChild("CreateFolderSection", ImVec2(0, 100), true, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::Text("Create New Folder:");

            float buttonWidth = 80.0f;
            float inputWidth = windowWidth - buttonWidth - spacing - padding * 2;

            ImGui::SetNextItemWidth(inputWidth);
            ImGui::InputText("##NewFolder", newFolderBuffer, sizeof(newFolderBuffer));

            ImGui::SameLine();
            bool canCreate = (newFolderBuffer[0] != '\0');
            if (!canCreate) ImGui::BeginDisabled();

            if (ImGui::Button("Create", ImVec2(buttonWidth, 0)) && canCreate) {
                fs::path newFolderPath = fs::path(currentDir) / std::string(newFolderBuffer);
                if (fs::exists(newFolderPath)) {
                    folderErrorMsg = "Folder already exists!";
                }
                else {
                    std::error_code ec;
                    if (fs::create_directory(newFolderPath, ec)) {
                        folderErrorMsg.clear();
                        newFolderBuffer[0] = '\0';
                    }
                    else {
                        folderErrorMsg = "Error creating folder!";
                    }
                }
            }

            if (!canCreate) ImGui::EndDisabled();

            // Error message
            if (!folderErrorMsg.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", folderErrorMsg.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // --- NAVIGATION: Botón de volver ---
        if (currentDir != root) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.8f));
            if (ImGui::Button("<", ImVec2(80, buttonHeight))) {
                fs::path currentPath(currentDir);
                fs::path parent = currentPath.parent_path();
                // Solo permite volver si no pasamos de root
                if (parent.string().length() >= root.length() && parent.string().find(root) == 0) {
                    currentDir = parent.string();
                }
                else {
                    currentDir = root; // Nunca dejes que pase de root
                }
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
        }


        // Current directory path
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Path: %s",
            fs::relative(currentDir, fs::path(root).parent_path()).string().c_str());

        ImGui::Separator();

        // --- DIRECTORY LISTING ---
        ImGui::Text("Folders:");

        // Table with proper sizing
        float actionColumnWidth = 230.0f;
        float availableWidth = windowWidth - padding;
        float nameColumnWidth = availableWidth - actionColumnWidth;

        ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
        if (ImGui::BeginTable("FolderTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 120))) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, nameColumnWidth);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, actionColumnWidth);
            ImGui::TableHeadersRow();

            // Directory entries
            std::vector<fs::directory_entry> folders;
            for (const auto& entry : fs::directory_iterator(currentDir)) {
                if (entry.is_directory()) {
                    folders.push_back(entry);
                }
            }

            // Sort folders alphabetically
            std::sort(folders.begin(), folders.end(),
                [](const fs::directory_entry& a, const fs::directory_entry& b) {
                    return a.path().filename().string() < b.path().filename().string();
                });

            for (const auto& entry : folders) {
                std::string name = entry.path().filename().string();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                // Folder icon and name
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui::Text("[DIR] %s", name.c_str());
                ImGui::PopStyleColor();

                ImGui::TableNextColumn();

                // Action buttons
                ImGui::PushID(name.c_str());

                if (ImGui::Button("Open", ImVec2(100, 0))) {
                    currentDir = entry.path().string();
                    folderErrorMsg.clear(); // Clear any previous errors
                }

                ImGui::SameLine();

                // Check if folder is empty for delete button
                bool isEmpty = true;
                try {
                    isEmpty = fs::directory_iterator(entry.path()) == fs::end(fs::directory_iterator());
                }
                catch (...) {
                    isEmpty = false;
                }

                if (!isEmpty) ImGui::BeginDisabled();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 0.8f));

                if (ImGui::Button("Delete", ImVec2(100, 0))) {
                    std::error_code ec;
                    fs::remove(entry.path(), ec);
                    if (ec) {
                        folderErrorMsg = "Cannot delete folder!";
                    }
                    else {
                        folderErrorMsg.clear();
                    }
                }

                ImGui::PopStyleColor(2);
                if (!isEmpty) ImGui::EndDisabled();

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::PopStyleColor();

        ImGui::Separator();

        // --- FILE NAME INPUT ---
        ImGui::Text("File Name:");

        float extensionWidth = ImGui::CalcTextSize(extension.c_str()).x + 10.0f;
        float fileInputWidth = windowWidth - extensionWidth - spacing - padding;

        ImGui::SetNextItemWidth(fileInputWidth);
        ImGui::InputText("##SaveFileName", fileNameBuffer, sizeof(fileNameBuffer));
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", extension.c_str());

        // --- PREVIEW PATH ---
        std::string fullPath;
        if (fileNameBuffer[0] != '\0') {
            fs::path savePath = fs::path(currentDir) / (std::string(fileNameBuffer) + extension);
            fullPath = savePath.string();
            ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Save as: %s",
                fs::relative(fullPath, fs::path(root).parent_path()).string().c_str());
        }

        ImGui::Separator();

        // --- ACTION BUTTONS ---
        bool canSave = (fileNameBuffer[0] != '\0');
        float buttonWidth = 80.0f;
        float totalButtonWidth = buttonWidth * 2 + spacing;
        float buttonStartX = (windowWidth - totalButtonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonStartX);

        if (!canSave) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));

        if (ImGui::Button("Save", ImVec2(buttonWidth, buttonHeight * 1.2f)) && canSave) {
            outSelected = FileSystem::GetPathAfterContent(fullPath);
            selected = true;
            ImGui::CloseCurrentPopup();
            initialized = false;
        }

        ImGui::PopStyleColor(2);
        if (!canSave) ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, buttonHeight * 1.2f))) {
            ImGui::CloseCurrentPopup();
            initialized = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::EndPopup();
    }
    else {
        initialized = false;
    }

    return selected;
}