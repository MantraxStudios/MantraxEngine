#pragma once
#include "UIDGenerator.h"
#include <atomic>
#include "../core/CoreExporter.h"

std::atomic<unsigned int> UIDGenerator::counter{ 1 };