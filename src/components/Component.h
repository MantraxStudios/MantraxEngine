#pragma once
#include "../core/CoreExporter.h"

class GameObject;

class MANTRAXCORE_API Component {
public:
    virtual ~Component() { destroy(); }
    virtual void start() {}
    virtual void update() {}

    virtual void setOwner(GameObject* owner) { this->owner = owner; }
    GameObject* getOwner() const { return owner; }

    // Sistema de estado del componente
    virtual bool isValid() const { return !isDestroyed && owner != nullptr; }
    virtual void destroy() { isDestroyed = true; owner = nullptr; }
    virtual void enable() { isEnabled = true; }
    virtual void disable() { isEnabled = false; }
    bool isActive() const { return isEnabled && !isDestroyed; }

protected:
    GameObject* owner = nullptr;
    bool isDestroyed = false;
    bool isEnabled = true;
};
