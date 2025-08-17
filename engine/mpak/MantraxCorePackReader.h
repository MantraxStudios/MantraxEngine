#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API MantraxCorePackReader {
public:
    MantraxCorePackReader(const std::string& corePackFile);

    bool IsValid() const;

    std::vector<std::string> ListFiles() const;

    bool ReadFile(const std::string& fileName, std::vector<char>& outData) const;

private:
    struct CorePackEntry {
        char name[100];
        uint64_t offset;
        uint64_t size;
    };

    std::string corePackPath;
    std::vector<CorePackEntry> entries;
    bool valid;
};
