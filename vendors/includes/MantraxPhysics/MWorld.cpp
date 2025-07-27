#include "MWorld.h"
#include <algorithm>

void MWorld::UpdateWorld(float deltaTime) {
	for (size_t i = 0; i < Bodys.size(); i++)
	{
		if (Bodys[i])
			Bodys[i]->UpdateBody(deltaTime);
	}
}

MBody* MWorld::CreateBody() {
	MBody* newBody = new MBody();
	
	if (newBody) {
		Bodys.push_back(newBody);
		return newBody;
	}

	return nullptr;
}

bool MWorld::RemoveBody(MBody* body) {
	if (!body) return false;
	
	auto it = std::find(Bodys.begin(), Bodys.end(), body);
	if (it != Bodys.end()) {
		Bodys.erase(it);
		delete body;
		return true;
	}
	
	return false;
}