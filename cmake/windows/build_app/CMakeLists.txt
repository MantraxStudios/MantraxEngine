cmake_minimum_required(VERSION 3.10)
project(MantraxApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
    add_compile_options(/bigobj)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

# Directorio de código fuente del ejecutable
set(MANTRAX_APP_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../editor/")
set(MANTRAX_INCLUDES_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../vendors/windows/")

# Archivos fuente del ejecutable
file(GLOB_RECURSE MANTRAX_APP_SOURCES CONFIGURE_DEPENDS
    "${MANTRAX_APP_SRC_DIR}/*"
    "${MANTRAX_INCLUDES_SRC_DIR}/includes/imgui/*"
)

# Crear el ejecutable
add_executable(${PROJECT_NAME} ${MANTRAX_APP_SOURCES})

# =================== BUSCAR OpenGL ===================
find_package(OpenGL REQUIRED)

# =================== INCLUIR DIRECTORIOS ===================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../engine/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/SDL3/
    ${OPENGL_INCLUDE_DIRS}
)

# =================== LIBRERÍAS ===================
file(GLOB_RECURSE GARIN_LIBS
    "../../../vendors/windows/libs/*.lib"
)

# MantraxCore precompilada
set(MANTRAX_CORE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../build_lib/bin/Debug/MantraxCore.lib")

# Librerías de sistema en Windows para OpenGL
set(WINDOWS_SYSTEM_LIBS
    opengl32.lib
    glu32.lib
    gdi32.lib
    user32.lib
    kernel32.lib
    winmm.lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../vendors/windows/includes/SDL3/")
    list(APPEND WINDOWS_SYSTEM_LIBS
        ole32.lib
        oleaut32.lib
        imm32.lib
        version.lib
        setupapi.lib
        advapi32.lib
    )
endif()

# =================== DEFINICIONES ===================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLM_ENABLE_EXPERIMENTAL
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    OPENGL_ENABLED
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WIN32_WINNT=0x0601
        WINVER=0x0601
    )
endif()

# =================== ENLAZAR ===================
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MANTRAX_CORE_LIB}     # Tu MantraxCore.lib
    ${GARIN_LIBS}
    ${OPENGL_LIBRARIES}
    ${WINDOWS_SYSTEM_LIBS}
)

# =================== CONFIGURACIONES DEBUG/RELEASE ===================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/ZI>
        $<$<CONFIG:Release>:/O2>
        /W0
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/INCREMENTAL>
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )
endif()
