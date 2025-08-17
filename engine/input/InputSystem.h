#pragma once
#include "InputAction.h"
#include <unordered_map>
#include <memory>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API InputSystem {
public:
    static InputSystem& getInstance() {
        static InputSystem instance;
        return instance;
    }

    // Registrar una nueva acción de input
    std::shared_ptr<InputAction> registerAction(const std::string& name, InputType type) {
        auto action = std::make_shared<InputAction>(name, type);
        actions[name] = action;
        return action;
    }

    // Obtener una acción por nombre
    std::shared_ptr<InputAction> getAction(const std::string& name) {
        auto it = actions.find(name);
        return (it != actions.end()) ? it->second : nullptr;
    }

    // Obtener todas las acciones registradas
    const std::unordered_map<std::string, std::shared_ptr<InputAction>>& getAllActions() const {
        return actions;
    }

    // Obtener una acción por índice (para iteración)
    std::shared_ptr<InputAction> getActionByIndex(int index) {
        int i = 0;
        for (const auto& [name, action] : actions) {
            if (i == index) return action;
            i++;
        }
        return nullptr;
    }

    // Obtener el nombre de una acción por índice
    std::string getActionNameByIndex(int index) {
        int i = 0;
        for (const auto& [name, action] : actions) {
            if (i == index) return name;
            i++;
        }
        return "";
    }

    // Obtener el número total de acciones
    size_t getActionCount() const {
        return actions.size();
    }

    // Eliminar una acción
    bool removeAction(const std::string& name) {
        auto it = actions.find(name);
        if (it != actions.end()) {
            actions.erase(it);
            return true;
        }
        return false;
    }

    // Limpiar todas las acciones
    void clearActions() {
        actions.clear();
        keyStates.clear();
        mouseButtonStates.clear();
    }

    // Procesar eventos de SDL
    void processInput(const SDL_Event& event) {
        for (const auto& [name, action] : actions) {
            processActionInput(action.get(), event);
        }
    }

    // Actualizar estados continuos (para Value y Vector2D)
    void update(float deltaTime) {
        for (const auto& [name, action] : actions) {
            if (action->getType() == InputType::Value) {
                updateValueAction(action.get());
            }
            else if (action->getType() == InputType::Vector2D) {
                updateVector2DAction(action.get());
            }
        }
    }

private:
    InputSystem() = default;
    std::unordered_map<std::string, std::shared_ptr<InputAction>> actions;
    std::unordered_map<SDL_Keycode, bool> keyStates;
    std::unordered_map<Uint8, bool> mouseButtonStates;

    void processActionInput(InputAction* action, const SDL_Event& event) {
        if (!action) return;

        switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            bool pressed = (event.type == SDL_EVENT_KEY_DOWN);
            keyStates[event.key.key] = pressed;

            if (action->getType() == InputType::Button) {
                for (const auto& binding : action->getBindings()) {
                    if (binding.isKeyboard && binding.key == event.key.key) {
                        action->updateButton(pressed);
                    }
                }
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            bool pressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
            mouseButtonStates[event.button.button] = pressed;

            if (action->getType() == InputType::MouseButton) {
                for (const auto& binding : action->getBindings()) {
                    if (!binding.isKeyboard && binding.mouseButton == event.button.button) {
                        action->updateButton(pressed);
                    }
                }
            }
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            if (action->getType() == InputType::MouseAxis) {
                for (const auto& binding : action->getBindings()) {
                    if (!binding.isKeyboard) {
                        if (binding.mouseAxis == MouseAxisType::X) {
                            action->updateMouseAxis(static_cast<float>(event.motion.xrel));
                        }
                        else if (binding.mouseAxis == MouseAxisType::Y) {
                            action->updateMouseAxis(static_cast<float>(event.motion.yrel));
                        }
                    }
                }
            }
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            if (action->getType() == InputType::MouseAxis) {
                for (const auto& binding : action->getBindings()) {
                    if (!binding.isKeyboard && binding.mouseAxis == MouseAxisType::ScrollWheel) {
                        action->updateMouseAxis(static_cast<float>(event.wheel.y));
                    }
                }
            }
            break;
        }
        }
    }

    void updateValueAction(InputAction* action) {
        float value = 0.0f;
        for (const auto& binding : action->getBindings()) {
            if (binding.isKeyboard && keyStates[binding.key]) {
                value += binding.isPositive ? 1.0f : -1.0f;
            }
        }
        action->updateValue(value);
    }

    void updateVector2DAction(InputAction* action) {
        glm::vec2 vector(0.0f);
        for (const auto& binding : action->getBindings()) {
            if (binding.isKeyboard && keyStates[binding.key]) {
                float value = binding.isPositive ? 1.0f : -1.0f;
                if (binding.axis == 0) {
                    vector.x += value;
                }
                else {
                    vector.y += value;
                }
            }
        }
        action->updateVector(vector);
    }
};