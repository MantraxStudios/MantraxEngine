#pragma once
#include "libs/MMaths.hpp"
#include "MCollision.h"

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

	MCollisionBehaviour* AttachShaped;
	
	void UpdateBody(float deltaTime);
};