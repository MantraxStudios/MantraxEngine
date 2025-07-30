#include "ScriptExecutor.h"
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void ScriptExecutor::setOwner(GameObject* owner) {
    Component::setOwner(owner);
}

void ScriptExecutor::start() {
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    lua["ThisObject"] = sol::make_object(lua.lua_state(), getOwner());
    CoreWrapper coreWrapper;
    coreWrapper.Register(lua);

    std::string fullPath = "x64/debug/Scripts/" + luaPath + ".lua";

    // Revisa que el archivo exista ANTES de intentar cargarlo
    if (!std::filesystem::exists(fullPath)) {
        std::cerr << "[ScriptExecutor] Archivo LUA no existe: " << fullPath << std::endl;
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

        sol::function startFunc = scriptTable["OnStart"];
        if (startFunc.valid()) {
            try {
                startFunc();
            }
            catch (const sol::error& e) {
                std::cerr << "[ScriptExecutor] Error en OnStart() de " << luaPath << ": " << e.what() << std::endl;
            }
        }
    }
    catch (const sol::error& e) {
        std::cerr << "[ScriptExecutor] Error cargando script " << fullPath << ">> " << e.what() << std::endl;
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
    try {
        json j = json::parse(data);

        // Solo asigna si existe y es string
        if (j.contains("luaPath") && j["luaPath"].is_string()) {
            luaPath = j["luaPath"];

            // Verifica si el script existe antes de recargarlo
            std::string scriptFile = "x64/debug/Scripts/" + luaPath + ".lua";
            if (std::filesystem::exists(scriptFile)) {
                reloadScript();
            }
            else {
                std::cerr << "[ScriptExecutor] Archivo LUA no existe: " << scriptFile << std::endl;
            }
        }
        else {
            std::cerr << "[ScriptExecutor] Campo 'luaPath' faltante o inválido en el componente serializado. NO se recarga script.\n";
        }

        // Opcional: habilitado/disabled
        if (j.contains("enabled")) {
            isEnabled = j.value("enabled", true);
        }
    }
    catch (const json::exception& e) {
        std::cerr << "[ScriptExecutor] Error al deserializar JSON: " << e.what() << std::endl;
    }
}
