#pragma once

#include <string>
#include <SDL.h>
#include <nlohmann/json.hpp>
#include "../input/InputSystem.h"

class MANTRAXCORE_API InputConfigLoader {
public:
    static SDL_Keycode stringToKeycode(const std::string& keyString);
    static MouseAxisType stringToMouseAxis(const std::string& axisString);
    static Uint8 stringToMouseButton(const std::string& buttonString);
    static InputType stringToInputType(const std::string& typeString);
    static void loadInputConfigFromJSON(const std::string& configPath = "config/input_config.json");
}; 