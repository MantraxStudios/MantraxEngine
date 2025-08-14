#include "RaycastSystem.h"
#include "PhysicsManager.h"
#include <iostream>

RaycastSystem* RaycastSystem::instance = nullptr;

RaycastSystem& RaycastSystem::getInstance() {
    if (!instance) {
        instance = new RaycastSystem();
    }
    return *instance;
}

void RaycastSystem::destroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

RaycastHit RaycastSystem::raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    return performRaycast(origin, direction, maxDistance);
}

std::vector<RaycastHit> RaycastSystem::raycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    return performRaycastAll(origin, direction, maxDistance);
}

RaycastHit RaycastSystem::raycastWithLayers(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxU32 layerMask) {
    physx::PxQueryFilterData filterData;
    filterData.data.word0 = layerMask;
    filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
    
    return performRaycast(origin, direction, maxDistance, &filterData);
}

std::vector<RaycastHit> RaycastSystem::raycastAllWithLayers(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxU32 layerMask) {
    physx::PxQueryFilterData filterData;
    filterData.data.word0 = layerMask;
    filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
    
    return performRaycastAll(origin, direction, maxDistance, &filterData);
}

RaycastHit RaycastSystem::sphereCast(const glm::vec3& origin, const glm::vec3& direction, float sphereRadius, float maxDistance) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    if (!scene) {
        return RaycastHit(); // Empty hit
    }
    
    physx::PxSphereGeometry sphereGeom(sphereRadius);
    physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z));
    physx::PxVec3 unitDir(direction.x, direction.y, direction.z);
    unitDir.normalize();
    
    physx::PxSweepBuffer hit;
    bool hasHit = scene->sweep(sphereGeom, pose, unitDir, maxDistance, hit);
    
    if (hasHit && hit.hasBlock) {
        return convertSweepHit(hit.block);
    }
    
    return RaycastHit(); // Empty hit
}

std::vector<RaycastHit> RaycastSystem::sphereCastAll(const glm::vec3& origin, const glm::vec3& direction, float sphereRadius, float maxDistance) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    std::vector<RaycastHit> hits;
    
    if (!scene) {
        return hits;
    }
    
    physx::PxSphereGeometry sphereGeom(sphereRadius);
    physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z));
    physx::PxVec3 unitDir(direction.x, direction.y, direction.z);
    unitDir.normalize();
    
    const physx::PxU32 bufferSize = 256;
    physx::PxSweepHit hitBuffer[bufferSize];
    physx::PxSweepBuffer sweepBuffer(hitBuffer, bufferSize);
    
    bool hasHit = scene->sweep(sphereGeom, pose, unitDir, maxDistance, sweepBuffer);
    
    if (hasHit) {
        for (physx::PxU32 i = 0; i < sweepBuffer.nbTouches; ++i) {
            hits.push_back(convertSweepHit(sweepBuffer.touches[i]));
        }
        
        if (sweepBuffer.hasBlock) {
            hits.push_back(convertSweepHit(sweepBuffer.block));
        }
    }
    
    return hits;
}

RaycastHit RaycastSystem::boxCast(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& halfExtents, float maxDistance) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    if (!scene) {
        return RaycastHit(); // Empty hit
    }
    
    physx::PxBoxGeometry boxGeom(halfExtents.x, halfExtents.y, halfExtents.z);
    physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z));
    physx::PxVec3 unitDir(direction.x, direction.y, direction.z);
    unitDir.normalize();
    
    physx::PxSweepBuffer hit;
    bool hasHit = scene->sweep(boxGeom, pose, unitDir, maxDistance, hit);
    
    if (hasHit && hit.hasBlock) {
        return convertSweepHit(hit.block);
    }
    
    return RaycastHit(); // Empty hit
}

std::vector<RaycastHit> RaycastSystem::boxCastAll(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& halfExtents, float maxDistance) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    std::vector<RaycastHit> hits;
    
    if (!scene) {
        return hits;
    }
    
    physx::PxBoxGeometry boxGeom(halfExtents.x, halfExtents.y, halfExtents.z);
    physx::PxTransform pose(physx::PxVec3(origin.x, origin.y, origin.z));
    physx::PxVec3 unitDir(direction.x, direction.y, direction.z);
    unitDir.normalize();
    
    const physx::PxU32 bufferSize = 256;
    physx::PxSweepHit hitBuffer[bufferSize];
    physx::PxSweepBuffer sweepBuffer(hitBuffer, bufferSize);
    
    bool hasHit = scene->sweep(boxGeom, pose, unitDir, maxDistance, sweepBuffer);
    
    if (hasHit) {
        for (physx::PxU32 i = 0; i < sweepBuffer.nbTouches; ++i) {
            hits.push_back(convertSweepHit(sweepBuffer.touches[i]));
        }
        
        if (sweepBuffer.hasBlock) {
            hits.push_back(convertSweepHit(sweepBuffer.block));
        }
    }
    
    return hits;
}

std::vector<RaycastHit> RaycastSystem::overlapSphere(const glm::vec3& center, float radius) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    std::vector<RaycastHit> hits;
    
    if (!scene) {
        return hits;
    }
    
    physx::PxSphereGeometry sphereGeom(radius);
    physx::PxTransform pose(physx::PxVec3(center.x, center.y, center.z));
    
    const physx::PxU32 bufferSize = 256;
    physx::PxOverlapHit hitBuffer[bufferSize];
    physx::PxOverlapBuffer overlapBuffer(hitBuffer, bufferSize);
    
    bool hasHit = scene->overlap(sphereGeom, pose, overlapBuffer);
    
    if (hasHit) {
        for (physx::PxU32 i = 0; i < overlapBuffer.nbTouches; ++i) {
            hits.push_back(convertOverlapHit(overlapBuffer.touches[i], center));
        }
    }
    
    return hits;
}

std::vector<RaycastHit> RaycastSystem::overlapBox(const glm::vec3& center, const glm::vec3& halfExtents) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    std::vector<RaycastHit> hits;
    
    if (!scene) {
        return hits;
    }
    
    physx::PxBoxGeometry boxGeom(halfExtents.x, halfExtents.y, halfExtents.z);
    physx::PxTransform pose(physx::PxVec3(center.x, center.y, center.z));
    
    const physx::PxU32 bufferSize = 256;
    physx::PxOverlapHit hitBuffer[bufferSize];
    physx::PxOverlapBuffer overlapBuffer(hitBuffer, bufferSize);
    
    bool hasHit = scene->overlap(boxGeom, pose, overlapBuffer);
    
    if (hasHit) {
        for (physx::PxU32 i = 0; i < overlapBuffer.nbTouches; ++i) {
            hits.push_back(convertOverlapHit(overlapBuffer.touches[i], center));
        }
    }
    
    return hits;
}

bool RaycastSystem::isPointInCollider(const glm::vec3& point, physx::PxShape* shape, const glm::vec3& shapePosition) {
    if (!shape) return false;
    
    physx::PxTransform shapePose(physx::PxVec3(shapePosition.x, shapePosition.y, shapePosition.z));
    physx::PxGeometryHolder geom = shape->getGeometry();
    
    physx::PxVec3 pxPoint(point.x, point.y, point.z);
    return physx::PxGeometryQuery::pointDistance(pxPoint, geom.any(), shapePose) <= 0.0f;
}

float RaycastSystem::distanceToCollider(const glm::vec3& point, physx::PxShape* shape, const glm::vec3& shapePosition) {
    if (!shape) return FLT_MAX;
    
    physx::PxTransform shapePose(physx::PxVec3(shapePosition.x, shapePosition.y, shapePosition.z));
    physx::PxGeometryHolder geom = shape->getGeometry();
    
    physx::PxVec3 pxPoint(point.x, point.y, point.z);
    return physx::PxGeometryQuery::pointDistance(pxPoint, geom.any(), shapePose);
}

// Private helper methods
RaycastHit RaycastSystem::performRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxQueryFilterData* filterData) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    if (!scene) {
        return RaycastHit(); // Empty hit
    }
    
    physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);
    physx::PxVec3 rayDir(direction.x, direction.y, direction.z);
    rayDir.normalize();
    
    physx::PxRaycastBuffer hit;
    physx::PxQueryFilterData defaultFilter;
    if (!filterData) {
        defaultFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
        filterData = &defaultFilter;
    }
    
    bool hasHit = scene->raycast(rayOrigin, rayDir, maxDistance, hit, physx::PxHitFlag::eDEFAULT, *filterData);
    
    if (hasHit && hit.hasBlock) {
        return convertHit(hit.block);
    }
    
    return RaycastHit(); // Empty hit
}

std::vector<RaycastHit> RaycastSystem::performRaycastAll(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, physx::PxQueryFilterData* filterData) {
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();
    
    std::vector<RaycastHit> hits;
    
    if (!scene) {
        return hits;
    }
    
    physx::PxVec3 rayOrigin(origin.x, origin.y, origin.z);
    physx::PxVec3 rayDir(direction.x, direction.y, direction.z);
    rayDir.normalize();
    
    const physx::PxU32 bufferSize = 256;
    physx::PxRaycastHit hitBuffer[bufferSize];
    physx::PxRaycastBuffer raycastBuffer(hitBuffer, bufferSize);
    
    physx::PxQueryFilterData defaultFilter;
    if (!filterData) {
        defaultFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
        filterData = &defaultFilter;
    }
    
    bool hasHit = scene->raycast(rayOrigin, rayDir, maxDistance, raycastBuffer, physx::PxHitFlag::eDEFAULT, *filterData);
    
    if (hasHit) {
        for (physx::PxU32 i = 0; i < raycastBuffer.nbTouches; ++i) {
            hits.push_back(convertHit(raycastBuffer.touches[i]));
        }
        
        if (raycastBuffer.hasBlock) {
            hits.push_back(convertHit(raycastBuffer.block));
        }
    }
    
    return hits;
}

RaycastHit RaycastSystem::convertHit(const physx::PxRaycastHit& pxHit) {
    RaycastHit hit;
    hit.hit = true;
    hit.position = glm::vec3(pxHit.position.x, pxHit.position.y, pxHit.position.z);
    hit.normal = glm::vec3(pxHit.normal.x, pxHit.normal.y, pxHit.normal.z);
    hit.distance = pxHit.distance;
    hit.actor = pxHit.actor;
    hit.shape = pxHit.shape;
    return hit;
}

RaycastHit RaycastSystem::convertSweepHit(const physx::PxSweepHit& pxHit) {
    RaycastHit hit;
    hit.hit = true;
    hit.position = glm::vec3(pxHit.position.x, pxHit.position.y, pxHit.position.z);
    hit.normal = glm::vec3(pxHit.normal.x, pxHit.normal.y, pxHit.normal.z);
    hit.distance = pxHit.distance;
    hit.actor = pxHit.actor;
    hit.shape = pxHit.shape;
    return hit;
}

RaycastHit RaycastSystem::convertOverlapHit(const physx::PxOverlapHit& pxHit, const glm::vec3& queryCenter) {
    RaycastHit hit;
    hit.hit = true;
    hit.actor = pxHit.actor;
    hit.shape = pxHit.shape;
    
    // For overlap hits, we need to calculate position and normal manually
    if (pxHit.actor) {
        physx::PxTransform actorPose = pxHit.actor->getGlobalPose();
        hit.position = glm::vec3(actorPose.p.x, actorPose.p.y, actorPose.p.z);
        
        // Calculate direction from query center to hit position
        glm::vec3 direction = hit.position - queryCenter;
        float distance = glm::length(direction);
        hit.distance = distance;
        
        if (distance > 0.001f) {
            hit.normal = glm::normalize(direction);
        } else {
            hit.normal = glm::vec3(0, 1, 0); // Default up
        }
    }
    
    return hit;
}
