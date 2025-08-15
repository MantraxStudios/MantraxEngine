#include "InputConfigLoader.h"
#include <iostream>
#include <fstream>

SDL_Keycode InputConfigLoader::stringToKeycode(const std::string& keyString) {
    // Teclas alfanuméricas
    if (keyString == "SDLK_a") return SDLK_A;
    if (keyString == "SDLK_b") return SDLK_B;
    if (keyString == "SDLK_c") return SDLK_C;
    if (keyString == "SDLK_d") return SDLK_D;
    if (keyString == "SDLK_e") return SDLK_E;
    if (keyString == "SDLK_f") return SDLK_F;
    if (keyString == "SDLK_g") return SDLK_G;
    if (keyString == "SDLK_h") return SDLK_H;
    if (keyString == "SDLK_i") return SDLK_I;
    if (keyString == "SDLK_j") return SDLK_J;
    if (keyString == "SDLK_k") return SDLK_K;
    if (keyString == "SDLK_l") return SDLK_L;
    if (keyString == "SDLK_m") return SDLK_M;
    if (keyString == "SDLK_n") return SDLK_N;
    if (keyString == "SDLK_o") return SDLK_O;
    if (keyString == "SDLK_p") return SDLK_P;
    if (keyString == "SDLK_q") return SDLK_Q;
    if (keyString == "SDLK_r") return SDLK_R;
    if (keyString == "SDLK_s") return SDLK_S;
    if (keyString == "SDLK_t") return SDLK_T;
    if (keyString == "SDLK_u") return SDLK_U;
    if (keyString == "SDLK_v") return SDLK_V;
    if (keyString == "SDLK_w") return SDLK_W;
    if (keyString == "SDLK_x") return SDLK_X;
    if (keyString == "SDLK_y") return SDLK_Y;
    if (keyString == "SDLK_z") return SDLK_Z;
    
    // Números
    if (keyString == "SDLK_0") return SDLK_0;
    if (keyString == "SDLK_1") return SDLK_1;
    if (keyString == "SDLK_2") return SDLK_2;
    if (keyString == "SDLK_3") return SDLK_3;
    if (keyString == "SDLK_4") return SDLK_4;
    if (keyString == "SDLK_5") return SDLK_5;
    if (keyString == "SDLK_6") return SDLK_6;
    if (keyString == "SDLK_7") return SDLK_7;
    if (keyString == "SDLK_8") return SDLK_8;
    if (keyString == "SDLK_9") return SDLK_9;
    
    // Teclas de función
    if (keyString == "SDLK_F1") return SDLK_F1;
    if (keyString == "SDLK_F2") return SDLK_F2;
    if (keyString == "SDLK_F3") return SDLK_F3;
    if (keyString == "SDLK_F4") return SDLK_F4;
    if (keyString == "SDLK_F5") return SDLK_F5;
    if (keyString == "SDLK_F6") return SDLK_F6;
    if (keyString == "SDLK_F7") return SDLK_F7;
    if (keyString == "SDLK_F8") return SDLK_F8;
    if (keyString == "SDLK_F9") return SDLK_F9;
    if (keyString == "SDLK_F10") return SDLK_F10;
    if (keyString == "SDLK_F11") return SDLK_F11;
    if (keyString == "SDLK_F12") return SDLK_F12;
    
    // Teclas especiales
    if (keyString == "SDLK_ESCAPE") return SDLK_ESCAPE;
    if (keyString == "SDLK_TAB") return SDLK_TAB;
    if (keyString == "SDLK_CAPSLOCK") return SDLK_CAPSLOCK;
    if (keyString == "SDLK_LSHIFT") return SDLK_LSHIFT;
    if (keyString == "SDLK_RSHIFT") return SDLK_RSHIFT;
    if (keyString == "SDLK_LCTRL") return SDLK_LCTRL;
    if (keyString == "SDLK_RCTRL") return SDLK_RCTRL;
    if (keyString == "SDLK_LALT") return SDLK_LALT;
    if (keyString == "SDLK_RALT") return SDLK_RALT;
    if (keyString == "SDLK_LGUI") return SDLK_LGUI;
    if (keyString == "SDLK_RGUI") return SDLK_RGUI;

    if (keyString == "SDLK_RETURN") return SDLK_RETURN;
    if (keyString == "SDLK_BACKSPACE") return SDLK_BACKSPACE;
    if (keyString == "SDLK_DELETE") return SDLK_DELETE;
    if (keyString == "SDLK_INSERT") return SDLK_INSERT;
    if (keyString == "SDLK_HOME") return SDLK_HOME;
    if (keyString == "SDLK_END") return SDLK_END;
    if (keyString == "SDLK_PAGEUP") return SDLK_PAGEUP;
    if (keyString == "SDLK_PAGEDOWN") return SDLK_PAGEDOWN;
    if (keyString == "SDLK_PRINTSCREEN") return SDLK_PRINTSCREEN;
    if (keyString == "SDLK_SCROLLLOCK") return SDLK_SCROLLLOCK;
    if (keyString == "SDLK_PAUSE") return SDLK_PAUSE;
    
    // Teclas de dirección
    if (keyString == "SDLK_UP") return SDLK_UP;
    if (keyString == "SDLK_DOWN") return SDLK_DOWN;
    if (keyString == "SDLK_LEFT") return SDLK_LEFT;
    if (keyString == "SDLK_RIGHT") return SDLK_RIGHT;
    
    // Teclas de puntuación
    if (keyString == "SDLK_SPACE") return SDLK_SPACE;
    if (keyString == "SDLK_PERIOD") return SDLK_PERIOD;
    if (keyString == "SDLK_COMMA") return SDLK_COMMA;
    if (keyString == "SDLK_SEMICOLON") return SDLK_SEMICOLON;
    if (keyString == "SDLK_COLON") return SDLK_COLON;
    if (keyString == "SDLK_QUOTE") return SDLK_APOSTROPHE;
    if (keyString == "SDLK_QUOTEDBL") return SDLK_DBLAPOSTROPHE;
    if (keyString == "SDLK_MINUS") return SDLK_MINUS;
    if (keyString == "SDLK_PLUS") return SDLK_PLUS;
    if (keyString == "SDLK_EQUALS") return SDLK_EQUALS;
    if (keyString == "SDLK_SLASH") return SDLK_SLASH;
    if (keyString == "SDLK_BACKSLASH") return SDLK_BACKSLASH;
    if (keyString == "SDLK_BACKQUOTE") return SDLK_GRAVE;
    if (keyString == "SDLK_LEFTBRACKET") return SDLK_LEFTBRACKET;
    if (keyString == "SDLK_RIGHTBRACKET") return SDLK_RIGHTBRACKET;
    if (keyString == "SDLK_LEFTPAREN") return SDLK_LEFTPAREN;
    if (keyString == "SDLK_RIGHTPAREN") return SDLK_RIGHTPAREN;
    if (keyString == "SDLK_EXCLAIM") return SDLK_EXCLAIM;
    if (keyString == "SDLK_QUESTION") return SDLK_QUESTION;
    if (keyString == "SDLK_AT") return SDLK_AT;
    if (keyString == "SDLK_HASH") return SDLK_HASH;
    if (keyString == "SDLK_DOLLAR") return SDLK_DOLLAR;
    if (keyString == "SDLK_PERCENT") return SDLK_PERCENT;
    if (keyString == "SDLK_CARET") return SDLK_CARET;
    if (keyString == "SDLK_AMPERSAND") return SDLK_AMPERSAND;
    if (keyString == "SDLK_ASTERISK") return SDLK_ASTERISK;
    if (keyString == "SDLK_UNDERSCORE") return SDLK_UNDERSCORE;
    if (keyString == "SDLK_LESS") return SDLK_LESS;
    if (keyString == "SDLK_GREATER") return SDLK_GREATER;
    
    // Teclas numéricas del teclado numérico
    if (keyString == "SDLK_KP_0") return SDLK_KP_0;
    if (keyString == "SDLK_KP_1") return SDLK_KP_1;
    if (keyString == "SDLK_KP_2") return SDLK_KP_2;
    if (keyString == "SDLK_KP_3") return SDLK_KP_3;
    if (keyString == "SDLK_KP_4") return SDLK_KP_4;
    if (keyString == "SDLK_KP_5") return SDLK_KP_5;
    if (keyString == "SDLK_KP_6") return SDLK_KP_6;
    if (keyString == "SDLK_KP_7") return SDLK_KP_7;
    if (keyString == "SDLK_KP_8") return SDLK_KP_8;
    if (keyString == "SDLK_KP_9") return SDLK_KP_9;
    if (keyString == "SDLK_KP_PLUS") return SDLK_KP_PLUS;
    if (keyString == "SDLK_KP_MINUS") return SDLK_KP_MINUS;
    if (keyString == "SDLK_KP_MULTIPLY") return SDLK_KP_MULTIPLY;
    if (keyString == "SDLK_KP_DIVIDE") return SDLK_KP_DIVIDE;
    if (keyString == "SDLK_KP_ENTER") return SDLK_KP_ENTER;
    if (keyString == "SDLK_KP_PERIOD") return SDLK_KP_PERIOD;
    if (keyString == "SDLK_KP_EQUALS") return SDLK_KP_EQUALS;
    if (keyString == "SDLK_KP_COMMA") return SDLK_KP_COMMA;
    if (keyString == "SDLK_KP_EQUALSAS400") return SDLK_KP_EQUALSAS400;
    
    // Teclas multimedia
    if (keyString == "SDLK_AUDIOPLAY") return SDLK_MEDIA_PLAY;
    if (keyString == "SDLK_AUDIOSTOP") return SDLK_MEDIA_STOP;
    if (keyString == "SDLK_AUDIOPREV") return SDLK_MEDIA_PREVIOUS_TRACK;
    if (keyString == "SDLK_AUDIONEXT") return SDLK_MEDIA_NEXT_TRACK;
    if (keyString == "SDLK_AUDIOMUTE") return SDLK_MUTE;
    if (keyString == "SDLK_VOLUMEUP") return SDLK_VOLUMEUP;
    if (keyString == "SDLK_VOLUMEDOWN") return SDLK_VOLUMEDOWN;
    if (keyString == "SDLK_MUTE") return SDLK_MUTE;
    if (keyString == "SDLK_MEDIASELECT") return SDLK_MEDIA_SELECT;
    
    // Teclas de navegación web (AC = Application Control)
    if (keyString == "SDLK_AC_BACK") return SDLK_AC_BACK;
    if (keyString == "SDLK_AC_FORWARD") return SDLK_AC_FORWARD;
    if (keyString == "SDLK_AC_REFRESH") return SDLK_AC_REFRESH;
    if (keyString == "SDLK_AC_STOP") return SDLK_AC_STOP;
    if (keyString == "SDLK_AC_SEARCH") return SDLK_AC_SEARCH;
    if (keyString == "SDLK_AC_BOOKMARKS") return SDLK_AC_BOOKMARKS;
    if (keyString == "SDLK_AC_HOME") return SDLK_AC_HOME;
    
    // Teclas de aplicación
    if (keyString == "SDLK_APPLICATION") return SDLK_APPLICATION;
    if (keyString == "SDLK_POWER") return SDLK_POWER;
    if (keyString == "SDLK_EXECUTE") return SDLK_EXECUTE;
    if (keyString == "SDLK_HELP") return SDLK_HELP;
    if (keyString == "SDLK_MENU") return SDLK_MENU;
    if (keyString == "SDLK_SELECT") return SDLK_SELECT;
    if (keyString == "SDLK_STOP") return SDLK_STOP;
    if (keyString == "SDLK_AGAIN") return SDLK_AGAIN;
    if (keyString == "SDLK_UNDO") return SDLK_UNDO;
    if (keyString == "SDLK_CUT") return SDLK_CUT;
    if (keyString == "SDLK_COPY") return SDLK_COPY;
    if (keyString == "SDLK_PASTE") return SDLK_PASTE;
    if (keyString == "SDLK_FIND") return SDLK_FIND;
    
    // Teclas de función extendidas
    if (keyString == "SDLK_F13") return SDLK_F13;
    if (keyString == "SDLK_F14") return SDLK_F14;
    if (keyString == "SDLK_F15") return SDLK_F15;
    if (keyString == "SDLK_F16") return SDLK_F16;
    if (keyString == "SDLK_F17") return SDLK_F17;
    if (keyString == "SDLK_F18") return SDLK_F18;
    if (keyString == "SDLK_F19") return SDLK_F19;
    if (keyString == "SDLK_F20") return SDLK_F20;
    if (keyString == "SDLK_F21") return SDLK_F21;
    if (keyString == "SDLK_F22") return SDLK_F22;
    if (keyString == "SDLK_F23") return SDLK_F23;
    if (keyString == "SDLK_F24") return SDLK_F24;
    
    // Teclas adicionales del sistema
    if (keyString == "SDLK_MODE") return SDLK_MODE;
    if (keyString == "SDLK_EJECT") return SDLK_MEDIA_EJECT;
    if (keyString == "SDLK_SLEEP") return SDLK_SLEEP;
    
    // Teclas de estado
    if (keyString == "SDLK_NUMLOCKCLEAR") return SDLK_NUMLOCKCLEAR;
    if (keyString == "SDLK_CLEAR") return SDLK_CLEAR;
    if (keyString == "SDLK_KP_CLEAR") return SDLK_KP_CLEAR;
    if (keyString == "SDLK_KP_CLEARENTRY") return SDLK_KP_CLEARENTRY;
    if (keyString == "SDLK_KP_BINARY") return SDLK_KP_BINARY;
    if (keyString == "SDLK_KP_OCTAL") return SDLK_KP_OCTAL;
    if (keyString == "SDLK_KP_DECIMAL") return SDLK_KP_DECIMAL;
    if (keyString == "SDLK_KP_HEXADECIMAL") return SDLK_KP_HEXADECIMAL;
    
    // Teclas de idioma (no disponibles en esta versión de SDL)
    // SDLK_LANG1 a SDLK_LANG9 no están disponibles
    
    // Teclas de sistema
    if (keyString == "SDLK_SYSREQ") return SDLK_SYSREQ;
    if (keyString == "SDLK_CANCEL") return SDLK_CANCEL;
    if (keyString == "SDLK_PRIOR") return SDLK_PRIOR;
    if (keyString == "SDLK_RETURN2") return SDLK_RETURN2;
    if (keyString == "SDLK_SEPARATOR") return SDLK_SEPARATOR;
    if (keyString == "SDLK_OUT") return SDLK_OUT;
    if (keyString == "SDLK_OPER") return SDLK_OPER;
    if (keyString == "SDLK_CLEARAGAIN") return SDLK_CLEARAGAIN;
    if (keyString == "SDLK_CRSEL") return SDLK_CRSEL;
    if (keyString == "SDLK_EXSEL") return SDLK_EXSEL;
    
    // Teclas de calculadora
    if (keyString == "SDLK_KP_00") return SDLK_KP_00;
    if (keyString == "SDLK_KP_000") return SDLK_KP_000;
    if (keyString == "SDLK_THOUSANDSSEPARATOR") return SDLK_THOUSANDSSEPARATOR;
    if (keyString == "SDLK_DECIMALSEPARATOR") return SDLK_DECIMALSEPARATOR;
    if (keyString == "SDLK_CURRENCYUNIT") return SDLK_CURRENCYUNIT;
    if (keyString == "SDLK_CURRENCYSUBUNIT") return SDLK_CURRENCYSUBUNIT;
    if (keyString == "SDLK_KP_LEFTPAREN") return SDLK_KP_LEFTPAREN;
    if (keyString == "SDLK_KP_RIGHTPAREN") return SDLK_KP_RIGHTPAREN;
    if (keyString == "SDLK_KP_LEFTBRACE") return SDLK_KP_LEFTBRACE;
    if (keyString == "SDLK_KP_RIGHTBRACE") return SDLK_KP_RIGHTBRACE;
    if (keyString == "SDLK_KP_TAB") return SDLK_KP_TAB;
    if (keyString == "SDLK_KP_BACKSPACE") return SDLK_KP_BACKSPACE;
    if (keyString == "SDLK_KP_A") return SDLK_KP_A;
    if (keyString == "SDLK_KP_B") return SDLK_KP_B;
    if (keyString == "SDLK_KP_C") return SDLK_KP_C;
    if (keyString == "SDLK_KP_D") return SDLK_KP_D;
    if (keyString == "SDLK_KP_E") return SDLK_KP_E;
    if (keyString == "SDLK_KP_F") return SDLK_KP_F;
    if (keyString == "SDLK_KP_XOR") return SDLK_KP_XOR;
    if (keyString == "SDLK_KP_POWER") return SDLK_KP_POWER;
    if (keyString == "SDLK_KP_PERCENT") return SDLK_KP_PERCENT;
    if (keyString == "SDLK_KP_LESS") return SDLK_KP_LESS;
    if (keyString == "SDLK_KP_GREATER") return SDLK_KP_GREATER;
    if (keyString == "SDLK_KP_AMPERSAND") return SDLK_KP_AMPERSAND;
    if (keyString == "SDLK_KP_DBLAMPERSAND") return SDLK_KP_DBLAMPERSAND;
    if (keyString == "SDLK_KP_VERTICALBAR") return SDLK_KP_VERTICALBAR;
    if (keyString == "SDLK_KP_DBLVERTICALBAR") return SDLK_KP_DBLVERTICALBAR;
    if (keyString == "SDLK_KP_COLON") return SDLK_KP_COLON;
    if (keyString == "SDLK_KP_HASH") return SDLK_KP_HASH;
    if (keyString == "SDLK_KP_SPACE") return SDLK_KP_SPACE;
    if (keyString == "SDLK_KP_AT") return SDLK_KP_AT;
    if (keyString == "SDLK_KP_EXCLAM") return SDLK_KP_EXCLAM;
    if (keyString == "SDLK_KP_MEMSTORE") return SDLK_KP_MEMSTORE;
    if (keyString == "SDLK_KP_MEMRECALL") return SDLK_KP_MEMRECALL;
    if (keyString == "SDLK_KP_MEMCLEAR") return SDLK_KP_MEMCLEAR;
    if (keyString == "SDLK_KP_MEMADD") return SDLK_KP_MEMADD;
    if (keyString == "SDLK_KP_MEMSUBTRACT") return SDLK_KP_MEMSUBTRACT;
    if (keyString == "SDLK_KP_MEMMULTIPLY") return SDLK_KP_MEMMULTIPLY;
    if (keyString == "SDLK_KP_MEMDIVIDE") return SDLK_KP_MEMDIVIDE;
    if (keyString == "SDLK_KP_PLUSMINUS") return SDLK_KP_PLUSMINUS;
    if (keyString == "SDLK_KP_CLEAR") return SDLK_KP_CLEAR;
    if (keyString == "SDLK_KP_CLEARENTRY") return SDLK_KP_CLEARENTRY;
    if (keyString == "SDLK_KP_BINARY") return SDLK_KP_BINARY;
    if (keyString == "SDLK_KP_OCTAL") return SDLK_KP_OCTAL;
    if (keyString == "SDLK_KP_DECIMAL") return SDLK_KP_DECIMAL;
    if (keyString == "SDLK_KP_HEXADECIMAL") return SDLK_KP_HEXADECIMAL;
    
    std::cerr << "Unknown key: " << keyString << std::endl;
    return SDLK_UNKNOWN;
}

MouseAxisType InputConfigLoader::stringToMouseAxis(const std::string& axisString) {
    if (axisString == "X") return MouseAxisType::X;
    if (axisString == "Y") return MouseAxisType::Y;
    if (axisString == "ScrollWheel") return MouseAxisType::ScrollWheel;
    
    std::cerr << "Unknown mouse axis: " << axisString << std::endl;
    return MouseAxisType::X;
}

Uint8 InputConfigLoader::stringToMouseButton(const std::string& buttonString) {
    if (buttonString == "SDL_BUTTON_LEFT") return SDL_BUTTON_LEFT;
    if (buttonString == "SDL_BUTTON_RIGHT") return SDL_BUTTON_RIGHT;
    if (buttonString == "SDL_BUTTON_MIDDLE") return SDL_BUTTON_MIDDLE;
    
    std::cerr << "Unknown mouse button: " << buttonString << std::endl;
    return SDL_BUTTON_LEFT;
}

InputType InputConfigLoader::stringToInputType(const std::string& typeString) {
    if (typeString == "Vector2D") return InputType::Vector2D;
    if (typeString == "Value") return InputType::Value;
    if (typeString == "MouseAxis") return InputType::MouseAxis;
    if (typeString == "MouseButton") return InputType::MouseButton;
    if (typeString == "Button") return InputType::Button;
    
    std::cerr << "Unknown input type: " << typeString << std::endl;
    return InputType::Button;
}

void InputConfigLoader::loadInputConfigFromJSON(const std::string& configPath) {
    auto& inputSystem = InputSystem::getInstance();
    
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open input config file: " << configPath << std::endl;
        return;
    }
    
    try {
        nlohmann::json config;
        file >> config;
        
        for (const auto& actionConfig : config["input_actions"]) {
            std::string name = actionConfig["name"];
            std::string typeStr = actionConfig["type"];
            
            InputType type = stringToInputType(typeStr);
            auto action = inputSystem.registerAction(name, type);
            
            if (actionConfig.contains("key_bindings")) {
                for (const auto& binding : actionConfig["key_bindings"]) {
                    SDL_Keycode key = stringToKeycode(binding["key"]);
                    bool positive = binding.value("positive", true);
                    int axis = binding.value("axis", 0);
                    
                    action->addKeyBinding(key, positive, axis);
                }
            }
            
            if (actionConfig.contains("mouse_axis")) {
                MouseAxisType axis = stringToMouseAxis(actionConfig["mouse_axis"]);
                action->addMouseAxisBinding(axis);
            }
            
            if (actionConfig.contains("mouse_button")) {
                Uint8 button = stringToMouseButton(actionConfig["mouse_button"]);
                action->addMouseButtonBinding(button);
            }
        }
        
        std::cout << "Input configuration loaded from JSON: " << configPath << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing input config JSON: " << e.what() << std::endl;
    }
}

void InputConfigLoader::saveInputConfigToJSON(const std::string& configPath) {
    auto& inputSystem = InputSystem::getInstance();
    
    nlohmann::json config;
    nlohmann::json actionsArray = nlohmann::json::array();
    
    // Iterate through all registered actions
    for (const auto& [name, action] : inputSystem.getAllActions()) {
        nlohmann::json actionConfig;
        actionConfig["name"] = name;
        
        // Convert InputType to string
        std::string typeStr;
        switch (action->getType()) {
            case InputType::Button: typeStr = "Button"; break;
            case InputType::Value: typeStr = "Value"; break;
            case InputType::Vector2D: typeStr = "Vector2D"; break;
            case InputType::MouseButton: typeStr = "MouseButton"; break;
            case InputType::MouseAxis: typeStr = "MouseAxis"; break;
        }
        actionConfig["type"] = typeStr;
        
        // Save key bindings
        nlohmann::json keyBindingsArray = nlohmann::json::array();
        for (const auto& binding : action->getBindings()) {
            if (binding.isKeyboard) {
                nlohmann::json keyBinding;
                keyBinding["key"] = keycodeToString(binding.key);
                keyBinding["positive"] = binding.isPositive;
                keyBinding["axis"] = binding.axis;
                keyBindingsArray.push_back(keyBinding);
            }
        }
        if (!keyBindingsArray.empty()) {
            actionConfig["key_bindings"] = keyBindingsArray;
        }
        
        // Save mouse bindings
        for (const auto& binding : action->getBindings()) {
            if (!binding.isKeyboard) {
                if (binding.mouseAxis != MouseAxisType::X) { // Assuming X is default
                    actionConfig["mouse_axis"] = mouseAxisToString(binding.mouseAxis);
                } else {
                    actionConfig["mouse_button"] = mouseButtonToString(binding.mouseButton);
                }
            }
        }
        
        actionsArray.push_back(actionConfig);
    }
    
    config["input_actions"] = actionsArray;
    
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << config.dump(2);
        file.close();
        std::cout << "Input configuration saved to JSON: " << configPath << std::endl;
    } else {
        std::cerr << "Failed to save input configuration to: " << configPath << std::endl;
    }
}

std::string InputConfigLoader::keycodeToString(SDL_Keycode keycode) {
    // This is a simplified implementation - you'd want a comprehensive mapping
    switch (keycode) {
        case SDLK_A: return "SDLK_a";
        case SDLK_B: return "SDLK_b";
        case SDLK_C: return "SDLK_c";
        case SDLK_D: return "SDLK_d";
        case SDLK_E: return "SDLK_e";
        case SDLK_F: return "SDLK_f";
        case SDLK_G: return "SDLK_g";
        case SDLK_H: return "SDLK_h";
        case SDLK_I: return "SDLK_i";
        case SDLK_J: return "SDLK_j";
        case SDLK_K: return "SDLK_k";
        case SDLK_L: return "SDLK_l";
        case SDLK_M: return "SDLK_m";
        case SDLK_N: return "SDLK_n";
        case SDLK_O: return "SDLK_o";
        case SDLK_P: return "SDLK_p";
        case SDLK_Q: return "SDLK_q";
        case SDLK_R: return "SDLK_r";
        case SDLK_S: return "SDLK_s";
        case SDLK_T: return "SDLK_t";
        case SDLK_U: return "SDLK_u";
        case SDLK_V: return "SDLK_v";
        case SDLK_W: return "SDLK_w";
        case SDLK_X: return "SDLK_x";
        case SDLK_Y: return "SDLK_y";
        case SDLK_Z: return "SDLK_z";
        case SDLK_0: return "SDLK_0";
        case SDLK_1: return "SDLK_1";
        case SDLK_2: return "SDLK_2";
        case SDLK_3: return "SDLK_3";
        case SDLK_4: return "SDLK_4";
        case SDLK_5: return "SDLK_5";
        case SDLK_6: return "SDLK_6";
        case SDLK_7: return "SDLK_7";
        case SDLK_8: return "SDLK_8";
        case SDLK_9: return "SDLK_9";
        case SDLK_SPACE: return "SDLK_SPACE";
        case SDLK_LSHIFT: return "SDLK_LSHIFT";
        case SDLK_ESCAPE: return "SDLK_ESCAPE";
        case SDLK_TAB: return "SDLK_TAB";
        case SDLK_RETURN: return "SDLK_RETURN";
        case SDLK_BACKSPACE: return "SDLK_BACKSPACE";
        case SDLK_DELETE: return "SDLK_DELETE";
        case SDLK_UP: return "SDLK_UP";
        case SDLK_DOWN: return "SDLK_DOWN";
        case SDLK_LEFT: return "SDLK_LEFT";
        case SDLK_RIGHT: return "SDLK_RIGHT";
        default: return "SDLK_UNKNOWN";
    }
}

std::string InputConfigLoader::mouseAxisToString(MouseAxisType axis) {
    switch (axis) {
        case MouseAxisType::X: return "X";
        case MouseAxisType::Y: return "Y";
        case MouseAxisType::ScrollWheel: return "ScrollWheel";
        default: return "X";
    }
}

std::string InputConfigLoader::mouseButtonToString(Uint8 button) {
    switch (button) {
        case SDL_BUTTON_LEFT: return "SDL_BUTTON_LEFT";
        case SDL_BUTTON_RIGHT: return "SDL_BUTTON_RIGHT";
        case SDL_BUTTON_MIDDLE: return "SDL_BUTTON_MIDDLE";
        default: return "SDL_BUTTON_LEFT";
    }
} 