#pragma once
#include "../core/CoreExporter.h"
#include <map>
#include <string>
#include <any>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <iostream>

class GameObject;

class MANTRAXCORE_API Component {
public:
    Component() : owner(nullptr), isDestroyed(false), isEnabled(true) {}
    
    virtual ~Component() {
        std::cout << "DESTRUCTOR Component ejecutándose - Tipo: " << typeid(*this).name() << std::endl;
        destroy();
    }

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

protected:
    GameObject* owner = nullptr;
    bool isDestroyed = false;
    bool isEnabled = true;
};
