#pragma once
#include <iostream>
#include <string>
#include <sol/sol.hpp>
#include "../core/CoreExporter.h"

class MANTRAXCORE_API CoreWrapper {
public:
	void Register(sol::state& lua);
	void RegisterInput(sol::state& lua);
	void RegisterDebug(sol::state& lua);
	void RegisterMaths(sol::state& lua);
	void RegisterGameObject(sol::state& lua);
	void RegisterCharacterController(sol::state& lua);
	void RegisterPhysicalObject(sol::state& lua);
	void RegisterLightComponent(sol::state& lua);
	void RegisterAudioSource(sol::state& lua);
	void RegisterScriptExecutor(sol::state& lua);
};