#pragma once
#include "FileSaveUtils.h"
#include <string>

// Ejemplos de uso de FileSaveUtils::SaveFile para diferentes tipos de archivos
class FileSaveExamples {
public:
    // Ejemplo 1: Guardar archivo de animación
    static bool SaveAnimatorExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".Animator", "Save Animator File");
    }
    
    // Ejemplo 2: Guardar archivo de escena
    static bool SaveSceneExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".scene", "Save Scene File");
    }
    
    // Ejemplo 3: Guardar archivo de configuración
    static bool SaveConfigExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".json", "Save Config File");
    }
    
    // Ejemplo 4: Guardar archivo de script
    static bool SaveScriptExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".lua", "Save Script File");
    }
    
    // Ejemplo 5: Guardar archivo de material
    static bool SaveMaterialExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".mat", "Save Material File");
    }
    
    // Ejemplo 6: Guardar archivo de prefab
    static bool SavePrefabExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".prefab", "Save Prefab File");
    }
    
    // Ejemplo 7: Guardar archivo de audio
    static bool SaveAudioExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".ogg", "Save Audio File");
    }
    
    // Ejemplo 8: Guardar archivo de textura
    static bool SaveTextureExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".png", "Save Texture File");
    }
    
    // Ejemplo 9: Guardar archivo de modelo 3D
    static bool SaveModelExample() {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, ".fbx", "Save Model File");
    }
    
    // Ejemplo 10: Uso genérico con cualquier extensión
    static bool SaveCustomFileExample(const std::string& extension, const std::string& title) {
        std::string saveAsPath;
        return FileSaveUtils::SaveFile(saveAsPath, extension, title);
    }
}; 