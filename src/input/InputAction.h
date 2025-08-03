#pragma once
#include <string>
#include <functional>
#include <vector>
#include <SDL.h>
#include <glm/glm.hpp>
#include "../core/CoreExporter.h"


enum class MANTRAXCORE_API InputType {
    Button,     // Para acciones de presionar/soltar (como saltar, disparar)
    Value,      // Para valores continuos (como movimiento)
    Vector2D,   // Para movimiento 2D (como WASD)
    MouseButton,// Para botones del mouse
    MouseAxis,  // Para movimiento del mouse y rueda
};

enum class MANTRAXCORE_API MouseAxisType {
    X,          // Movimiento horizontal del mouse
    Y,          // Movimiento vertical del mouse
    ScrollWheel // Rueda del mouse
};

struct MANTRAXCORE_API InputBinding {
    union {
        SDL_Keycode key;
        Uint8 mouseButton;
        MouseAxisType mouseAxis;
    };
    bool isKeyboard;
    bool isPositive;  // Para Value/Vector2D: true = valor positivo, false = valor negativo
    int axis;         // Para Vector2D: 0 = X, 1 = Y

    // Constructor para teclas del teclado
    static InputBinding ForKeyboard(SDL_Keycode k, bool positive = true, int ax = 0) {
        InputBinding binding;
        binding.key = k;
        binding.isKeyboard = true;
        binding.isPositive = positive;
        binding.axis = ax;
        return binding;
    }

    // Constructor para botones del mouse
    static InputBinding ForMouseButton(Uint8 button) {
        InputBinding binding;
        binding.mouseButton = button;
        binding.isKeyboard = false;
        binding.isPositive = true;
        binding.axis = 0;
        return binding;
    }

    // Constructor para ejes del mouse
    static InputBinding ForMouseAxis(MouseAxisType axisType) {
        InputBinding binding;
        binding.mouseAxis = axisType;
        binding.isKeyboard = false;
        binding.isPositive = true;
        binding.axis = 0;
        return binding;
    }
};

class MANTRAXCORE_API InputAction {
public:
    InputAction(const std::string& name, InputType type)
        : name(name), type(type), value(0.0f), vector(0.0f), mouseDelta(0.0f) {
    }

    void addKeyBinding(SDL_Keycode key, bool isPositive = true, int axis = 0) {
        bindings.push_back(InputBinding::ForKeyboard(key, isPositive, axis));
    }

    void addMouseButtonBinding(Uint8 button) {
        bindings.push_back(InputBinding::ForMouseButton(button));
    }

    void addMouseAxisBinding(MouseAxisType axisType) {
        bindings.push_back(InputBinding::ForMouseAxis(axisType));
    }

    // Modificar bindings existentes
    void updateKeyBinding(int index, SDL_Keycode key, bool isPositive = true, int axis = 0) {
        if (index >= 0 && index < static_cast<int>(bindings.size())) {
            bindings[index] = InputBinding::ForKeyboard(key, isPositive, axis);
        }
    }

    void updateMouseButtonBinding(int index, Uint8 button) {
        if (index >= 0 && index < static_cast<int>(bindings.size())) {
            bindings[index] = InputBinding::ForMouseButton(button);
        }
    }

    void updateMouseAxisBinding(int index, MouseAxisType axisType) {
        if (index >= 0 && index < static_cast<int>(bindings.size())) {
            bindings[index] = InputBinding::ForMouseAxis(axisType);
        }
    }

    // Eliminar bindings
    void removeBinding(int index) {
        if (index >= 0 && index < static_cast<int>(bindings.size())) {
            bindings.erase(bindings.begin() + index);
        }
    }

    // Limpiar todos los bindings
    void clearBindings() {
        bindings.clear();
    }

    // Callbacks
    void bindButtonCallback(std::function<void(bool)> callback) {
        buttonCallback = callback;
    }

    void bindValueCallback(std::function<void(float)> callback) {
        valueCallback = callback;
    }

    void bindVector2DCallback(std::function<void(const glm::vec2&)> callback) {
        vector2DCallback = callback;
    }

    void bindMouseAxisCallback(std::function<void(float)> callback) {
        mouseAxisCallback = callback;
    }

    // Getters
    const std::string& getName() const { return name; }
    InputType getType() const { return type; }
    float getValue() const { return value; }
    const glm::vec2& getVector() const { return vector; }
    float getMouseDelta() const { return mouseDelta; }
    const std::vector<InputBinding>& getBindings() const { return bindings; }

    // Update methods
    void updateButton(bool pressed) {
        if (buttonCallback) buttonCallback(pressed);
    }

    void updateValue(float newValue) {
        value = newValue;
        if (valueCallback) valueCallback(value);
    }

    void updateVector(const glm::vec2& newVector) {
        vector = newVector;
        if (vector2DCallback) vector2DCallback(vector);
    }

    void updateMouseAxis(float delta) {
        mouseDelta = delta;
        if (mouseAxisCallback) mouseAxisCallback(mouseDelta);
    }

private:
    std::string name;
    InputType type;
    float value;
    glm::vec2 vector;
    float mouseDelta;
    std::vector<InputBinding> bindings;

    std::function<void(bool)> buttonCallback;
    std::function<void(float)> valueCallback;
    std::function<void(const glm::vec2&)> vector2DCallback;
    std::function<void(float)> mouseAxisCallback;
};