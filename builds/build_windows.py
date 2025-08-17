# -*- coding: utf-8 -*-
import os
import subprocess
import sys

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
    build_lib_path = os.path.join(script_dir, '..', 'cmake', 'build_lib', 'bin')
    build_app_path = os.path.join(script_dir, '..', 'cmake', 'build_app', 'bin')

    # 1️⃣ Compilar MantraxCore
    run_cmake(build_lib_path)
    build_solution(build_lib_path, "MantraxCore.sln")

    # Verificar que la librería exista
    lib_path = os.path.join(build_lib_path, "Debug", "MantraxCore.lib")
    if not os.path.exists(lib_path):
        print(f"ERROR: No se encontró la librería {lib_path}")
        sys.exit(1)

    # 2️⃣ Compilar MantraxApp
    run_cmake(build_app_path)
    build_solution(build_app_path, "MantraxApp.sln")

if __name__ == "__main__":
    main()
