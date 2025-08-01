// FileExplorer.h
#pragma once

#include <string>
#include <filesystem>
#include <imgui/imgui.h>

struct SavePopupState {
    std::string currentDir;
    bool initialized = false;
    char fileNameBuffer[256] = "";
    char newFolderBuffer[128] = "";
    std::string folderErrorMsg;
};


class FileExplorer {
public:
    static bool ShowPopup(const std::string& root, std::string& outSelected, const std::string& extension);
    static bool ShowSavePopup(const std::string& root, std::string& outSelected, const std::string& extension, const std::string& defaultWindowName = "Save File");
    static bool ShowSavePopupAnimator(const std::string& root, std::string& outSelected, const std::string& extension, const std::string& defaultWindowName = "Save File Animator");
};
