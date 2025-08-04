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
#include <fstream>
#include "../SceneSaver.h"
#include "../EUI/EditorInfo.h"
#include "../EUI/UIBuilder.h"

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

// Cache para iconos de tipos de archivo
static std::map<std::string, Texture*>* s_iconCache = nullptr;
static bool s_iconsLoaded = false;

// Variables para el menú contextual
static bool s_showContextMenu = false;
static std::string s_contextMenuPath = "";
static std::string s_contextMenuName = "";
static bool s_isContextMenuDirectory = false;
static ImVec2 s_contextMenuPos;

// Variables para renombrar archivos
static bool s_showRenamePopup = false;
static char s_renameBuffer[256] = "";
static std::string s_renameOriginalPath = "";
static std::string s_renameOriginalName = "";

// Variables para crear scripts Lua
static bool s_showCreateScriptPopup = false;
static char s_scriptNameBuffer[256] = "";

// Variables para editar scripts Lua
static bool s_showLuaEditorPopup = false;
static std::string s_luaEditorPath = "";
static std::string s_luaEditorName = "";
static char s_luaEditorBuffer[16384] = ""; // Increased buffer size

// Función para inicializar de forma segura
void InitializeAssetsBrowser() {
    if (!s_initialized) {
        s_currentPath = new std::string();
        s_currentEntries = new std::vector<FileEntry>();
        s_selectedFile = new std::string();
        s_contentRootPath = new std::string();
        s_textureCache = new std::map<std::string, Texture*>();
        s_textureSizes = new std::map<std::string, ImVec2>();
        s_iconCache = new std::map<std::string, Texture*>();
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
        
        // Limpiar cache de iconos
        if (s_iconCache) {
            for (auto& pair : *s_iconCache) {
                delete pair.second;
            }
            delete s_iconCache;
        }
        
        s_currentPath = nullptr;
        s_currentEntries = nullptr;
        s_selectedFile = nullptr;
        s_contentRootPath = nullptr;
        s_textureCache = nullptr;
        s_textureSizes = nullptr;
        s_iconCache = nullptr;
        s_initialized = false;
        s_iconsLoaded = false;
    }
}

// Función para cargar iconos de tipos de archivo
void LoadFileTypeIcons() {
    if (s_iconsLoaded || !s_iconCache) {
        return;
    }
    
    std::string iconsPath = "engine/icons/";
    
    // Mapeo de extensiones a iconos
    std::map<std::string, std::string> extensionToIcon = {
        {".png", "image.png"},
        {".jpg", "image.png"},
        {".jpeg", "image.png"},
        {".bmp", "image.png"},
        {".tga", "image.png"},
        {".tiff", "image.png"},
        {".gif", "image.png"},
        {".webp", "image.png"},
        {".wav", "audiosource.png"},
        {".mp3", "audiosource.png"},
        {".ogg", "audiosource.png"},
        {".obj", "model.png"},
        {".fbx", "model.png"},
        {".dae", "model.png"},
        {".txt", "text.png"},
        {".md", "text.png"},
        {".cpp", "cpp.png"},
        {".h", "cpp.png"},
        {".c", "cpp.png"},
        {".shader", "cpp.png"},
        {".glsl", "cpp.png"},
        {".animator", "gameobject.png"},
        {".lua", "lua.png"},
        {".json", "json.png"},
        {".scene", "gameobject.png"},
        {".prefab", "prefab.png"},
        {".mp4", "video.png"},
        {".avi", "video.png"},
        {".mov", "video.png"},
        {".mkv", "video.png"}
    };
    
    // Función auxiliar para cargar icono con path correcto
    auto LoadIconTexture = [](const std::string& iconName) -> Texture* {
        Texture* iconTexture = new Texture();
        bool loadResult = iconTexture->loadIconFromFile(iconName);
        if (loadResult) {
            return iconTexture;
        } else {
            delete iconTexture;
            return nullptr;
        }
    };
    
    // Cargar cada icono
    for (const auto& pair : extensionToIcon) {
        std::string iconPath = iconsPath + pair.second;
        Texture* iconTexture = LoadIconTexture(iconPath);
        
        if (iconTexture) {
            (*s_iconCache)[pair.first] = iconTexture;
        } else {
            // Si falla, usar icono por defecto
            Texture* defaultIcon = LoadIconTexture(iconsPath + "unknown.png");
            if (defaultIcon) {
                (*s_iconCache)[pair.first] = defaultIcon;
            }
        }
    }
    
    // Cargar icono para directorios
    Texture* folderIcon = LoadIconTexture(iconsPath + "Folder.png");
    if (folderIcon) {
        (*s_iconCache)["[DIR]"] = folderIcon;
    }
    
    // Cargar icono por defecto para archivos desconocidos
    Texture* unknownIcon = LoadIconTexture(iconsPath + "unknown.png");
    if (unknownIcon) {
        (*s_iconCache)["[FILE]"] = unknownIcon;
    }
    
    s_iconsLoaded = true;
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

// Función para crear un script Lua
bool CreateLuaScript(const std::string& scriptPath, const std::string& scriptName) {
    try {
        // Crear el contenido del script
        std::string content = "-- " + scriptName + "\n";
        content += "-- Created by MantraxEngine ContentBrowser\n\n";
        content += "function OnStart()\n";
        content += "    -- Initialize your script here\n";
        content += "    print(\"Script started: " + scriptName + "\")\n";
        content += "end\n\n";
        content += "function OnTick()\n";
        content += "    -- Update your script here\n";
        content += "end\n\n";
        content += "function OnDestroy()\n";
        content += "    -- Cleanup your script here\n";
        content += "end\n";
        
        // Usar FileSystem para escribir el archivo
        bool success = FileSystem::writeString(scriptPath, content);
        if (success) {
            std::cout << "Script created successfully: " << scriptPath << std::endl;
        } else {
            std::cerr << "Failed to write script file: " << scriptPath << std::endl;
        }
        return success;
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating Lua script: " << e.what() << std::endl;
        return false;
    }
}

// Función para renombrar archivo o directorio
bool RenameFile(const std::string& oldPath, const std::string& newName) {
    try {
        // Validar parámetros de entrada
        if (oldPath.empty() || newName.empty()) {
            std::cerr << "Error renaming file: Invalid parameters" << std::endl;
            return false;
        }
        
        fs::path oldPathObj(oldPath);
        fs::path newPathObj = oldPathObj.parent_path() / newName;
        
        // Verificar que el archivo original existe
        if (!fs::exists(oldPathObj)) {
            std::cerr << "Error renaming file: Source file does not exist: " << oldPath << std::endl;
            return false;
        }
        
        // Verificar que el nombre nuevo no esté vacío después de procesar
        if (newName.empty() || newName == "." || newName == "..") {
            std::cerr << "Error renaming file: Invalid new name: " << newName << std::endl;
            return false;
        }
        
        if (fs::exists(newPathObj)) {
            std::cerr << "Error renaming file: Destination file already exists: " << newPathObj.string() << std::endl;
            return false; // Ya existe un archivo con ese nombre
        }
        
        fs::rename(oldPathObj, newPathObj);
        std::cout << "File renamed successfully: " << oldPath << " -> " << newPathObj.string() << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

// Función para eliminar archivo o directorio
bool DeleteFile(const std::string& path) {
    try {
        fs::path pathObj(path);
        
        if (fs::is_directory(pathObj)) {
            fs::remove_all(pathObj);
        } else {
            fs::remove(pathObj);
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting file: " << e.what() << std::endl;
        return false;
    }
}

// Función para mostrar el menú contextual
void ShowContextMenu() {
    if (!s_showContextMenu) return;
    
    // Abrir el popup y marcar que ya se procesó
    ImGui::OpenPopup("ContextMenu");
    s_showContextMenu = false;
    
    // El popup se abrirá en el siguiente frame
}

// Función para renderizar el menú contextual (se llama en cada frame)
void RenderContextMenu() {
    if (ImGui::BeginPopup("ContextMenu", ImGuiWindowFlags_AlwaysAutoResize)) {
        
        // Opciones para directorios
        if (s_isContextMenuDirectory) {
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::MenuItem("Lua Script")) {
                    s_showCreateScriptPopup = true;
                    // Limpiar el buffer para que se inicialice con el nombre por defecto en el popup
                    memset(s_scriptNameBuffer, 0, sizeof(s_scriptNameBuffer));
                    std::cout << "Lua Script menu item clicked. Popup should open." << std::endl;
                }
                
                if (ImGui::MenuItem("Text File")) {
                    // Crear archivo de texto
                    std::string textPath = s_contextMenuPath + "\\NewText.txt";
                    std::ofstream file(textPath);
                    if (file.is_open()) {
                        file << "# New Text File\n";
                        file << "# Created by MantraxEngine ContentBrowser\n";
                        file.close();
                        s_needsRefresh = true;
                    }
                }
                
                if (ImGui::MenuItem("JSON File")) {
                    // Crear archivo JSON
                    std::string jsonPath = s_contextMenuPath + "\\NewConfig.json";
                    std::ofstream file(jsonPath);
                    if (file.is_open()) {
                        file << "{\n";
                        file << "    \"name\": \"NewConfig\",\n";
                        file << "    \"version\": \"1.0\",\n";
                        file << "    \"description\": \"Configuration file\"\n";
                        file << "}\n";
                        file.close();
                        s_needsRefresh = true;
                    }
                }
                
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Open in Explorer")) {
                // Abrir carpeta en el explorador de Windows
                std::string command = "explorer \"" + s_contextMenuPath + "\"";
                system(command.c_str());
            }
            
            if (ImGui::MenuItem("Copy Path")) {
                ImGui::SetClipboardText(s_contextMenuPath.c_str());
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Rename")) {
                s_showRenamePopup = true;
                strncpy_s(s_renameBuffer, sizeof(s_renameBuffer), s_contextMenuName.c_str(), _TRUNCATE);
                s_renameOriginalPath = s_contextMenuPath;
                s_renameOriginalName = s_contextMenuName;
            }
            
            if (ImGui::MenuItem("Delete")) {
                if (DeleteFile(s_contextMenuPath)) {
                    s_needsRefresh = true;
                    if (s_selectedFile) {
                        s_selectedFile->clear();
                    }
                }
            }
        }
        // Opciones para archivos
        else {
            // Detectar tipo de archivo para opciones específicas
            size_t dotPos = s_contextMenuName.find_last_of('.');
            std::string extension;
            if (dotPos != std::string::npos) {
                extension = s_contextMenuName.substr(dotPos);
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            }
            
            // Opciones específicas según el tipo de archivo
            if (extension == ".scene") {
                if (ImGui::MenuItem("Open Scene")) {
                    EditorInfo::currentScenePath = s_contextMenuPath;
                    if (SceneSaver::LoadScene(s_contextMenuPath)) {
                        std::cout << "Escena cargada: " << s_contextMenuName << std::endl;
                    }
                }
                ImGui::Separator();
            }
                         else if (extension == ".lua") {
                 if (ImGui::MenuItem("Edit Script")) {
                     // Abrir editor de scripts Lua
                     s_showLuaEditorPopup = true;
                     s_luaEditorPath = s_contextMenuPath;
                     s_luaEditorName = s_contextMenuName;
                 }
                 
                 if (ImGui::MenuItem("Open in Text Editor")) {
                     std::string command = "notepad \"" + s_contextMenuPath + "\"";
                     system(command.c_str());
                 }
                 ImGui::Separator();
             }
            else if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
                     extension == ".bmp" || extension == ".tga" || extension == ".tiff") {
                if (ImGui::MenuItem("Open in Image Viewer")) {
                    std::string command = "mspaint \"" + s_contextMenuPath + "\"";
                    system(command.c_str());
                }
                ImGui::Separator();
            }
            
            if (ImGui::MenuItem("Open in Explorer")) {
                // Abrir carpeta contenedora en el explorador y seleccionar el archivo
                std::string command = "explorer /select,\"" + s_contextMenuPath + "\"";
                system(command.c_str());
            }
            
            if (ImGui::MenuItem("Copy Path")) {
                ImGui::SetClipboardText(s_contextMenuPath.c_str());
            }
            
            if (ImGui::MenuItem("Copy Name")) {
                ImGui::SetClipboardText(s_contextMenuName.c_str());
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Rename")) {
                s_showRenamePopup = true;
                strncpy_s(s_renameBuffer, sizeof(s_renameBuffer), s_contextMenuName.c_str(), _TRUNCATE);
                s_renameOriginalPath = s_contextMenuPath;
                s_renameOriginalName = s_contextMenuName;
            }
            
            if (ImGui::MenuItem("Delete")) {
                if (DeleteFile(s_contextMenuPath)) {
                    s_needsRefresh = true;
                    if (s_selectedFile) {
                        s_selectedFile->clear();
                    }
                }
            }
        }
        
        ImGui::EndPopup();
    }
}

// Función para mostrar el popup de renombrar
void ShowRenamePopup() {
    if (!s_showRenamePopup) return;
    
    ImGui::OpenPopup("RenamePopup");
    s_showRenamePopup = false;
    
    // Asegurar que el buffer esté inicializado
    if (strlen(s_renameBuffer) == 0 && !s_renameOriginalName.empty()) {
        strncpy_s(s_renameBuffer, sizeof(s_renameBuffer), s_renameOriginalName.c_str(), _TRUNCATE);
    }
    
    if (ImGui::BeginPopupModal("RenamePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rename: %s", s_renameOriginalName.c_str());
        ImGui::Separator();
        
        if (ImGui::InputText("New Name", s_renameBuffer, sizeof(s_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            std::string newName = s_renameBuffer;
            if (!newName.empty() && newName != s_renameOriginalName) {
                if (RenameFile(s_renameOriginalPath, newName)) {
                    s_needsRefresh = true;
                    if (s_selectedFile) {
                        s_selectedFile->clear();
                    }
                }
            }
            ImGui::CloseCurrentPopup();
        }
        
        if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            std::string newName = s_renameBuffer;
            if (!newName.empty() && newName != s_renameOriginalName) {
                if (RenameFile(s_renameOriginalPath, newName)) {
                    s_needsRefresh = true;
                    if (s_selectedFile) {
                        s_selectedFile->clear();
                    }
                }
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

// Función para mostrar el popup de crear script
void ShowCreateScriptPopup() {
    if (s_showCreateScriptPopup) {
        ImGui::OpenPopup("CreateScriptPopup");
        s_showCreateScriptPopup = false;
        std::cout << "Opening CreateScriptPopup" << std::endl;
    }
    
    if (ImGui::BeginPopup("CreateScriptPopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Create Lua Script");
        ImGui::Separator();
        
        ImGui::Text("Enter script name:");
        ImGui::Spacing();
        
        // Inicializar con un nombre por defecto si está vacío
        if (strlen(s_scriptNameBuffer) == 0) {
            strncpy_s(s_scriptNameBuffer, sizeof(s_scriptNameBuffer), "NewScript.lua", _TRUNCATE);
        }
        
        if (ImGui::InputText("Script Name", s_scriptNameBuffer, sizeof(s_scriptNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            // Procesar la creación del script cuando se presiona Enter
            std::string scriptName = s_scriptNameBuffer;
            if (!scriptName.empty()) {
                // Asegurar que tenga extensión .lua
                if (scriptName.find(".lua") == std::string::npos) {
                    scriptName += ".lua";
                }
                
                std::string scriptPath = s_contextMenuPath + "\\" + scriptName;
                if (CreateLuaScript(scriptPath, scriptName)) {
                    s_needsRefresh = true;
                    std::cout << "Script created: " << scriptPath << std::endl;
                } else {
                    std::cerr << "Failed to create script: " << scriptPath << std::endl;
                }
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::Spacing();
        
        if (ImGui::Button("Create") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            // Procesar la creación del script cuando se presiona el botón Create o Enter
            std::string scriptName = s_scriptNameBuffer;
            if (!scriptName.empty()) {
                // Asegurar que tenga extensión .lua
                if (scriptName.find(".lua") == std::string::npos) {
                    scriptName += ".lua";
                }
                
                std::string scriptPath = s_contextMenuPath + "\\" + scriptName;
                if (CreateLuaScript(scriptPath, scriptName)) {
                    s_needsRefresh = true;
                    std::cout << "Script created: " << scriptPath << std::endl;
                } else {
                    std::cerr << "Failed to create script: " << scriptPath << std::endl;
                }
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        
        // Cerrar popup si se hace clic fuera de él
        // Usar una detección más precisa para evitar cerrar cuando se hace clic en elementos del popup
        if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void FileDragInfo(const FileEntry& entry) {
    size_t dotPos = entry.name.find_last_of('.');
    std::string extension;
    if (dotPos != std::string::npos) {
        extension = entry.name.substr(dotPos);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".png" || extension == ".jpg") {
            UIBuilder::Drag("TextureClass", FileSystem::GetPathAfterContent(entry.path));
        }
        else if (extension == ".mp3" || extension == ".wav" || extension == ".ogg") {
            UIBuilder::Drag("AudioClass", FileSystem::GetPathAfterContent(entry.path));
        }
        else if (extension == ".animator") {
            UIBuilder::Drag("AnimatorClass", FileSystem::GetPathAfterContent(entry.path));
        }
    }
    // Si no tiene extensión, simplemente no hace nada
}


// Función para obtener el icono según el tipo de archivo
Texture* GetFileIcon(const FileEntry& entry) {
    if (!s_iconCache) {
        return nullptr;
    }
    
    if (entry.isDirectory) {
        auto it = s_iconCache->find("[DIR]");
        if (it != s_iconCache->end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    size_t dotPos = entry.name.find_last_of('.');
    if (dotPos == std::string::npos) {
        auto it = s_iconCache->find("[FILE]");
        if (it != s_iconCache->end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

    std::string extension = entry.name.substr(dotPos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    auto it = s_iconCache->find(extension);
    if (it != s_iconCache->end()) {
        return it->second;
    }
    
    // Si no se encuentra, usar icono por defecto
    auto defaultIt = s_iconCache->find("[FILE]");
    if (defaultIt != s_iconCache->end()) {
        return defaultIt->second;
    } else {
        return nullptr;
    }
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
                    if (s_selectedFile) {
                        s_selectedFile->clear();
                    }
                }
            }
        }
        
        ImGui::PopStyleVar();
        
    } catch (...) {
        ImGui::Text("Error rendering breadcrumbs");
    }
}

// Función para renderizar preview de imagen
// Función para renderizar preview de imagen - CORREGIDA
void RenderImagePreview(const std::string& pathImage) {
    if (!s_textureCache || !s_textureSizes) return;

    std::string imagePath = FileSystem::GetPathAfterContent(pathImage);

    // Verificar si la textura ya está en cache
    auto it = s_textureCache->find(imagePath);
    Texture* texture = nullptr;
    ImVec2 textureSize;

    if (it == s_textureCache->end()) {
        // Cargar la textura si no está en cache
        texture = LoadImageTexture(imagePath);
        if (texture) {
            (*s_textureCache)[imagePath] = texture;
            textureSize = ImVec2(static_cast<float>(texture->getWidth()),
                static_cast<float>(texture->getHeight()));
            (*s_textureSizes)[imagePath] = textureSize;
        }
    }
    else {
        texture = it->second;
        // CORREGIDO: Usar el tamaño del cache en lugar de recalcularlo
        textureSize = (*s_textureSizes)[imagePath];
    }

    if (texture) {
        // Calcular tamaño del preview (puedes ajustar este valor)
        float maxSize = 256.0f; // CAMBIADO: de 512.0f a 256.0f para mejor visibilidad
        ImVec2 previewSize = textureSize;

        if (previewSize.x > maxSize || previewSize.y > maxSize) {
            float scale = maxSize / std::max(previewSize.x, previewSize.y);
            previewSize.x *= scale;
            previewSize.y *= scale;
        }

        // Asegurar un tamaño mínimo
        float minSize = 128.0f;
        if (previewSize.x < minSize && previewSize.y < minSize) {
            float scale = minSize / std::max(previewSize.x, previewSize.y);
            previewSize.x *= scale;
            previewSize.y *= scale;
        }

        // Convertir el ID de OpenGL a ImTextureID para ImGui
        ImTextureID textureId = (ImTextureID)(intptr_t)texture->getID();

        // Renderizar la imagen con coordenadas UV invertidas en Y para corregir la orientación
        ImGui::Image(textureId, previewSize, ImVec2(0, 0), ImVec2(1, 1)); // normal

        // Mostrar información adicional
        ImGui::Text("Size: %.0fx%.0f", textureSize.x, textureSize.y);
        ImGui::Text("Preview: %.0fx%.0f", previewSize.x, previewSize.y); // DEBUG: mostrar tamaño del preview
        ImGui::Text("File: %s", fs::path(imagePath).filename().string().c_str());
    }
    else {
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
    
    // Manejar clic derecho en el área vacía del TreeView
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1) && !ImGui::IsAnyItemHovered()) {
        s_showContextMenu = true;
        s_contextMenuPath = *s_currentPath; // Ruta del directorio actual
        s_contextMenuName = fs::path(*s_currentPath).filename().string();
        s_isContextMenuDirectory = true;
        s_contextMenuPos = ImGui::GetMousePos();
    }
    
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
        
        // Obtener el icono del archivo
        Texture* fileIcon = GetFileIcon(entry);
        
        // Crear el texto del nodo (solo el nombre, sin icono de texto)
        std::string displayText = entry.name;
        
        // Renderizar el icono si está disponible
        if (fileIcon) {
            ImTextureID iconId = (ImTextureID)(intptr_t)fileIcon->getID();
            ImVec2 iconSize(32, 32); // Tamaño más grande para el icono
            
            // Renderizar el icono
            ImGui::Image(iconId, iconSize);
            ImGui::SameLine();
            
            // Centrar verticalmente el texto con el icono
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (iconSize.y - ImGui::GetTextLineHeight()) * 0.5f);
        } else {
            // Debug: mostrar cuando no hay icono
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[?]");
            ImGui::SameLine();
        }
        
        // Para todos los elementos, usar Selectable para mejor visibilidad
        if (ImGui::Selectable(displayText.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick) && ImGui::IsMouseDoubleClicked(0)) {
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
                    EditorInfo::currentScenePath = entry.path;
                    if (SceneSaver::LoadScene(entry.path)) {
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
                if (s_selectedFile) {
                    s_selectedFile->clear();
                }
            }

            // Doble click para abrir archivos .lua con Visual Studio Code
            if (ImGui::IsMouseDoubleClicked(0) && !entry.isDirectory) {
                std::string extension;
                size_t dotPos = entry.name.find_last_of('.');
                if (dotPos != std::string::npos) {
                    extension = entry.name.substr(dotPos);
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                }

                if (extension == ".lua") {
                    // Abrir con Visual Studio Code
                    std::string command = "code \"" + entry.path + "\"";
                    int result = system(command.c_str());
                    if (result == 0) {
                        std::cout << "Archivo Lua abierto con Visual Studio Code: " << entry.name << std::endl;
                    } else {
                        std::cerr << "Error al abrir archivo con Visual Studio Code: " << entry.path << std::endl;
                        // Fallback: intentar abrir con notepad
                        std::string fallbackCommand = "notepad \"" + entry.path + "\"";
                        system(fallbackCommand.c_str());
                        std::cout << "Archivo abierto con Notepad como fallback" << std::endl;
                    }
                }
            }
        }

        // Manejar clic derecho para menú contextual
        if (ImGui::IsItemClicked(1) || (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))) { // Clic derecho
            s_showContextMenu = true;
            s_contextMenuPath = entry.path;
            s_contextMenuName = entry.name;
            s_isContextMenuDirectory = entry.isDirectory;
            s_contextMenuPos = ImGui::GetMousePos();
        }

        FileDragInfo(entry);

        
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
    InitializeAssetsBrowser();
    
    // Cargar iconos solo una vez después de la inicialización
    static bool iconsLoadedThisFrame = false;
    if (!iconsLoadedThisFrame) {
        LoadFileTypeIcons(); // Cargar iconos al inicio
        iconsLoadedThisFrame = true;
    }

    if (!s_currentPath || !s_currentEntries || !s_selectedFile || !s_contentRootPath || !s_textureCache || !s_textureSizes) {
        ImGui::Begin("Content Browser");
        ImGui::Text("Error: Failed to initialize browser");
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));
    
    ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_MenuBar);

    if (s_currentPath->empty()) {
        try {
            std::string projectPath = FileSystem::getProjectPath();
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
        
        // Test button for popup
        if (ImGui::Button("Test Popup")) {
            ImGui::OpenPopup("TestPopup");
        }
        
        if (ImGui::BeginPopup("TestPopup")) {
            ImGui::Text("Test popup works!");
            if (ImGui::MenuItem("Test Item")) {
                std::cout << "Test item clicked!" << std::endl;
            }
            ImGui::EndPopup();
        }
        
        ImGui::SameLine();
        
        // Test button for script creation popup
        if (ImGui::Button("Test Script Popup")) {
            s_showCreateScriptPopup = true;
            s_contextMenuPath = *s_currentPath;
            memset(s_scriptNameBuffer, 0, sizeof(s_scriptNameBuffer));
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
                        if (s_selectedFile) {
                            s_selectedFile->clear();
                        }
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
            if (s_selectedFile) {
                s_selectedFile->clear();
            }
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
    
    // Mostrar menús contextuales - FUERA del TreeView
    ShowContextMenu();
    RenderContextMenu(); // Renderizar el popup en cada frame
    ShowRenamePopup();
    ShowCreateScriptPopup();
    ShowLuaEditorPopup();

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

// Función para mostrar el popup de editar script Lua
void ContentBrowser::ShowLuaEditorPopup() {
    if (!s_showLuaEditorPopup) return;
    
    ImGui::OpenPopup("LuaEditorPopup");
    s_showLuaEditorPopup = false;
    
    // Configurar tamaño de ventana
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                           ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("LuaEditorPopup", nullptr, ImGuiWindowFlags_NoCollapse)) {
        // Título personalizado
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "📝 Lua Script Editor");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " - %s", s_luaEditorName.c_str());
        ImGui::Separator();
        
        // Toolbar
        if (ImGui::Button("📁 Open External")) {
            // Abrir en editor externo
            std::string command = "notepad \"" + s_luaEditorPath + "\"";
            system(command.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("💾 Save")) {
            try {
                if (FileSystem::writeString(s_luaEditorPath, s_luaEditorBuffer)) {
                    std::cout << "Script saved: " << s_luaEditorName << std::endl;
                } else {
                    std::cerr << "Failed to save script: " << s_luaEditorPath << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error saving script: " << e.what() << std::endl;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("🔄 Reload")) {
            try {
                std::string content;
                if (FileSystem::readString(s_luaEditorPath, content)) {
                    // Limpiar buffer antes de copiar
                    memset(s_luaEditorBuffer, 0, sizeof(s_luaEditorBuffer));
                    // Copiar contenido de forma segura
                    if (content.length() < sizeof(s_luaEditorBuffer)) {
                        strncpy_s(s_luaEditorBuffer, sizeof(s_luaEditorBuffer), content.c_str(), _TRUNCATE);
                        std::cout << "Script reloaded: " << s_luaEditorName << std::endl;
                    } else {
                        std::cerr << "Script too large to load in editor: " << s_luaEditorName << std::endl;
                    }
                } else {
                    std::cerr << "Failed to reload script: " << s_luaEditorPath << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error reloading script: " << e.what() << std::endl;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("📋 Copy Path")) {
            ImGui::SetClipboardText(s_luaEditorPath.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("📂 Show in Explorer")) {
            std::string command = "explorer /select,\"" + s_luaEditorPath + "\"";
            system(command.c_str());
        }
        
        ImGui::Separator();
        
        // Cargar contenido del archivo solo la primera vez
        static std::string lastLoadedFile = "";
        if (lastLoadedFile != s_luaEditorPath) {
            lastLoadedFile = s_luaEditorPath;
            try {
                std::string content;
                if (FileSystem::readString(s_luaEditorPath, content)) {
                    // Limpiar buffer antes de copiar
                    memset(s_luaEditorBuffer, 0, sizeof(s_luaEditorBuffer));
                    // Copiar contenido de forma segura
                    if (content.length() < sizeof(s_luaEditorBuffer)) {
                        strncpy_s(s_luaEditorBuffer, sizeof(s_luaEditorBuffer), content.c_str(), _TRUNCATE);
                    } else {
                        std::cerr << "Script too large to load in editor: " << s_luaEditorName << std::endl;
                        // Cargar solo los primeros caracteres
                        strncpy_s(s_luaEditorBuffer, sizeof(s_luaEditorBuffer), content.substr(0, sizeof(s_luaEditorBuffer) - 1).c_str(), _TRUNCATE);
                    }
                } else {
                    memset(s_luaEditorBuffer, 0, sizeof(s_luaEditorBuffer));
                    std::cerr << "Failed to load script: " << s_luaEditorPath << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error loading script: " << e.what() << std::endl;
                memset(s_luaEditorBuffer, 0, sizeof(s_luaEditorBuffer));
            }
        }
        
        // Área del editor
        ImGui::BeginChild("EditorArea", ImVec2(0, 400), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        // Configurar área de texto
        ImGui::PushItemWidth(-1);
        
        // Contador de líneas y caracteres
        int lineCount = 1;
        int charCount = 0;
        for (int i = 0; s_luaEditorBuffer[i] != '\0'; i++) {
            if (s_luaEditorBuffer[i] == '\n') lineCount++;
            charCount++;
        }
        
        // Editor de texto multilínea con mejor UX
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        
        // Área de texto editable
        if (ImGui::InputTextMultiline("##LuaEditor", s_luaEditorBuffer, sizeof(s_luaEditorBuffer),
                                     ImVec2(-1, 380),
                                     ImGuiInputTextFlags_AllowTabInput |
                                     ImGuiInputTextFlags_NoHorizontalScroll)) {
            // El contenido se actualiza automáticamente
        }
        
        ImGui::PopStyleVar();
        
        // Manejar entrada de texto
        if (ImGui::IsWindowFocused()) {
            // Atajos de teclado
            if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
                try {
                    if (FileSystem::writeString(s_luaEditorPath, s_luaEditorBuffer)) {
                        std::cout << "Script saved: " << s_luaEditorName << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error saving script: " << e.what() << std::endl;
                }
            }
        }
        
        ImGui::PopItemWidth();
        ImGui::EndChild();
        
        // Barra de estado
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
            "Lines: %d | Characters: %d | Path: %s",
            lineCount, charCount, s_luaEditorPath.c_str());
        
        // Botones de acción
        ImGui::Separator();
        if (ImGui::Button("💾 Save", ImVec2(80, 0))) {
            try {
                if (FileSystem::writeString(s_luaEditorPath, s_luaEditorBuffer)) {
                    std::cout << "Script saved: " << s_luaEditorName << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error saving script: " << e.what() << std::endl;
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("❌ Cancel", ImVec2(80, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("▶️ Run Script", ImVec2(100, 0))) {
            // Aquí podrías implementar la ejecución del script
            std::cout << "Running script: " << s_luaEditorName << std::endl;
        }
        
        ImGui::EndPopup();
    }
}