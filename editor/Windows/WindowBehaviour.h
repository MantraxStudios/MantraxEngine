#pragma once
#include <string>

class WindowBehaviour {
public:
    WindowBehaviour() : isOpen(true) {}
    virtual ~WindowBehaviour() = default;
    virtual void OnRenderGUI() = 0;
    virtual const std::string& getName() const = 0;
    bool isOpen;
};