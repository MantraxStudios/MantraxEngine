#include "ScriptExecutor.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include "../core/FileSystem.h"

using json = nlohmann::json;

// Initialize static member
std::vector<ScriptExecutor*> ScriptExecutor::s_instances;

void ScriptExecutor::defines() {
    set_var("LuaPath", &luaPath);
    
    // Register this instance
    s_instances.push_back(this);
}

void ScriptExecutor::setOwner(GameObject* owner) {
    Component::setOwner(owner);
}

void ScriptExecutor::start() {
    // Clear state at the beginning
    scriptTable = sol::table();
    scriptLoaded = false;
    lastError.clear();
    
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    // Register the self() function to return the owner GameObject
    lua.set_function("self", [this]() { 
        GameObject* owner = getOwner();
        if (!owner) {
            std::cerr << "[ScriptExecutor] Warning: Owner GameObject is null in self() function" << std::endl;
        }
        return owner; 
    });
    
    CoreWrapper coreWrapper;
    coreWrapper.Register(lua);

    std::string fullPath = FileSystem::getProjectPath() + "\\Content\\" + luaPath + ".lua";

    // Check if file exists before trying to load it
    if (!std::filesystem::exists(fullPath)) {
        std::cerr << "[ScriptExecutor] LUA file does not exist: " << fullPath << std::endl;
        lastError = "Script file not found: " + fullPath;
        return;
    }

    try {
        lua.script_file(fullPath);

        if (lua[luaPath].valid() && lua[luaPath].get_type() == sol::type::table) {
            scriptTable = lua[luaPath];
        }
        else {
            scriptTable = lua.globals();
        }

        // Set script as loaded if we got here successfully
        scriptLoaded = true;
        lastError.clear();

        sol::function startFunc = scriptTable["OnStart"];
        if (startFunc.valid()) {
            try {
                startFunc();
            }
            catch (const sol::error& e) {
                lastError = e.what();
                std::cerr << "[ScriptExecutor] Error in OnStart() of " << luaPath << ": " << e.what() << std::endl;
            }
            catch (const std::exception& e) {
                lastError = e.what();
                std::cerr << "[ScriptExecutor] Exception in OnStart() of " << luaPath << ": " << e.what() << std::endl;
            }
        }
    }
    catch (const sol::error& e) {
        lastError = e.what();
        std::cerr << "[ScriptExecutor] Error loading script " << fullPath << ": " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        lastError = e.what();
        std::cerr << "[ScriptExecutor] Exception loading script " << fullPath << ": " << e.what() << std::endl;
    }
}

void ScriptExecutor::update() {
    // Don't execute if script is not loaded or file doesn't exist
    if (!scriptLoaded || !isScriptValid()) {
        return;
    }

    // Check if script table is valid
    if (!scriptTable.valid()) {
        return;
    }

    sol::function updateFunc = scriptTable["OnTick"];
    if (updateFunc.valid()) {
        try {
            updateFunc();
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error in update() of script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::destroy() {
    // Don't execute if script is not loaded or file doesn't exist
    if (!scriptLoaded || !isScriptValid()) {
        return;
    }

    // Check if script table is valid
    if (!scriptTable.valid()) {
        return;
    }

    // Call onDestroy() in Lua if it exists
    sol::function destroyFunc = scriptTable["OnDestroy"];
    if (destroyFunc.valid()) {
        try {
            destroyFunc();
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error in OnDestroy() of script " << luaPath << ": " << e.what() << std::endl;
        }
    }
    
    // Clear script table and set as not loaded
    scriptTable = sol::table();
    scriptLoaded = false;
}

void ScriptExecutor::reloadScript() {
    // Call destroy for proper cleanup
    destroy();
    
    // Clear lua state completely
    lua = sol::state();
    
    // Restart the script
    start();
}

bool ScriptExecutor::isScriptValid() const {
    if (!scriptLoaded) {
        return false;
    }
    
    // Check if script file still exists on disk
    std::string fullPath = FileSystem::getProjectPath() + "\\Content\\" + luaPath + ".lua";
    return std::filesystem::exists(fullPath);
}

bool ScriptExecutor::hasFunction(const std::string& functionName) const {
    if (!isScriptValid()) {
        return false;
    }
    
    if (!scriptTable.valid()) {
        return false;
    }
    
    sol::function func = scriptTable[functionName];
    return func.valid();
}

void ScriptExecutor::onTriggerEnter(GameObject* other) {
    if (!scriptLoaded || !isScriptValid() || !scriptTable.valid()) {
        return;
    }

    if (!other) {
        return;
    }

    // Call OnTriggerEnter() in Lua if it exists
    sol::function triggerEnterFunc = scriptTable["OnTriggerEnter"];
    if (triggerEnterFunc.valid()) {
        try {
            triggerEnterFunc(other);
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error in OnTriggerEnter() of script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::onTriggerExit(GameObject* other) {
    if (!scriptLoaded || !isScriptValid() || !scriptTable.valid()) {
        return;
    }

    if (!other) {
        return;
    }

    // Call OnTriggerExit() in Lua if it exists
    sol::function triggerExitFunc = scriptTable["OnTriggerExit"];
    if (triggerExitFunc.valid()) {
        try {
            triggerExitFunc(other);
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error in OnTriggerExit() of script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::notifyScriptDeleted(const std::string& scriptName) {
    std::cout << "[ScriptExecutor] Notifying all instances about deleted script: " << scriptName << std::endl;
    
    for (auto* instance : s_instances) {
        if (instance && instance->luaPath == scriptName) {
            std::cout << "[ScriptExecutor] Stopping execution for script: " << scriptName << std::endl;
            instance->destroy();
            instance->lastError = "Script file was deleted";
            instance->scriptLoaded = false;
        }
    }
}

void ScriptExecutor::notifyScriptModified(const std::string& scriptName) {
    std::cout << "[ScriptExecutor] Notifying all instances about modified script: " << scriptName << std::endl;
    
    for (auto* instance : s_instances) {
        if (instance && instance->luaPath == scriptName) {
            std::cout << "[ScriptExecutor] Reloading script: " << scriptName << std::endl;
            instance->reloadScript();
        }
    }
}

std::string ScriptExecutor::serializeComponent() const {
    json j;
    j["luaPath"] = luaPath;
    j["enabled"] = isEnabled;
    return j.dump();
}

void ScriptExecutor::deserialize(const std::string& data) {
    try {
        json j = json::parse(data);

        // Only assign if it exists and is string
        if (j.contains("luaPath") && j["luaPath"].is_string()) {
            luaPath = j["luaPath"];
            
            // Check if script exists before reloading
            std::string fullPath = FileSystem::getProjectPath() + "\\Content\\" + luaPath + ".lua";
            if (std::filesystem::exists(fullPath)) {
                reloadScript();
            }
            else {
                std::cerr << "[ScriptExecutor] LUA file does not exist: " << fullPath << std::endl;
                lastError = "Script file not found: " + fullPath;
                scriptLoaded = false;
            }
        }
        else {
            std::cerr << "[ScriptExecutor] Missing or invalid 'luaPath' field in serialized component. Script will not be reloaded." << std::endl;
        }

        // Optional: enabled/disabled
        if (j.contains("enabled")) {
            isEnabled = j.value("enabled", true);
        }
    }
    catch (const json::exception& e) {
        std::cerr << "[ScriptExecutor] Error deserializing JSON: " << e.what() << std::endl;
    }
}
