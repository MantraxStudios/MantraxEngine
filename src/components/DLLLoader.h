#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Loader.h"
#include "../components/Component.h"
#include "../core/CoreExporter.h"
#include "types.h"

class Component;
class MANTRAXCORE_API DLLLoader
{
public:
    static DLLLoader* instance;
    std::unordered_map<std::string, Function<Shared<Component>()>> factories;
    std::unique_ptr<Loader> loader;
    uint64_t loader_dll_stamp = 0;

    std::string from_dll_path;
    std::string dll_path;
    bool dll_in_recompile = false;

    DLLLoader();

    static void create();
    static void release();

    void load_components(const std::string& _path);
    void check_components_reload();
    void reset_component_registry();
    void update();
    void assign_data();

    Shared<Component> create_component(const std::string& name) const;
};
