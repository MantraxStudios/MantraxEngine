#pragma once
#include <vector>
#include <memory>
#include "WindowBehaviour.h"

class RenderWindows
{
public:
    RenderWindows();
    void RenderUI();

private:
    std::vector<std::unique_ptr<WindowBehaviour>> windows;
    
};
