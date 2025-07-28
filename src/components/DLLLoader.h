#pragma once 
#include <windows.h>
#include <metahost.h>
#include <iostream>
#include <string>
#pragma comment(lib, "mscoree.lib")

#include "../core/CoreExporter.h"

class MANTRAXCORE_API DLLLoader {
public:
	static ICLRMetaHost* pMetaHost;
	static ICLRRuntimeInfo* pRuntimeInfo;
	static ICLRRuntimeHost* pClrRuntimeHost;

	static int Inyection();
	static void Shutdown();
};