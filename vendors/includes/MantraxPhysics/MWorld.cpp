#include "MWorld.h"
#include <algorithm>

void MWorld::UpdateWorld(float deltaTime) {
    // 1. Integración
    for (size_t i = 0; i < Bodys.size(); i++) {
        if (Bodys[i])
            Bodys[i]->UpdateBody(deltaTime);
    }

    // 2. Colisiones
    for (size_t i = 0; i < Bodys.size(); i++) {
        MBody* a = Bodys[i];
        if (!a || !a->AttachShaped) continue;

        for (size_t j = i + 1; j < Bodys.size(); j++) {
            MBody* b = Bodys[j];
            if (!b || !b->AttachShaped) continue;

            if (CheckAABB(a, b)) {
                ResolveCollision(a, b);

                MCollisionBox* boxA = dynamic_cast<MCollisionBox*>(a->AttachShaped);
                MCollisionBox* boxB = dynamic_cast<MCollisionBox*>(b->AttachShaped);
                if (boxA && boxA->isTrigger) {
                    // evento trigger
                }
                if (boxB && boxB->isTrigger) {
                    // evento trigger
                }
            }
        }
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