#pragma once
#include <iostream>
#include <string>
#include "GameObject.h"
#include "MantraxBehaviour.h"
#include "GameBehaviourFactory.h"
#include "../core/CoreExporter.h"

class MANTRAXCORE_API ScriptExecutor : public Component
{
private:
    std::unique_ptr<MantraxBehaviour> Script;
public:
    std::string scriptClassName = "FirstScript";
    void setOwner(GameObject* owner) override;
    void update() override;
    void start() override;
    void destroy() override;
};