#pragma once
#include "../core/CoreExporter.h"
#include <string>
#include <vector>

class MANTRAXCORE_API MantraxCorePackBuilder {
public:
    static bool Build(const std::vector<std::string>& files, const std::string& outCorePack);
};
