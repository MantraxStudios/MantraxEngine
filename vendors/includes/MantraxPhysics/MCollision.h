#pragma once 
#include "libs/MMaths.hpp"

class MCollisionBehaviour {
public:
	Vector3 Size;
};


class MCollisionBox : public MCollisionBehaviour {
public:
	bool contactListener;
	bool isTrigger;
};