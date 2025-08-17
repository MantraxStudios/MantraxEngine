#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <components/SceneManager.h>
#include <components/GameObject.h>
#include <components/LightComponent.h>
#include <components/PhysicalObject.h>
#include <components/Rigidbody.h>
#include <components/Collider.h>
#include <components/ScriptExecutor.h>
#include <components/CharacterController.h>
#include <components/AudioSource.h>
#include <components/SpriteAnimator.h>
#include <render/Texture.h>
#include <components/GameBehaviourFactory.h>
#include <render/MaterialManager.h>
#include <core/PhysicsManager.h>
#include <core/RaycastSystem.h>
#include <render/RenderConfig.h>
#include "../EUI/UIBuilder.h"
#include <nlohmann/json.hpp>
#include "../Windows/FileExplorer.h"
#include <core/FileSystem.h>
#include "../EUI/ComponentSerializer.h"
#include <glm/gtc/type_ptr.hpp>

#define ChangeVar(T, Direccion, ValorNuevo) (*(T*)(Direccion) = (ValorNuevo))

// Forward declaration for state management
struct ComponentUIState;

class ComponentSerializer
{
private:
    // State management per GameObject
    static std::unordered_map<GameObject*, std::unique_ptr<ComponentUIState>> objectStates;
    
    // Helper to get or create state for a GameObject
    static ComponentUIState* getOrCreateState(GameObject* go);
    
public:
    // Component renderers
    static void RenderAudioSourceComponent(GameObject* go);
    static void RenderLightComponent(GameObject* go);
    static void RenderSpriteAnimatorComponent(GameObject* go);
    static void RenderPhysicalObjectComponent(GameObject* go);
    static void RenderRigidbodyComponent(GameObject* go);
    static void RenderColliderComponent(GameObject* go);
    static void RenderScriptExecutorComponent(GameObject* go);
    static void RenderCharacterControllerComponent(GameObject* go);
    
    // Cleanup method to remove state for deleted objects
    static void cleanupObjectState(GameObject* go);
    
    // Clear all states (useful for scene changes)
    static void clearAllStates();

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

// Structure to hold UI state per GameObject
struct ComponentUIState {
    // SpriteAnimator states
    char materialName[128] = "SpriteMaterial";
    glm::vec3 albedoColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.3f;
    glm::vec3 emissiveColor = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec2 tiling = glm::vec2(1.0f, 1.0f);
    bool showFileDialog = false;
    char animatorFilePath[256] = "";
    std::string lastDroppedAnimatorFile = "";
    char newStateName[64] = "New State";
    char playbackStateName[64] = "";
    char texturePath[256] = "";
    
    // PhysicalObject states
    int currentCollisionGroup = 0;
    bool collisionMaskStatic = true;
    bool collisionMaskDynamic = true;
    bool collisionMaskTrigger = false;
    bool collisionMaskPlayer = false;
    bool collisionMaskEnemy = false;
    bool collisionMaskProjectile = false;
    bool collisionMaskSensor = false;
    glm::vec3 forceVector = glm::vec3(0.0f, 0.0f, 0.0f);
    bool showForceDialog = false;
    glm::vec3 torqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
    bool showTorqueDialog = false;
    glm::vec3 impulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
    bool showImpulseDialog = false;
    glm::vec3 raycastOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 raycastDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    float raycastDistance = 100.0f;
    RaycastHit lastRaycastHit;
    
    // ScriptExecutor states
    char scriptPathBuffer[256] = "";
    
    // Rigidbody states
    bool rigidbodyShowForceDialog = false;
    bool rigidbodyShowTorqueDialog = false;
    bool rigidbodyShowImpulseDialog = false;
    glm::vec3 rigidbodyForceVector = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rigidbodyTorqueVector = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rigidbodyImpulseVector = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Collider states
    // (Most collider state is handled by the component itself)
    
    // Raycast states
    glm::vec3 raycastOriginGlobal = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 raycastDirectionGlobal = glm::vec3(0.0f, 0.0f, 1.0f);
    float raycastDistanceGlobal = 100.0f;
    RaycastHit lastRaycastHitGlobal;
};