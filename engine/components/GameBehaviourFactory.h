#pragma once
#include <iostream>
#include <functional>
#include <unordered_map>
#include <memory>               
#include "MantraxBehaviour.h"
#include "../core/CoreExporter.h"

class MANTRAXCORE_API GameBehaviourFactory
{
public:
    using Creator = std::function<std::unique_ptr<MantraxBehaviour>()>;
    std::unordered_map<std::string, Creator> creators;

    static GameBehaviourFactory& instance()
    {
        static GameBehaviourFactory factory;
        return factory;
    }

    void register_class(const std::string& class_name, Creator creator)
    {
        try
        {
            creators[class_name] = std::move(creator);
            std::cout << "Class Registre: " << class_name << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Trapped exception: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unknown exception trapped" << std::endl;
        }
    }

    void use_registered_class(const std::string& class_name)
    {
        if (creators.find(class_name) != creators.end())
        {
            std::cout << "Creating instance of: " << class_name << std::endl;
            std::unique_ptr<MantraxBehaviour> instance = creators[class_name]();
        }
        else
        {
            std::cout << "Error: Class not found: " << class_name << std::endl;
        }
    }

    std::unique_ptr<MantraxBehaviour> create_instance_by_name(const std::string& class_name)
    {
        auto it = creators.find(class_name);
        if (it != creators.end())
        {
            std::cout << "******** Clase encontrada, creando instancia: " << class_name << std::endl;
            return it->second();
        }
        else
        {
            std::cerr << "Error Clase " << class_name << " no encontrada en el registro" << std::endl;
            return nullptr;
        }
    }

    void reload_components()
    {
        creators.clear();
        std::cout << "Components clearing succefully" << std::endl;
    }

    std::vector<std::string> get_registered_class_names() const
    {
        std::vector<std::string> names;
        names.reserve(creators.size());
        for (const auto& pair : creators) {
            names.push_back(pair.first);
        }
        return names;
    }

    GameBehaviourFactory() = default;

    GameBehaviourFactory(const GameBehaviourFactory&) = delete;
    GameBehaviourFactory& operator=(const GameBehaviourFactory&) = delete;
};

#define MANTRAXCLASS(ClassName)                                                                                                                                    \
    namespace                                                                                                                                                      \
    {                                                                                                                                                              \
        const bool registered_##ClassName =                                                                                                                        \
            (GameBehaviourFactory::instance().register_class(#ClassName, []() -> std::unique_ptr<MantraxBehaviour> { return std::make_unique<ClassName>(); }), true); \
    }
