#pragma once

#include <string>
#include <core/FileSystem.h>
#include "FileExplorer.h"

class FileSaveUtils {
public:
    // Función genérica para guardar cualquier tipo de archivo
    // Uso: FileSaveUtils::SaveFile(saveAsPath, ".Animator", "Save Animator File");
    static bool SaveFile(std::string& saveAsPath, const std::string& extension, const std::string& windowTitle = "Save File");
    
    // Función para obtener la ruta completa después del guardado
    static std::string GetFullPath(const std::string& relativePath);
    
    // Función para validar que el guardado fue exitoso
    static bool ValidateSave(const std::string& fullPath);
    
private:
    static std::string GetContentPath();
}; 