# -*- coding: utf-8 -*-
import os
import subprocess
import platform

def run_windows_script():
    # Run the build_windows.py script
    subprocess.run(['python', 'builds/build_windows.py'], check=True)

def run_linux_build():
    def create_bin(path):
        bin_path = os.path.join(path, "bin")
        if not os.path.exists(bin_path):
            os.makedirs(bin_path)
            print(f'Folder "bin" successfully created at {path}.')
        else:
            print(f'The folder "bin" already exists at {path}.')
        return bin_path

    def build_linux(source_path):
        bin_path = create_bin(source_path)
        subprocess.run(['cmake', source_path], cwd=bin_path, check=True)
        subprocess.run(['make'], cwd=bin_path, check=True)

    build_lib_path = os.path.abspath("cmake/build_lib")
    build_app_path = os.path.abspath("cmake/build_app")

    build_linux(build_lib_path)
    build_linux(build_app_path)

def main():
    print("=== Build Menu ===")
    print("1. Compile on Windows (Visual Studio)")
    print("2. Compile on Linux (Working on it for Linux, not yet compatible)")
    choice = input("Select an option (1 or 2): ")

    if choice == '1':
        if platform.system() != "Windows":
            print("Warning: you are running Windows commands on a non-Windows system.")
        run_windows_script()
    elif choice == '2':
        run_linux_build()
    else:
        print("Invalid option. Exiting.")

    input("Press Enter to exit...")

if __name__ == "__main__":
    main()
