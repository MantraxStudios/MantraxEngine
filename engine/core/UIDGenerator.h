#pragma once
#include <atomic>
#include "../core/CoreExporter.h"


class MANTRAXCORE_API UIDGenerator
{
public:
    static unsigned int Generate()
    {
        return counter++;
    }

private:
    static std::atomic<unsigned int> counter;
};
