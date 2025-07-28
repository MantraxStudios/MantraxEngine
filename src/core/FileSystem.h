#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "CoreExporter.h"

class MANTRAXCORE_API FileSystem {
public:
    // Escritura de archivos
    static bool writeString(const std::string& filePath, const std::string& content);
    static bool writeLines(const std::string& filePath, const std::vector<std::string>& lines);
    static bool appendString(const std::string& filePath, const std::string& content);
    static bool appendLine(const std::string& filePath, const std::string& line);

    // Lectura de archivos
    static bool readString(const std::string& filePath, std::string& outContent);
    static bool readLines(const std::string& filePath, std::vector<std::string>& outLines);
    
    // Utilidades de archivos
    static bool fileExists(const std::string& filePath);
    static bool createDirectory(const std::string& dirPath);
    static bool deleteFile(const std::string& filePath);
    static std::string getFileExtension(const std::string& filePath);
    static std::string getFileName(const std::string& filePath);
    static std::string getFileNameWithoutExtension(const std::string& filePath);
    static std::string getDirectoryPath(const std::string& filePath);
    
    // Utilidades de paths
    static std::string combinePath(const std::string& path1, const std::string& path2);
    static std::string normalizePath(const std::string& path);
    static std::string getAbsolutePath(const std::string& relativePath);
    static std::string getRelativePath(const std::string& absolutePath, const std::string& basePath);
    static std::string getProjectPath();

private:
    static bool ensureDirectoryExists(const std::string& filePath);
}; 