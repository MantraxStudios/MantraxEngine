#pragma once

#ifdef MANTRAXCORE_EXPORTS
#define MANTRAXCORE_API __declspec(dllexport)
#else
#define MANTRAXCORE_API __declspec(dllimport)
#endif