#pragma once
#include <iostream>
#include <string>
#include "GameObject.h"
#include "../wrapper/CoreWrapper.h"
#include "../core/CoreExporter.h"
#include <sol/sol.hpp>

class MANTRAXCORE_API ScriptExecutor : public Component
{
public:
    std::string luaPath = "ExampleScript";

    void setOwner(GameObject* owner) override;
    void update() override;
    void start() override;
    void destroy() override;

    // Inspector helper methods
    bool isScriptLoaded() const { return scriptTable.valid(); }
    std::string getLastError() const { return lastError; }
    void reloadScript();
    bool hasFunction(const std::string& functionName) const;

    // Trigger event methods
    void onTriggerEnter(GameObject* other);
    void onTriggerExit(GameObject* other);

private:
    sol::state lua;
    sol::table scriptTable;
    std::string lastError;
    bool scriptLoaded = false;
};
