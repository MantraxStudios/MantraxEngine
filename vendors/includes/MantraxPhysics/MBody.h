#pragma once
#include "libs/MMaths.hpp"

enum TypeBody {
	bStatic,
	bDynamic
};

class MBody
{
public:
	Vector3 Position;
	Vector3 Velocity;
	Vector3 AngularVelocity;
	Quaternion Rotation;

	TypeBody BodyType = TypeBody::bDynamic;
	
	bool isSleeping;
	bool UseGravity = true;
	
	void UpdateBody(float deltaTime);
};