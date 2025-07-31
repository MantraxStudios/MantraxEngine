// FileExplorer.h
#pragma once

#include <string>
#include <filesystem>
#include <imgui/imgui.h>

class FileExplorer {
public:
    static bool ShowPopup(const std::string& root, std::string& outSelected, const std::string& extension);
    static bool ShowSavePopup(const std::string& root, std::string& outSelected, const std::string& extension);
};
