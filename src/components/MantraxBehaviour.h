#pragma once
#include <iostream>
#include "../core/CoreExporter.h"
#include "GameObject.h"

class MANTRAXCORE_API MantraxBehaviour {
public:
    GameObject* Self = nullptr;

    virtual void OnInit() {}
    virtual void OnTick() {}
};