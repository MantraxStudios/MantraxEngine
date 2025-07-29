#include "ScriptExecutor.h"

void ScriptExecutor::setOwner(GameObject* owner) {
    Component::setOwner(owner);
}

void ScriptExecutor::start() {

    try
    {
        Script = GameBehaviourFactory::instance().create_instance_by_name(scriptClassName);

        if (Script == nullptr)
        {
            std::cerr << "Factory returned nullptr for ClassName: " << scriptClassName << std::endl;
        }
        else
        {
            try
            {
                Script.get()->Self = getOwner();
                Script.get()->OnInit();
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }

            std::cerr << "Factory returned " << scriptClassName << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::string errorMessage = "Error on init: ";
    }
}

void ScriptExecutor::update() {
    try
    {
        if (Script != nullptr)
        {
            Script->OnTick();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error en update: " << e.what() << '\n';
    }
}

void ScriptExecutor::destroy() {
    if (Script != nullptr)
    {
        Script.release();
    }
}