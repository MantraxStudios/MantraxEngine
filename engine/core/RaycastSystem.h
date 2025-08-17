#pragma once
#include <physx/PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <vector>
#include "CoreExporter.h"
#include "PhysicsEvents.h"

class MANTRAXCORE_API RaycastSystem {
private:
    static RaycastSystem* instance;
    
    RaycastSystem() = default;
    
public:
    ~RaycastSystem() = default;
    
    // Singleton access
    static RaycastSystem& getInstance();
    static void destroyInstance();
    
    // Raycast methods
    RaycastHit raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    std::vector<RaycastHit> raycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f);
    
    // Raycast with layer filtering
    RaycastHit raycastWithLayers(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxU32 layerMask);
    std::vector<RaycastHit> raycastAllWithLayers(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxU32 layerMask);
    
    // Sphere cast
    RaycastHit sphereCast(const glm::vec3& origin, const glm::vec3& direction, float sphereRadius, float maxDistance = 1000.0f);
    std::vector<RaycastHit> sphereCastAll(const glm::vec3& origin, const glm::vec3& direction, float sphereRadius, float maxDistance = 1000.0f);
    
    // Box cast
    RaycastHit boxCast(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& halfExtents, float maxDistance = 1000.0f);
    std::vector<RaycastHit> boxCastAll(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& halfExtents, float maxDistance = 1000.0f);
    
    // Overlap tests
    std::vector<RaycastHit> overlapSphere(const glm::vec3& center, float radius);
    std::vector<RaycastHit> overlapBox(const glm::vec3& center, const glm::vec3& halfExtents);
    
    // Utility methods
    bool isPointInCollider(const glm::vec3& point, physx::PxShape* shape, const glm::vec3& shapePosition);
    float distanceToCollider(const glm::vec3& point, physx::PxShape* shape, const glm::vec3& shapePosition);
    
private:
    // Helper methods
    RaycastHit performRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxQueryFilterData* filterData = nullptr);
    std::vector<RaycastHit> performRaycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxQueryFilterData* filterData = nullptr);
    
    // Convert PhysX results to RaycastHit
    RaycastHit convertHit(const physx::PxRaycastHit& pxHit);
    RaycastHit convertSweepHit(const physx::PxSweepHit& pxHit);
    RaycastHit convertOverlapHit(const physx::PxOverlapHit& pxHit, const glm::vec3& queryCenter);
};
