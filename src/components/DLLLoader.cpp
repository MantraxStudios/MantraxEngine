#include "DLLLoader.h"
#include "../input/InputSystem.h"
#include <SDL.h>

#include <windows.h>
#include <iostream>

typedef void (*HolaFunc)();

int DLLLoader::Inyection() {
    HMODULE h = LoadLibrary(L"D:/Proyects/MantraxGame/MantraxLoader/Build/MantraxLoader.dll");
    if (!h)
    {
        std::cout << "No se pudo cargar la DLL" << std::endl;
        return -1;
    }

    HolaFunc HolaDesdeDLL = (HolaFunc)GetProcAddress(h, "HolaDesdeDLL");
    if (!HolaDesdeDLL)
    {
        std::cout << "No se encontró la función" << std::endl;
        return -1;
    }

    HolaDesdeDLL(); // Llamada a la función del DLL

    FreeLibrary(h);
    return 0;
}


void DLLLoader::Shutdown() {

}
