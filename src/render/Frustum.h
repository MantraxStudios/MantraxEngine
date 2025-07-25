#pragma once
#include <glm/glm.hpp>
#include <array>
#include "../core/CoreExporter.h"


// Forward declarations
struct MANTRAXCORE_API BoundingBox;
struct MANTRAXCORE_API BoundingSphere;

// Estructura para representar un plano del frustum
struct MANTRAXCORE_API Plane {
    glm::vec3 normal;
    float distance;
    
    Plane() : normal(0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}
    
    // Calcula la distancia de un punto al plano
    float distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

// Estructura para bounding box (AABB)
struct MANTRAXCORE_API BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
    
    BoundingBox() : min(0.0f), max(0.0f) {}
    BoundingBox(const glm::vec3& minPoint, const glm::vec3& maxPoint) 
        : min(minPoint), max(maxPoint) {}
    
    glm::vec3 getCenter() const { return (min + max) * 0.5f; }
    glm::vec3 getSize() const { return max - min; }
};

// Estructura para bounding sphere
struct MANTRAXCORE_API BoundingSphere {
    glm::vec3 center;
    float radius;
    
    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
};

// Enums para resultados de culling
enum class MANTRAXCORE_API CullResult {
    INSIDE,     // Completamente dentro del frustum
    OUTSIDE,    // Completamente fuera del frustum
    INTERSECT   // Parcialmente dentro del frustum
};

class MANTRAXCORE_API Frustum {
public:
    // Indices de los planos del frustum
    enum PlaneIndex {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5,
        PLANE_COUNT = 6
    };

    Frustum();
    
    // Extrae los planos del frustum de las matrices view-projection
    void extractFromMatrix(const glm::mat4& viewProjectionMatrix);
    
    // Métodos de culling para diferentes tipos de bounding volumes
    CullResult testBoundingBox(const BoundingBox& box) const;
    CullResult testBoundingSphere(const BoundingSphere& sphere) const;
    CullResult testPoint(const glm::vec3& point) const;
    
    // Método helper para testear una AABB transformada
    CullResult testTransformedBoundingBox(const BoundingBox& localBox, 
                                          const glm::mat4& transform) const;
    
    // Getters
    const Plane& getPlane(PlaneIndex index) const { return planes[index]; }
    const std::array<Plane, PLANE_COUNT>& getPlanes() const { return planes; }

private:
    std::array<Plane, PLANE_COUNT> planes;
    
    // Normaliza un plano
    void normalizePlane(Plane& plane);
}; 