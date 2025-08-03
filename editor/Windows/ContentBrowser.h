#pragma once
#include "WindowBehaviour.h"
#include <string>
#include <vector>

class ContentBrowser : public WindowBehaviour {
public:
    void OnRenderGUI() override;

    void ShowLuaEditorPopup();

    const std::string& getName() const override {
        static const std::string name = "ContentBrowser";
        return name;
    }
}; 