// projecthub.h
#pragma once

#include <string>
#include <vector>

class ProjectHub {
public:
    struct Project {
        std::string name;
        std::string path;
        Project(const std::string& n, const std::string& p)
            : name(n), path(p) {
        }
    };

    explicit ProjectHub(const std::string& baseDirectory);

    void render(); // Llama a esta función en tu loop principal ImGui

    // Opcional: recargar proyectos si cambian externamente
    void reloadProjects();

private:
    std::string basePath;
    std::vector<Project> projects;
    char newProjectName[128]{};
    int selected = -1;

    void readProjects();
};
