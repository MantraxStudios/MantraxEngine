#include "ScriptExecutor.h"

ScriptExecutor::ScriptExecutor() 
    : Script(nullptr)
    , scriptClassName("")
    , autoStart(false)
{
    // Ensure Script is properly initialized
    Script = nullptr;
}

ScriptExecutor::~ScriptExecutor() {
    destroy();
}

void ScriptExecutor::setOwner(GameObject* owner) {
    Component::setOwner(owner);
}

void ScriptExecutor::start() {
    if (autoStart && !scriptClassName.empty()) {
        loadScript();
    }
}

void ScriptExecutor::update() {
    if (!isActive() || !Script) {
        return;
    }
    
    try {
        // Call the script's OnTick method
        Script->OnTick();
    }
    catch (const std::exception& e) {
        // Log error but don't crash
        std::cerr << "Error in script OnTick: " << e.what() << std::endl;
    }
    catch (...) {
        // Log unknown error but don't crash
        std::cerr << "Unknown error in script OnTick" << std::endl;
    }
}

void ScriptExecutor::destroy() {
    unloadScript();
    Component::destroy();
}

void ScriptExecutor::setScriptClassName(const std::string& className) {
    scriptClassName = className;
}

void ScriptExecutor::loadScript() {
    if (scriptClassName.empty()) {
        return;
    }
    
    // Unload any existing script first
    unloadScript();
    
    try {
        // Create new script instance using GameBehaviourFactory
        auto scriptInstance = GameBehaviourFactory::instance().create_instance_by_name(scriptClassName);
        if (scriptInstance) {
            Script = scriptInstance.release();
            if (Script && owner) {
                Script->Self = owner;
                
                // Call OnInit on the script
                Script->OnInit();
            }
        }
    }
    catch (const std::exception& e) {
        // Log error and clean up
        std::cerr << "Error loading script: " << e.what() << std::endl;
        unloadScript();
    }
    catch (...) {
        // Log unknown error and clean up
        std::cerr << "Unknown error loading script: " << scriptClassName << std::endl;
        unloadScript();
    }
}

void ScriptExecutor::unloadScript() {
    if (Script) {
        try {
            delete Script;
        }
        catch (...) {
            // Ignore errors during deletion
        }
        Script = nullptr;
    }
}

void ScriptExecutor::setAutoStart(bool autoStart) {
    this->autoStart = autoStart;
}

void ScriptExecutor::resetToDefaults() {
    unloadScript();
    scriptClassName.clear();
    autoStart = false;
}