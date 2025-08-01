#include "FileSaveUtils.h"
#include <iostream>

std::string FileSaveUtils::GetContentPath() {
    return FileSystem::getProjectPath() + "\\Content\\";
}

bool FileSaveUtils::SaveFile(std::string& saveAsPath, const std::string& extension, const std::string& windowTitle) {
    std::cout << "FileSaveUtils: Opening save dialog for " << extension << " file" << std::endl;
    bool result = FileExplorer::ShowSavePopup(GetContentPath(), saveAsPath, extension, windowTitle);
    
    if (result && !saveAsPath.empty()) {
        std::string fullPath = GetContentPath() + saveAsPath;
        std::cout << "FileSaveUtils: File will be saved to: " << fullPath << std::endl;
    }
    
    return result;
}

std::string FileSaveUtils::GetFullPath(const std::string& relativePath) {
    return GetContentPath() + relativePath;
}

bool FileSaveUtils::ValidateSave(const std::string& fullPath) {
    return FileSystem::fileExists(fullPath);
} 