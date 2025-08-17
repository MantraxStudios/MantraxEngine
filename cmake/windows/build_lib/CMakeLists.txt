cmake_minimum_required(VERSION 3.10)
project(MantraxCore)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set("Project_Name" MantraxCore)

if (MSVC)
    add_compile_options(/bigobj)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

set(MANTRAX_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../src")

file(GLOB_RECURSE MANTRAX_COMPONENTS_SOURCES   CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/components/*")
file(GLOB_RECURSE MANTRAX_COREMODULE_SOURCES   CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/core/*")
file(GLOB_RECURSE MANTRAX_INPUT_SOURCES        CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/input/*")
file(GLOB_RECURSE MANTRAX_MAIN_SOURCES         CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/main/*")
file(GLOB_RECURSE MANTRAX_MPAK_SOURCES         CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/mpak/*")
file(GLOB_RECURSE MANTRAX_RENDER_SOURCES       CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/render/*")
file(GLOB_RECURSE MANTRAX_UI_SOURCES           CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/ui/*")
file(GLOB_RECURSE MANTRAX_UIELEMENTS_SOURCES   CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/ui/UIElements/*")
file(GLOB_RECURSE MANTRAX_WRAPPER_SOURCES      CONFIGURE_DEPENDS "${MANTRAX_SRC_DIR}/wrapper/*")

# Lista agregada final
set(MANTRAX_CORE_SOURCES
  ${MANTRAX_COMPONENTS_SOURCES}
  ${MANTRAX_COREMODULE_SOURCES}
  ${MANTRAX_INPUT_SOURCES}
  ${MANTRAX_MAIN_SOURCES}
  ${MANTRAX_MPAK_SOURCES}
  ${MANTRAX_RENDER_SOURCES}
  ${MANTRAX_UI_SOURCES}
  ${MANTRAX_UIELEMENTS_SOURCES}
  ${MANTRAX_WRAPPER_SOURCES}
)

# PRIMERO: Crear la biblioteca
add_library(${Project_Name} SHARED ${MANTRAX_CORE_SOURCES})

# DESPUÉS: Configurar precompiled headers (CMake 3.16+ requerido)
if (MSVC AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${Project_Name} PRIVATE
        <memory>
        <vector>
        <string>
        <unordered_map>
        <iostream>
        <windows.h>
    )
endif()

# =================== BUSCAR OpenGL ===================
find_package(OpenGL REQUIRED)

# =================== INCLUIR DIRECTORIOS ===================
target_include_directories(${Project_Name} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/SDL3/
    ${OPENGL_INCLUDE_DIRS}
)

# =================== ENLAZAR BIBLIOTECAS ===================

# Bibliotecas de tu proyecto
file(GLOB_RECURSE GARIN_LIBS
    "../../../vendors/windows/libs/*.lib"
)

# Bibliotecas del sistema Windows para OpenGL
set(WINDOWS_SYSTEM_LIBS
    opengl32.lib      # OpenGL básico de Windows
    glu32.lib         # GLU (OpenGL Utility Library)
    gdi32.lib         # GDI (Graphics Device Interface)
    user32.lib        # User32 para ventanas
    kernel32.lib      # Kernel básico de Windows
    winmm.lib         # Windows Multimedia (para timers, etc)
)

# Si usas SDL3 con OpenGL, también necesitas estas:
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/SDL3/")
    list(APPEND WINDOWS_SYSTEM_LIBS
        ole32.lib         # OLE para SDL
        oleaut32.lib      # OLE Automation
        imm32.lib         # Input Method Manager
        version.lib       # Version info
        setupapi.lib      # Setup API
        advapi32.lib      # Advanced API
    )
endif()

# =================== DEFINICIONES DEL COMPILADOR ===================
target_compile_definitions(${Project_Name} PRIVATE 
    MANTRAXCORE_EXPORTS 
    GLM_ENABLE_EXPERIMENTAL
    WIN32_LEAN_AND_MEAN     # Incluye solo lo esencial de Windows
    NOMINMAX                # Evita conflictos con std::min/max
    OPENGL_ENABLED          # Tu flag personalizado para OpenGL
)

# Solo en Windows
if(WIN32)
    target_compile_definitions(${Project_Name} PRIVATE
        _WIN32_WINNT=0x0601     # Windows 7 o superior
        WINVER=0x0601           # Windows 7 o superior
    )
endif()

# =================== ENLAZAR TODAS LAS LIBRERÍAS ===================
target_link_libraries(${Project_Name} PRIVATE 
    ${GARIN_LIBS}           # Tus librerías personalizadas
    ${OPENGL_LIBRARIES}     # OpenGL encontrado por find_package
    ${WINDOWS_SYSTEM_LIBS}  # Librerías del sistema Windows
)

# =================== CONFIGURACIÓN ADICIONAL PARA RELEASE/DEBUG ===================
if(MSVC)
    # Configuraciones específicas para Release/Debug
    target_compile_options(${Project_Name} PRIVATE
        $<$<CONFIG:Debug>:/ZI>      # Edit and Continue para Debug
        $<$<CONFIG:Release>:/O2>    # Optimización máxima para Release
    )
    
    # Configuración del linker
    target_link_options(${Project_Name} PRIVATE
        $<$<CONFIG:Debug>:/INCREMENTAL>     # Linking incremental en Debug
        $<$<CONFIG:Release>:/INCREMENTAL:NO> # No incremental en Release
    )
endif()