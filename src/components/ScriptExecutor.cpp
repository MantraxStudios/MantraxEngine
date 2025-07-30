#include "ScriptExecutor.h"
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ScriptExecutor::setOwner(GameObject* owner) {
    Component::setOwner(owner);
}

void ScriptExecutor::start() {
    // Abrimos librerías de Lua
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    lua["ThisObject"] = sol::make_object(lua.lua_state(), getOwner());
    CoreWrapper coreWrapper;
    coreWrapper.Register(lua);

    // Ruta final del script
    std::string fullPath = "x64/debug/Scripts/" + luaPath + ".lua";


    try {
        // Ejecutar el script
        lua.script_file(fullPath);

        // Si el script devuelve una tabla global con el mismo nombre
        if (lua[luaPath].valid() && lua[luaPath].get_type() == sol::type::table) {
            scriptTable = lua[luaPath];
        }
        else {
            // Si no hay tabla, usar el entorno global
            scriptTable = lua.globals();
        }

        // Llamar a start() en Lua si existe
        sol::function startFunc = scriptTable["OnStart"];
        if (startFunc.valid()) {
            startFunc();
        }

    }
    catch (const sol::error& e) {
        std::cerr << "Error cargando script " << fullPath << ": " << e.what() << std::endl;
    }
}

void ScriptExecutor::update() {
    // Llamar a update() en Lua si existe
    sol::function updateFunc = scriptTable["OnTick"];
    if (updateFunc.valid()) {
        try {
            updateFunc();
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error en update() del script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::destroy() {
    // Llamar a onDestroy() en Lua si existe
    sol::function destroyFunc = scriptTable["onDestroy"];
    if (destroyFunc.valid()) {
        try {
            destroyFunc();
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error en onDestroy() del script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::reloadScript() {
    // Clear current state
    scriptTable = sol::table();
    scriptLoaded = false;
    lastError.clear();
    
    // Restart the script
    start();
}

bool ScriptExecutor::hasFunction(const std::string& functionName) const {
    if (!scriptTable.valid()) {
        return false;
    }
    
    sol::function func = scriptTable[functionName];
    return func.valid();
}

void ScriptExecutor::onTriggerEnter(GameObject* other) {
    if (!scriptTable.valid() || !other) {
        return;
    }

    // Llamar a OnTriggerEnter() en Lua si existe
    sol::function triggerEnterFunc = scriptTable["OnTriggerEnter"];
    if (triggerEnterFunc.valid()) {
        try {
            triggerEnterFunc(other);
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error en OnTriggerEnter() del script " << luaPath << ": " << e.what() << std::endl;
        }
    }
}

void ScriptExecutor::onTriggerExit(GameObject* other) {
    if (!scriptTable.valid() || !other) {
        return;
    }

    // Llamar a OnTriggerExit() en Lua si existe
    sol::function triggerExitFunc = scriptTable["OnTriggerExit"];
    if (triggerExitFunc.valid()) {
        try {
            triggerExitFunc(other);
            lastError.clear(); // Clear error if successful
        }
        catch (const sol::error& e) {
            lastError = e.what();
            std::cerr << "Error en OnTriggerExit() del script " << luaPath << ": " << e.what() << std::endl;
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
    json j = json::parse(data);
    luaPath = j.value("luaPath", "ExampleScript");

    // Puedes recargar automáticamente el script:
    reloadScript();
}
