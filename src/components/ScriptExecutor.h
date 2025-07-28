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
    MantraxBehaviour* Script;
    std::string scriptClassName;
    bool autoStart;

public:
    ScriptExecutor();
    ~ScriptExecutor();
    
    void setOwner(GameObject* owner) override;
    void update() override;
    void start() override;
    void destroy() override;
    
    // Script management
    void setScriptClassName(const std::string& className);
    std::string getScriptClassName() const { return scriptClassName; }
    
    MantraxBehaviour* getScript() const { return Script; }
    
    // Execution control
    void loadScript();
    void unloadScript();
    bool hasScript() const { return Script != nullptr; }
    
    // Configuration
    void setAutoStart(bool autoStart);
    bool getAutoStart() const { return autoStart; }
    
    // Utility
    void resetToDefaults();
};