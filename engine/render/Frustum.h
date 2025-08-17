#pragma once
#include <glm/glm.hpp>
#include <array>
#include "../core/CoreExporter.h"

// Forward declarations
struct BoundingBox;
struct BoundingSphere;

// Estructura para representar un plano del frustum
struct MANTRAXCORE_API Plane {
    glm::vec3 normal;
    float distance;

    Plane() : normal(0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}

    inline float distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

// AABB
struct MANTRAXCORE_API BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    BoundingBox() : min(0.0f), max(0.0f) {}
    BoundingBox(const glm::vec3& minPoint, const glm::vec3& maxPoint)
        : min(minPoint), max(maxPoint) {
    }

    inline glm::vec3 getCenter() const { return (min + max) * 0.5f; }
    inline glm::vec3 getSize() const { return max - min; }
};

// Bounding Sphere
struct MANTRAXCORE_API BoundingSphere {
    glm::vec3 center;
    float radius;

    BoundingSphere() : center(0.0f), radius(0.0f) {}
    BoundingSphere(const glm::vec3& c, float r) : center(c), radius(r) {}
};

// Resultados de culling
enum class MANTRAXCORE_API CullResult {
    INSIDE,
    OUTSIDE,
    INTERSECT
};

class MANTRAXCORE_API Frustum {
public:
    enum PlaneIndex {
        LEFT = 0,
        RIGHT,
        BOTTOM,
        TOP,
        NEAR_PLANE,  // Cambiado de NEAR a NEAR_PLANE para evitar conflictos
        FAR_PLANE    // Cambiado de FAR a FAR_PLANE para evitar conflictos
    };

    static constexpr size_t PLANE_COUNT = 6;

    // Constructor
    Frustum();

    // Métodos públicos - solo declaraciones en el header
    void extractFromMatrix(const glm::mat4& viewProjectionMatrix);
    CullResult testBoundingBox(const BoundingBox& box) const;
    CullResult testBoundingSphere(const BoundingSphere& sphere) const;
    CullResult testPoint(const glm::vec3& point) const;
    CullResult testTransformedBoundingBox(const BoundingBox& localBox,
        const glm::mat4& transform) const;

    // Métodos inline para acceso a planos
    inline const Plane& getPlane(PlaneIndex index) const {
        return planes[static_cast<size_t>(index)];
    }

    inline const std::array<Plane, PLANE_COUNT>& getPlanes() const {
        return planes;
    }

private:
    std::array<Plane, PLANE_COUNT> planes;

    // Método privado - solo declaración
    void normalizePlane(Plane& plane);
};