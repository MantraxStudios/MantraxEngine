#pragma once
#include "../core/CoreExporter.h"
#include <map>
#include <string>
#include <any>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include "../core/UIDGenerator.h"

class GameObject;

class MANTRAXCORE_API Component {
public:
    Component() : owner(nullptr), isDestroyed(false), isEnabled(true) {}
    std::unordered_map<std::string, std::any> variableMap;

    // Para punteros de tipos básicos
    void set_var(const std::string& name, float* ptr) {
        variableMap[name] = std::any(ptr);
    }

    void set_var(const std::string& name, bool* ptr) {
        variableMap[name] = std::any(ptr);
    }

    void set_var(const std::string& name, int* ptr) {
        variableMap[name] = std::any(ptr);
    }

    void set_var(const std::string& name, std::string* ptr) {
        variableMap[name] = std::any(ptr);
    }

    void set_var(const std::string& name, glm::vec2* ptr) {
        variableMap[name] = std::any(ptr);
    }

    void set_var(const std::string& name, glm::vec3* ptr) {
        variableMap[name] = std::any(ptr);
    }

    // Versión template genérica (alternativa)
    template<typename T>
    void set_var_generic(const std::string& name, T* ptr) {
        static_assert(std::is_pointer_v<T*>, "Debe ser un puntero");
        variableMap[name] = std::any(ptr);
    }

    template <typename T>
    T& get_var(const std::string& name)
    {
        try
        {
            return *std::any_cast<T*>(variableMap.at(name));
        }
        catch (const std::out_of_range& e)
        {
            throw std::runtime_error("Variable not found: " + name);
        }
        catch (const std::bad_any_cast& e)
        {
            throw std::runtime_error("Incorrect type for variable: " + name);
        }
    }

    virtual ~Component() {
        std::cout << "DESTRUCTOR Component ejecutándose - Tipo: " << typeid(*this).name() << std::endl;
        destroy();
    }

    virtual std::string getComponentName() const = 0;
    virtual void defines() {}
    virtual void start() {}
    virtual void update() {}
    virtual std::string serializeComponent() const { return "{ }"; }
    virtual void deserialize(const std::string& data) {}

    virtual void setOwner(GameObject* owner) { this->owner = owner; }
    GameObject* getOwner() const { return owner; }

    // Sistema de estado del componente
    virtual bool isValid() const { return !isDestroyed && owner != nullptr; }
    virtual void destroy() {
        std::cout << "DESTROY ejecut�ndose - isDestroyed: " << isDestroyed << std::endl;
        isDestroyed = true;
        owner = nullptr;
    }
    virtual void enable() { isEnabled = true; }
    virtual void disable() { isEnabled = false; }
    bool isActive() const { return isEnabled && !isDestroyed; }

    int id = UIDGenerator::Generate();

protected:
    GameObject* owner = nullptr;
    bool isDestroyed = false;
    bool isEnabled = true;
};
