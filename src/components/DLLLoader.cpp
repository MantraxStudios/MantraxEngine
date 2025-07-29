#include "DLLLoader.h"
#include "GameBehaviourFactory.h"
#include <stdexcept>
#include "SceneManager.h"
#include "../wrapper/CoreWrapper.h"

DLLLoader* DLLLoader::instance = nullptr;

DLLLoader::DLLLoader()
{
    loader = std::make_unique<Loader>();
}

void DLLLoader::create()
{
    if (DLLLoader::instance)
        throw std::runtime_error("Dynamic lib loader already created.");

    DLLLoader::instance = new DLLLoader();
}

void DLLLoader::release()
{
    delete DLLLoader::instance;
    DLLLoader::instance = nullptr;
}

void DLLLoader::load_components(const std::string& _path)
{
    std::cout << "Initializing DLL" << std::endl;

    auto copy_file = [](const std::filesystem::path& from, const std::filesystem::path& to)
        {
            std::ifstream src(from, std::ios::binary);
            std::ofstream dst(to, std::ios::binary);
            dst << src.rdbuf();
        };

    // Configurar rutas (ajusta según lo necesites)
    from_dll_path = "MantraxLoader.dll";
    dll_path = "MantraxLoader.dll";

    // Liberar loader previo si existe
    if (loader && loader->is_loaded())
    {
        loader->free();
    }

    //// Reemplazar archivo dll
    //try
    //{
    //    if (std::filesystem::exists(dll_path))
    //    {
    //        std::filesystem::remove(dll_path);
    //        std::cout << "File Removed: " << dll_path << std::endl;
    //    }
    //    else
    //    {
    //        std::cout << "File Not Found: " << dll_path << std::endl;
    //    }
    //}
    //catch (const std::filesystem::filesystem_error& e)
    //{
    //    std::cerr << e.what() << std::endl;
    //}

    if (!std::filesystem::exists(from_dll_path))
    {
        std::cerr << "Source DLL not found: " << from_dll_path << std::endl;
        return;
    }

    copy_file(from_dll_path, dll_path);

    // Cargar la nueva DLL
    loader = std::make_unique<Loader>();
    loader->load(dll_path.c_str());

    if (!loader->is_loaded())
    {
        std::cerr << "DLL not loaded: " << dll_path << std::endl;
        return;
    }

    try
    {
        using FuncType = void (*)(GameBehaviourFactory*);
        auto func = loader->get_function<FuncType>("REGISTER_COMPONENTS");

        using FuncTypeAPI = void (*)(SceneManager*);
        auto func_graphics = loader->get_function<FuncTypeAPI>("REGISTER_APIS");

        if (!func)
        {
            std::cerr << "Error: No se pudo cargar la función 'REGISTER_COMPONENTS'." << std::endl;
            return;
        }

        GameBehaviourFactory* factoryPtr = &GameBehaviourFactory::instance();
        if (!factoryPtr)
        {
            std::cerr << "---------- Error: No se pudo obtener la instancia de GameBehaviourFactory." << std::endl;
            return;
        }

        func(factoryPtr);

        if (!func_graphics)
        {
            std::cout << "Failed to load components API" << std::endl;
            return;
        }

        func_graphics(&SceneManager::getInstance());


        typedef void (*LoadScene)(const char*);
        typedef void (*LoadEngineWrapper)(LoadScene);

        auto EngineWrapperLoad = loader->get_function<LoadEngineWrapper>("LoadEngineWrapper");

        EngineWrapperLoad([](const char* sceneNameCStr) {
            std::string sceneName(sceneNameCStr);
            CoreWrapper::ChangeScene(sceneName);
        });

        // Load Input System Wrapper Functions
        typedef void (*LoadInputWrapper)(
            InputAction* (*registerAction)(const char*, int),
            void (*addKeyBinding)(const char*, int, bool, int),
            void (*addMouseButtonBinding)(const char*, int),
            void (*addMouseAxisBinding)(const char*, int),
            bool (*getButton)(const char*),
            float (*getValue)(const char*),
            void (*getVector2D)(const char*, float*, float*),
            float (*getMouseAxis)(const char*),
            void (*bindButtonCallback)(const char*, void (*)(bool)),
            void (*bindValueCallback)(const char*, void (*)(float)),
            void (*bindVector2DCallback)(const char*, void (*)(float, float)),
            void (*bindMouseAxisCallback)(const char*, void (*)(float))
        );

        auto InputWrapperLoad = loader->get_function<LoadInputWrapper>("LoadInputWrapper");
        if (InputWrapperLoad) {
            InputWrapperLoad(
                CoreWrapper::RegisterInputAction,
                CoreWrapper::AddKeyBinding,
                CoreWrapper::AddMouseButtonBinding,
                CoreWrapper::AddMouseAxisBinding,
                CoreWrapper::GetInputActionButton,
                CoreWrapper::GetInputActionValue,
                CoreWrapper::GetInputActionVector2D,
                CoreWrapper::GetInputActionMouseAxis,
                CoreWrapper::BindButtonCallback,
                CoreWrapper::BindValueCallback,
                CoreWrapper::BindVector2DCallback,
                CoreWrapper::BindMouseAxisCallback
            );
            std::cout << "Input System Wrapper Installed In DLL Successfully" << std::endl;
        } else {
            std::cout << "Input System Wrapper not found in DLL (optional)" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cerr << "Started DLL System" << std::endl;
    dll_in_recompile = true;

    loader_dll_stamp = std::filesystem::last_write_time(from_dll_path).time_since_epoch().count();
}

void DLLLoader::assign_data()
{
    // Aquí puedes asignar datos si hace falta
}

void DLLLoader::update()
{
    // Aquí pondrías la lógica de actualización si fuera necesario
}

void DLLLoader::check_components_reload()
{
    const auto from_dll_path = "D:/Proyects/MantraxGame/MantraxLoader/Build/MantraxLoader.dll";
    auto stamp = std::filesystem::last_write_time(from_dll_path).time_since_epoch().count();

    if (stamp != loader_dll_stamp)
    {
        std::cout << "Reloading components..." << std::endl;
    }
}

Shared<Component> DLLLoader::create_component(const std::string& name) const
{
    auto it = factories.find(name);
    if (it != factories.end())
    {
        return it->second();
    }

    return nullptr;
}

void DLLLoader::reset_component_registry()
{
    factories.clear();
}
