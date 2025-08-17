#pragma once 
#include "libs/MMaths.hpp"

class MCollisionBehaviour {
public:
	Vector3 Size;

	virtual ~MCollisionBehaviour() {}
};


class MCollisionBox : public MCollisionBehaviour {
public:
	bool contactListener;
	bool isTrigger;
};