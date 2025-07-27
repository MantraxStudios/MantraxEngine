#pragma once
#include "libs/MMaths.hpp"
#include "MBody.h"
#include <iostream>
#include <vector>


class MWorld
{
public:
	std::vector<MBody*> Bodys = std::vector<MBody*>();
	Vector3 GravityDirection = Vector3(0.0f, -1.0f, 0.0f);
	float WorldGravity = 9.81f;

	void UpdateWorld(float deltaTime);
	MBody* CreateBody();
	bool RemoveBody(MBody* body);
};