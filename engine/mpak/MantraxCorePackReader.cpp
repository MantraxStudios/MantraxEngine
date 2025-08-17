#include "MantraxCorePackReader.h"
#include <fstream>
#include <cstring>

MantraxCorePackReader::MantraxCorePackReader(const std::string& corePackFile)
    : corePackPath(corePackFile), valid(false)
{
    std::ifstream in(corePackFile, std::ios::binary);
    if (!in) return;

    uint32_t fileCount = 0;
    in.read((char*)&fileCount, sizeof(fileCount));
    if (!fileCount) return;

    entries.resize(fileCount);
    for (uint32_t i = 0; i < fileCount; ++i) {
        in.read((char*)&entries[i], sizeof(CorePackEntry));
    }

    valid = true;
}

bool MantraxCorePackReader::IsValid() const {
    return valid;
}

std::vector<std::string> MantraxCorePackReader::ListFiles() const {
    std::vector<std::string> names;
    for (const auto& e : entries) {
        names.push_back(std::string(e.name));
    }
    return names;
}

bool MantraxCorePackReader::ReadFile(const std::string& fileName, std::vector<char>& outData) const {
    for (const auto& e : entries) {
        if (fileName == std::string(e.name)) {
            std::ifstream in(corePackPath, std::ios::binary);
            if (!in) return false;
            in.seekg(e.offset, std::ios::beg);
            outData.resize(e.size);
            in.read(outData.data(), e.size);
            return true;
        }
    }
    return false;
}
