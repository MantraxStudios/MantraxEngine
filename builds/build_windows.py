# -*- coding: utf-8 -*-
import os
import subprocess
import sys
import shutil

def run_cmake(bin_path):
    os.makedirs(bin_path, exist_ok=True)
    subprocess.run(['cmake', '..', '-G', 'Visual Studio 17 2022'], cwd=bin_path, check=True)

def build_solution(bin_path, solution_name, config="Debug"):
    # Verificar si la solución existe
    sln_path = os.path.join(bin_path, solution_name)
    if not os.path.exists(sln_path):
        raise FileNotFoundError(f"No se encontró la solución: {sln_path}")

    # Compilar con MSBuild
    subprocess.run(['msbuild', solution_name, f'/p:Configuration={config}'], cwd=bin_path, check=True)

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_lib_path = os.path.join(script_dir, '..', 'cmake', 'windows/build_lib', 'bin')
    build_app_path = os.path.join(script_dir, '..', 'cmake', 'windows/build_app', 'bin')

    # 1️⃣ Compilar MantraxCore
    run_cmake(build_lib_path)
    build_solution(build_lib_path, "MantraxCore.sln")

    # Verificar que la librería exista
    lib_path = os.path.join(build_lib_path, "Debug", "MantraxCore.lib")
    if not os.path.exists(lib_path):
        print(f"ERROR: No se encontró la librería {lib_path}")
        sys.exit(1)

    # Carpeta Debug de la app
    debug_folder = os.path.join(build_app_path, "Debug")
    os.makedirs(debug_folder, exist_ok=True)

    # Copiar DLL de MantraxCore a Debug de la app
    dll_src = os.path.join(build_lib_path, "Debug", "MantraxCore.dll")
    dll_dst = os.path.join(debug_folder, "MantraxCore.dll")
    if os.path.exists(dll_src):
        shutil.copy2(dll_src, dll_dst)
        print(f"Copiado {dll_src} -> {dll_dst}")
    else:
        print(f"ERROR: No se encontró el DLL {dll_src}")
        sys.exit(1)

    # Copiar contenido de Data al Debug de la app (sin crear la carpeta Data)
    data_src = os.path.join(script_dir, "Data")
    if os.path.exists(data_src):
        for root, dirs, files in os.walk(data_src):
            rel_path = os.path.relpath(root, data_src)
            dest_path = os.path.join(debug_folder, rel_path)  # directo en Debug
            os.makedirs(dest_path, exist_ok=True)
            for file in files:
                shutil.copy2(os.path.join(root, file), os.path.join(dest_path, file))
        print(f"Copiado contenido de {data_src} -> {debug_folder}")
    else:
        print(f"ADVERTENCIA: No se encontró la carpeta Data en {script_dir}")

    # 2️⃣ Compilar MantraxApp
    run_cmake(build_app_path)
    build_solution(build_app_path, "MantraxApp.sln")

    # 3️⃣ Abrir carpeta Debug de build_app
    if os.path.exists(debug_folder):
        print(f"Abriendo carpeta: {debug_folder}")
        os.startfile(debug_folder)  # Solo en Windows
    else:
        print(f"ERROR: No se encontró la carpeta {debug_folder}")

if __name__ == "__main__":
    main()
