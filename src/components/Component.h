#pragma once
#include "../core/CoreExporter.h"

class GameObject;

class MANTRAXCORE_API Component {
public:
    virtual ~Component() = default;
    virtual void start() {}
    virtual void update() {}

    void setOwner(GameObject* owner) { this->owner = owner; }
    GameObject* getOwner() const { return owner; }

protected:
    GameObject* owner = nullptr;
};
