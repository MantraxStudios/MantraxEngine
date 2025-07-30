#include "ContentBrowser.h"
#include "core/FileSystem.h"
#include "render/Texture.h"
#include <filesystem>
#include <algorithm>
#include <imgui/imgui.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "../SceneSaver.h"
#include "../EUI/EditorInfo.h"

namespace fs = std::filesystem;

// Estructura para almacenar información de archivos de forma segura
struct FileEntry {
    std::string path;
    std::string name;
    bool isDirectory;
    uintmax_t size;

    FileEntry() : isDirectory(false), size(0) {}
    FileEntry(const std::string& p, const std::string& n, bool isDir, uintmax_t s = 0)
        : path(p), name(n), isDirectory(isDir), size(s) {
    }
};

// Variables miembro de la clase Assets (deberías agregarlas al header)
// Si no puedes modificar el header, usa variables estáticas con inicialización segura
static std::string* s_currentPath = nullptr;
static std::vector<FileEntry>* s_currentEntries = nullptr;
static bool s_needsRefresh = true;
static std::string* s_selectedFile = nullptr;
static bool s_initialized = false;
static std::string* s_contentRootPath = nullptr;

// Cache para texturas de preview usando la clase Texture del core
static std::map<std::string, Texture*>* s_textureCache = nullptr;
static std::map<std::string, ImVec2>* s_textureSizes = nullptr;

// Función para inicializar de forma segura
void InitializeAssetsBrowser() {
    if (!s_initialized) {
        s_currentPath = new std::string();
        s_currentEntries = new std::vector<FileEntry>();
        s_selectedFile = new std::string();
        s_contentRootPath = new std::string();
        s_textureCache = new std::map<std::string, Texture*>();
        s_textureSizes = new std::map<std::string, ImVec2>();
        s_initialized = true;
    }
}

// Función para limpiar memoria (llamar en destructor o shutdown)
void CleanupAssetsBrowser() {
    if (s_initialized) {
        delete s_currentPath;
        delete s_currentEntries;
        delete s_selectedFile;
        delete s_contentRootPath;
        
        // Limpiar cache de texturas
        if (s_textureCache) {
            for (auto& pair : *s_textureCache) {
                delete pair.second;
            }
            delete s_textureCache;
        }
        delete s_textureSizes;
        
        s_currentPath = nullptr;
        s_currentEntries = nullptr;
        s_selectedFile = nullptr;
        s_contentRootPath = nullptr;
        s_textureCache = nullptr;
        s_textureSizes = nullptr;
        s_initialized = false;
    }
}

// Función para verificar si una ruta está dentro del directorio Content
bool IsPathWithinContent(const std::string& path) {
    if (!s_contentRootPath || s_contentRootPath->empty()) return false;
    
    try {
        fs::path contentPath(*s_contentRootPath);
        fs::path checkPath(path);
        
        // Normalizar las rutas
        contentPath = contentPath.lexically_normal();
        checkPath = checkPath.lexically_normal();
        
        // Verificar si checkPath es un subdirectorio de contentPath
        auto relative = checkPath.lexically_relative(contentPath);
        return !relative.empty() && relative.native()[0] != '.';
    }
    catch (...) {
        return false;
    }
}

// Función para obtener el path relativo desde Content
std::string GetRelativePathFromContent(const std::string& fullPath) {
    if (!s_contentRootPath || s_contentRootPath->empty()) return fullPath;
    
    try {
        fs::path contentPath(*s_contentRootPath);
        fs::path checkPath(fullPath);
        
        auto relative = checkPath.lexically_relative(contentPath);
        if (!relative.empty() && relative.native()[0] != '.') {
            return relative.string();
        }
    }
    catch (...) {
        // Si hay error, devolver la ruta completa
    }
    
    return fullPath;
}

// Función para verificar si un archivo es una imagen
bool IsImageFile(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return false;
    
    std::string extension = filename.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    return (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
            extension == ".bmp" || extension == ".tga" || extension == ".tiff" ||
            extension == ".gif" || extension == ".webp");
}

// Función para cargar textura de imagen usando la clase Texture del core
Texture* LoadImageTexture(const std::string& imagePath) {
    try {
        // Crear una nueva textura usando la clase Texture del core
        Texture* texture = new Texture();
        
        // Intentar cargar la imagen
        if (texture->loadFromFile(imagePath)) {
            return texture;
        } else {
            // Si falla la carga, eliminar la textura y retornar nullptr
            delete texture;
            return nullptr;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading texture: " << imagePath << " - " << e.what() << std::endl;
        return nullptr;
    }
}

// Función para obtener el tamaño de la textura
ImVec2 GetTextureSize(const std::string& imagePath) {
    auto it = s_textureCache->find(imagePath);
    if (it != s_textureCache->end() && it->second) {
        return ImVec2(static_cast<float>(it->second->getWidth()), 
                     static_cast<float>(it->second->getHeight()));
    }
    return ImVec2(128, 128); // Tamaño por defecto
}

// Función para refrescar el contenido del directorio actual
void RefreshDirectory() {
    if (!s_currentEntries || !s_currentPath) return;

    s_currentEntries->clear();

    if (s_currentPath->empty()) return;

    try {
        if (!fs::exists(*s_currentPath) || !fs::is_directory(*s_currentPath)) {
            return;
        }

        for (const auto& entry : fs::directory_iterator(*s_currentPath)) {
            try {
                std::string filename = entry.path().filename().string();
                bool isDir = entry.is_directory();
                uintmax_t size = 0;

                if (!isDir) {
                    try {
                        size = fs::file_size(entry.path());
                    }
                    catch (...) {
                        size = 0;
                    }
                }

                s_currentEntries->emplace_back(
                    entry.path().string(),
                    filename,
                    isDir,
                    size
                );

            }
            catch (const std::exception& e) {
                // Saltar archivos problemáticos
                continue;
            }
        }

        // Ordenar de forma segura
        if (!s_currentEntries->empty()) {
            std::sort(s_currentEntries->begin(), s_currentEntries->end(),
                [](const FileEntry& a, const FileEntry& b) {
                    if (a.isDirectory != b.isDirectory) {
                        return a.isDirectory > b.isDirectory;
                    }
                    return a.name < b.name;
                });
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error al leer directorio: " << e.what() << std::endl;
    }

    s_needsRefresh = false;
}

// Función para obtener el icono según el tipo de archivo
const char* GetFileIcon(const FileEntry& entry) {
    if (entry.isDirectory) {
        return "[DIR]";
    }

    size_t dotPos = entry.name.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "[FILE]";
    }

    std::string extension = entry.name.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
        extension == ".bmp" || extension == ".tga") {
        return "[IMG]";
    }
    else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
        return "[SND]";
    }
    else if (extension == ".obj" || extension == ".fbx" || extension == ".dae") {
        return "[3D]";
    }
    else if (extension == ".txt" || extension == ".md") {
        return "[TXT]";
    }
    else if (extension == ".cpp" || extension == ".h" || extension == ".c") {
        return "[CODE]";
    }
    else if (extension == ".shader" || extension == ".glsl") {
        return "[SHDR]";
    }

    return "[FILE]";
}

// Función para formatear el tamaño de archivo
std::string FormatFileSize(uintmax_t size) {
    if (size == 0) return "0 B";

    const char* units[] = { "B", "KB", "MB", "GB" };
    int unitIndex = 0;
    double fileSize = static_cast<double>(size);

    while (fileSize >= 1024.0 && unitIndex < 3) {
        fileSize /= 1024.0;
        unitIndex++;
    }

    char buffer[32];
    if (unitIndex == 0) {
        snprintf(buffer, sizeof(buffer), "%.0f %s", fileSize, units[unitIndex]);
    }
    else {
        snprintf(buffer, sizeof(buffer), "%.1f %s", fileSize, units[unitIndex]);
    }

    return std::string(buffer);
}

// Función para renderizar breadcrumbs
void RenderBreadcrumbs() {
    if (!s_currentPath || !s_contentRootPath) return;
    
    try {
        fs::path currentPath(*s_currentPath);
        fs::path contentPath(*s_contentRootPath);
        
        std::vector<std::string> breadcrumbs;
        std::vector<std::string> breadcrumbPaths;
        
        // Construir breadcrumbs desde Content hasta la ruta actual
        fs::path relativePath = currentPath.lexically_relative(contentPath);
        
        if (!relativePath.empty() && relativePath.native()[0] != '.') {
            // Agregar Content como primer breadcrumb
            breadcrumbs.push_back("Content");
            breadcrumbPaths.push_back(contentPath.string());
            
            // Agregar cada parte del path
            for (const auto& part : relativePath) {
                breadcrumbs.push_back(part.string());
                breadcrumbPaths.push_back((contentPath / relativePath).string());
            }
        } else {
            breadcrumbs.push_back("Content");
            breadcrumbPaths.push_back(contentPath.string());
        }
        
        // Renderizar breadcrumbs
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
        
        for (size_t i = 0; i < breadcrumbs.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), " > ");
                ImGui::SameLine();
            }
            
            if (ImGui::Button(breadcrumbs[i].c_str())) {
                if (i < breadcrumbPaths.size()) {
                    *s_currentPath = breadcrumbPaths[i];
                    s_needsRefresh = true;
                    s_selectedFile->clear();
                }
            }
        }
        
        ImGui::PopStyleVar();
        
    } catch (...) {
        ImGui::Text("Error rendering breadcrumbs");
    }
}

// Función para renderizar preview de imagen
void RenderImagePreview(const std::string& imagePath) {
    if (!s_textureCache || !s_textureSizes) return;
    
    // Verificar si la textura ya está en cache
    auto it = s_textureCache->find(imagePath);
    Texture* texture = nullptr;
    ImVec2 textureSize;
    
    if (it == s_textureCache->end()) {
        // Cargar la textura si no está en cache
        texture = LoadImageTexture(imagePath);
        if (texture) {
            (*s_textureCache)[imagePath] = texture;
            (*s_textureSizes)[imagePath] = ImVec2(static_cast<float>(texture->getWidth()), 
                                                  static_cast<float>(texture->getHeight()));
        }
    } else {
        texture = it->second;
        textureSize = (*s_textureSizes)[imagePath];
    }
    
    if (texture) {
        // Obtener el tamaño real de la textura
        textureSize = ImVec2(static_cast<float>(texture->getWidth()), 
                           static_cast<float>(texture->getHeight()));
        
        // Calcular tamaño del preview (máximo 128x128)
        float maxSize = 128.0f;
        ImVec2 previewSize = textureSize;
        
        if (previewSize.x > maxSize || previewSize.y > maxSize) {
            float scale = maxSize / std::max(previewSize.x, previewSize.y);
            previewSize.x *= scale;
            previewSize.y *= scale;
        }
        
        // Convertir el ID de OpenGL a ImTextureID para ImGui
        ImTextureID textureId = (ImTextureID)(intptr_t)texture->getID();
        
        // Renderizar la imagen con coordenadas UV invertidas en Y para corregir la orientación
        ImGui::Image(textureId, previewSize, ImVec2(0, 1), ImVec2(1, 0));
        
        // Mostrar información adicional
        ImGui::Text("Size: %.0fx%.0f", textureSize.x, textureSize.y);
        ImGui::Text("File: %s", fs::path(imagePath).filename().string().c_str());
    } else {
        // Si no se puede cargar la textura, mostrar un placeholder
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[Preview not available]");
        ImGui::Text("File: %s", fs::path(imagePath).filename().string().c_str());
    }
}

// Función para renderizar el TreeView
void RenderTreeView() {
    if (!s_currentEntries || s_currentEntries->empty()) {
        ImGui::Text("No files found");
        return;
    }

    // Configurar el área del TreeView
    ImGui::BeginChild("TreeView", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    for (size_t i = 0; i < s_currentEntries->size(); ++i) {
        const FileEntry& entry = (*s_currentEntries)[i];
        
        ImGui::PushID(static_cast<int>(i));
        
        // Determinar si el nodo está seleccionado
        bool isSelected = (s_selectedFile && *s_selectedFile == entry.path);
        
        // Configurar el estilo del nodo
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.6f, 1.0f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.6f, 1.0f, 0.5f));
        }
        
        // Crear el texto del nodo
        std::string displayText = std::string(GetFileIcon(entry)) + " " + entry.name;
        
        // Para todos los elementos, usar Selectable para mejor visibilidad
        if (ImGui::Selectable(displayText.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (s_selectedFile) {
                *s_selectedFile = entry.path;
            }

            // --- Abrir escena si es .scene ---
            if (!entry.isDirectory) {
                std::string extension;
                size_t dotPos = entry.name.find_last_of('.');
                if (dotPos != std::string::npos) {
                    extension = entry.name.substr(dotPos);
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                }

                // Solo con click (NO doble click) -- o puedes usar doble click si prefieres
                if (extension == ".scene") {
                    // Puedes preguntar al usuario (popup), o simplemente abrir
                    if (
                        EditorInfo::currentScenePath = entry.path;
                        SceneSaver::LoadScene(entry.path)) {
                        std::cout << "Escena cargada: " << entry.name << std::endl;
                    }
                    else {
                        std::cerr << "Error cargando escena: " << entry.path << std::endl;
                    }
                }
            }

            // Doble click para navegar en carpetas
            if (ImGui::IsMouseDoubleClicked(0) && entry.isDirectory) {
                *s_currentPath = entry.path;
                s_needsRefresh = true;
                s_selectedFile->clear();
            }
        }

        
        // Mostrar información adicional en tooltip
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Name: %s", entry.name.c_str());
            if (!entry.isDirectory) {
                ImGui::Text("Size: %s", FormatFileSize(entry.size).c_str());
            }
            ImGui::Text("Path: %s", GetRelativePathFromContent(entry.path).c_str());
            
            // Si es una imagen, mostrar preview
            if (!entry.isDirectory && IsImageFile(entry.name)) {
                ImGui::Separator();
                ImGui::Text("Image Preview:");
                RenderImagePreview(entry.path);
            }
            
            ImGui::EndTooltip();
        }
        
        if (isSelected) {
            ImGui::PopStyleColor(2);
        }
        
        ImGui::PopID();
    }
    
    ImGui::EndChild();
}

void ContentBrowser::OnRenderGUI() {
    // Inicializar de forma segura
    InitializeAssetsBrowser();

    if (!s_currentPath || !s_currentEntries || !s_selectedFile || !s_contentRootPath || !s_textureCache || !s_textureSizes) {
        ImGui::Begin("Content Browser");
        ImGui::Text("Error: Failed to initialize browser");
        ImGui::End();
        return;
    }

    // Configurar estilo moderno
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
    
    ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_MenuBar);

    // Inicializar la ruta si es necesario
    if (s_currentPath->empty()) {
        try {
            std::string projectPath = FileSystem::getProjectPath();
            // Remover barra final si existe
            if (!projectPath.empty() && (projectPath.back() == '\\' || projectPath.back() == '/')) {
                projectPath.pop_back();
            }
            *s_contentRootPath = projectPath + "\\Content";
            *s_currentPath = *s_contentRootPath;
            s_needsRefresh = true;
        }
        catch (...) {
            ImGui::Text("Error: Cannot access project path");
            ImGui::End();
            ImGui::PopStyleVar(2);
            return;
        }
    }

    // Barra de menú mejorada
    if (ImGui::BeginMenuBar()) {
        // Botón Refresh
        if (ImGui::Button("Refresh")) {
            s_needsRefresh = true;
        }

        ImGui::SameLine();
        
        // Botón Up con validación de límites
        bool canGoUp = false;
        try {
            fs::path currentPathObj(*s_currentPath);
            fs::path contentPathObj(*s_contentRootPath);
            
            // Solo permitir ir hacia arriba si no estamos en Content y estamos dentro del directorio Content
            canGoUp = (currentPathObj != contentPathObj) && IsPathWithinContent(*s_currentPath);
            
            if (canGoUp) {
                if (ImGui::Button("Up")) {
                    fs::path parentPath = currentPathObj.parent_path();
                    // Verificar que el parent path esté dentro de Content
                    if (IsPathWithinContent(parentPath.string()) || parentPath == contentPathObj) {
                        *s_currentPath = parentPath.string();
                        s_needsRefresh = true;
                        s_selectedFile->clear();
                    }
                }
            } else {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                ImGui::Button("Up");
                ImGui::PopStyleVar();
            }
        }
        catch (...) {
            // Ignorar errores de navegación
        }

        ImGui::SameLine();
        
        // Botón para ir a Content
        if (ImGui::Button("Content")) {
            *s_currentPath = *s_contentRootPath;
            s_needsRefresh = true;
            s_selectedFile->clear();
        }

        ImGui::EndMenuBar();
    }

    // Breadcrumbs
    RenderBreadcrumbs();
    
    ImGui::Separator();

    // Refrescar si es necesario
    if (s_needsRefresh) {
        RefreshDirectory();
    }

    // Renderizar TreeView
    RenderTreeView();

    // Panel de información del archivo seleccionado mejorado
    if (s_selectedFile && !s_selectedFile->empty()) {
        ImGui::Separator();
        ImGui::Spacing();
        
        try {
            fs::path selectedPath(*s_selectedFile);
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Selected: %s", selectedPath.filename().string().c_str());

            if (fs::exists(*s_selectedFile)) {
                std::string relativePath = GetRelativePathFromContent(*s_selectedFile);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Path: %s", relativePath.c_str());

                // Si es una imagen, mostrar preview más grande
                if (IsImageFile(selectedPath.filename().string())) {
                    ImGui::Separator();
                    ImGui::Text("Image Preview:");
                    RenderImagePreview(*s_selectedFile);
                }

                // Botones de acción
                ImGui::Spacing();
                if (ImGui::Button("Copy Path")) {
                    ImGui::SetClipboardText(s_selectedFile->c_str());
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Show in Explorer")) {
                    // Aquí podrías implementar la funcionalidad para abrir el explorador
                    // Por ahora solo copiamos la ruta
                    ImGui::SetClipboardText(s_selectedFile->c_str());
                }
            }
        }
        catch (...) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error accessing selected file");
        }
    }

    // Mostrar estadísticas del directorio actual
    ImGui::Separator();
    if (s_currentEntries) {
        int folderCount = 0, fileCount = 0;
        uintmax_t totalSize = 0;
        
        for (const auto& entry : *s_currentEntries) {
            if (entry.isDirectory) folderCount++;
            else {
                fileCount++;
                totalSize += entry.size;
            }
        }
        
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), 
            "Folders: %d | Files: %d | Total Size: %s", 
            folderCount, fileCount, FormatFileSize(totalSize).c_str());
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}