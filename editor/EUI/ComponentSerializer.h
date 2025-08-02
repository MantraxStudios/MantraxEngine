#pragma once
#include <iostream>
#include <string>
#include "../../src/components/SceneManager.h"
#include "../../src/components/GameObject.h"
#include "../../src/components/LightComponent.h"
#include "../../src/components/PhysicalObject.h"
#include "../../src/components/ScriptExecutor.h"
#include "../../src/components/CharacterController.h"
#include "../../src/components/AudioSource.h"
#include "../../src/components/SpriteAnimator.h"
#include "../../src/render/Texture.h"
#include "../EUI/UIBuilder.h"
#include "../../src/components/GameBehaviourFactory.h"
#include "../../src/render/MaterialManager.h"
#include "../../src/core/PhysicsManager.h"
#include "../../src/render/RenderConfig.h"
#include <nlohmann/json.hpp>
#include "../Windows/FileExplorer.h"
#include <core/FileSystem.h>
#include "../EUI/ComponentSerializer.h"
#include <glm/gtc/type_ptr.hpp>

#define ChangeVar(T, Direccion, ValorNuevo) (*(T*)(Direccion) = (ValorNuevo))

class ComponentSerializer
{
public:
    // Component renderers
    static void RenderAudioSourceComponent(GameObject* go);
    static void RenderLightComponent(GameObject* go);
    static void RenderSpriteAnimatorComponent(GameObject* go);
    static void RenderPhysicalObjectComponent(GameObject* go);
    static void RenderScriptExecutorComponent(GameObject* go);
    static void RenderCharacterControllerComponent(GameObject* go);

    // Helper function to calculate collision mask from checkboxes
    static CollisionMask CalculateCollisionMask(bool staticObj, bool dynamicObj, bool trigger, bool player, bool enemy, bool projectile, bool sensor) {
        CollisionMask mask = static_cast<CollisionMask>(0);
        if (staticObj) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::STATIC_MASK));
        if (dynamicObj) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::DYNAMIC_MASK));
        if (trigger) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::TRIGGER_MASK));
        if (player) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::PLAYER_MASK));
        if (enemy) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::ENEMY_MASK));
        if (projectile) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::PROJECTILE_MASK));
        if (sensor) mask = static_cast<CollisionMask>(static_cast<physx::PxU32>(mask) | static_cast<physx::PxU32>(CollisionMask::SENSOR_MASK));
        return mask;
    }
};