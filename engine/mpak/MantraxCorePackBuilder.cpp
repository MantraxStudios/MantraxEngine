#include "MantraxCorePackBuilder.h"
#include <fstream>
#include <cstring>
#include <iostream>

const int FNAME_SIZE = 100;

struct CorePackEntry {
    char name[FNAME_SIZE];
    uint64_t offset;
    uint64_t size;
};

bool MantraxCorePackBuilder::Build(const std::vector<std::string>& files, const std::string& outCorePack) {
    std::ofstream out(outCorePack, std::ios::binary);
    if (!out) return false;

    uint32_t fileCount = files.size();
    out.write((char*)&fileCount, sizeof(fileCount));

    std::vector<CorePackEntry> entries(fileCount);
    size_t headerSize = sizeof(uint32_t) + fileCount * sizeof(CorePackEntry);
    size_t dataOffset = headerSize;

    for (size_t i = 0; i < files.size(); ++i) {
        strncpy_s(entries[i].name, files[i].c_str(), FNAME_SIZE - 1);
        entries[i].name[FNAME_SIZE - 1] = '\0';
        std::ifstream in(files[i], std::ios::binary | std::ios::ate);
        if (!in) {
            std::cerr << "No se puede abrir: " << files[i] << std::endl;
            return false;
        }
        entries[i].offset = dataOffset;
        entries[i].size = in.tellg();
        dataOffset += entries[i].size;
    }

    for (size_t i = 0; i < files.size(); ++i) {
        out.write((char*)&entries[i], sizeof(CorePackEntry));
    }

    for (size_t i = 0; i < files.size(); ++i) {
        std::ifstream in(files[i], std::ios::binary);
        std::vector<char> buffer(entries[i].size);
        in.read(buffer.data(), entries[i].size);
        out.write(buffer.data(), entries[i].size);
        std::cout << "Empaquetado: " << files[i] << " (" << entries[i].size << " bytes)\n";
    }

    out.close();
    std::cout << "CorePack creado: " << outCorePack << std::endl;
    return true;
}
