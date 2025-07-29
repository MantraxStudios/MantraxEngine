#pragma once
#include <iostream>
#include <string>
#include <sol/sol.hpp>

class CoreWrapper {
public:
	void Register(sol::state& lua);
	void RegisterInput(sol::state& lua);
	void RegisterDebug(sol::state& lua);
	void RegisterMaths(sol::state& lua);
	void RegisterGameObject(sol::state& lua);
};