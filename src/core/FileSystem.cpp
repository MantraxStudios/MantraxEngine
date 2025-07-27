#include "FileSystem.h"
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

bool FileSystem::writeString(const std::string& filePath, const std::string& content) {
    try {
        if (!ensureDirectoryExists(filePath)) {
            return false;
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }

        file << content;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing to file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::writeLines(const std::string& filePath, const std::vector<std::string>& lines) {
    try {
        if (!ensureDirectoryExists(filePath)) {
            return false;
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }

        for (const auto& line : lines) {
            file << line << std::endl;
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error writing lines to file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::appendString(const std::string& filePath, const std::string& content) {
    try {
        if (!ensureDirectoryExists(filePath)) {
            return false;
        }

        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for appending: " << filePath << std::endl;
            return false;
        }

        file << content;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error appending to file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::appendLine(const std::string& filePath, const std::string& line) {
    return appendString(filePath, line + "\n");
}

bool FileSystem::readString(const std::string& filePath, std::string& outContent) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        outContent = buffer.str();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading from file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::readLines(const std::string& filePath, std::vector<std::string>& outLines) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            outLines.push_back(line);
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading lines from file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::fileExists(const std::string& filePath) {
    return fs::exists(filePath);
}

bool FileSystem::createDirectory(const std::string& dirPath) {
    try {
        return fs::create_directories(dirPath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating directory " << dirPath << ": " << e.what() << std::endl;
        return false;
    }
}

bool FileSystem::deleteFile(const std::string& filePath) {
    try {
        return fs::remove(filePath);
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting file " << filePath << ": " << e.what() << std::endl;
        return false;
    }
}

std::string FileSystem::getFileExtension(const std::string& filePath) {
    return fs::path(filePath).extension().string();
}

std::string FileSystem::getFileName(const std::string& filePath) {
    return fs::path(filePath).filename().string();
}

std::string FileSystem::getFileNameWithoutExtension(const std::string& filePath) {
    return fs::path(filePath).stem().string();
}

std::string FileSystem::getDirectoryPath(const std::string& filePath) {
    return fs::path(filePath).parent_path().string();
}

std::string FileSystem::combinePath(const std::string& path1, const std::string& path2) {
    fs::path combined = fs::path(path1) / fs::path(path2);
    return combined.string();
}

std::string FileSystem::normalizePath(const std::string& path) {
    return fs::path(path).lexically_normal().string();
}

std::string FileSystem::getAbsolutePath(const std::string& relativePath) {
    try {
        return fs::absolute(relativePath).string();
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting absolute path for " << relativePath << ": " << e.what() << std::endl;
        return relativePath;
    }
}

std::string FileSystem::getRelativePath(const std::string& absolutePath, const std::string& basePath) {
    try {
        return fs::relative(absolutePath, basePath).string();
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting relative path for " << absolutePath << ": " << e.what() << std::endl;
        return absolutePath;
    }
}

bool FileSystem::ensureDirectoryExists(const std::string& filePath) {
    try {
        fs::path path = fs::path(filePath).parent_path();
        if (!path.empty() && !fs::exists(path)) {
            return fs::create_directories(path);
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error ensuring directory exists for " << filePath << ": " << e.what() << std::endl;
        return false;
    }
} 