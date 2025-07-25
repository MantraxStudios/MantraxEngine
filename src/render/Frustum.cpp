#include "Frustum.h"
#include <algorithm>

Frustum::Frustum() {
    // Initialize all planes to default
    for (auto& plane : planes) {
        plane = Plane();
    }
}

void Frustum::extractFromMatrix(const glm::mat4& viewProjectionMatrix) {
    // Extraer los planos del frustum de la matriz view-projection combinada
    // Los planos se extraen de las filas de la matriz
    
    const float* m = &viewProjectionMatrix[0][0];
    
    // Left plane: m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]
    planes[LEFT].normal.x = m[3] + m[0];
    planes[LEFT].normal.y = m[7] + m[4];
    planes[LEFT].normal.z = m[11] + m[8];
    planes[LEFT].distance = m[15] + m[12];
    normalizePlane(planes[LEFT]);
    
    // Right plane: m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]
    planes[RIGHT].normal.x = m[3] - m[0];
    planes[RIGHT].normal.y = m[7] - m[4];
    planes[RIGHT].normal.z = m[11] - m[8];
    planes[RIGHT].distance = m[15] - m[12];
    normalizePlane(planes[RIGHT]);
    
    // Bottom plane: m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]
    planes[BOTTOM].normal.x = m[3] + m[1];
    planes[BOTTOM].normal.y = m[7] + m[5];
    planes[BOTTOM].normal.z = m[11] + m[9];
    planes[BOTTOM].distance = m[15] + m[13];
    normalizePlane(planes[BOTTOM]);
    
    // Top plane: m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]
    planes[TOP].normal.x = m[3] - m[1];
    planes[TOP].normal.y = m[7] - m[5];
    planes[TOP].normal.z = m[11] - m[9];
    planes[TOP].distance = m[15] - m[13];
    normalizePlane(planes[TOP]);
    
    // Near plane: m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]
    planes[NEAR].normal.x = m[3] + m[2];
    planes[NEAR].normal.y = m[7] + m[6];
    planes[NEAR].normal.z = m[11] + m[10];
    planes[NEAR].distance = m[15] + m[14];
    normalizePlane(planes[NEAR]);
    
    // Far plane: m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]
    planes[FAR].normal.x = m[3] - m[2];
    planes[FAR].normal.y = m[7] - m[6];
    planes[FAR].normal.z = m[11] - m[10];
    planes[FAR].distance = m[15] - m[14];
    normalizePlane(planes[FAR]);
}

void Frustum::normalizePlane(Plane& plane) {
    float magnitude = glm::length(plane.normal);
    if (magnitude > 0.0f) {
        plane.normal /= magnitude;
        plane.distance /= magnitude;
    }
}

CullResult Frustum::testPoint(const glm::vec3& point) const {
    // Un punto está dentro del frustum si está del lado positivo de todos los planos
    for (const auto& plane : planes) {
        if (plane.distanceToPoint(point) < 0.0f) {
            return CullResult::OUTSIDE;
        }
    }
    return CullResult::INSIDE;
}

CullResult Frustum::testBoundingSphere(const BoundingSphere& sphere) const {
    bool intersecting = false;
    
    for (const auto& plane : planes) {
        float distance = plane.distanceToPoint(sphere.center);
        
        if (distance < -sphere.radius) {
            // Esfera completamente fuera de este plano
            return CullResult::OUTSIDE;
        } else if (distance < sphere.radius) {
            // Esfera intersecta con este plano
            intersecting = true;
        }
        // Si distance >= sphere.radius, la esfera está completamente del lado positivo
    }
    
    return intersecting ? CullResult::INTERSECT : CullResult::INSIDE;
}

CullResult Frustum::testBoundingBox(const BoundingBox& box) const {
    bool intersecting = false;
    
    for (const auto& plane : planes) {
        // Obtener el vértice más cercano y más lejano al plano
        glm::vec3 nearVertex = box.min;
        glm::vec3 farVertex = box.max;
        
        // Para cada componente, elegir min o max basado en la normal del plano
        if (plane.normal.x > 0.0f) {
            nearVertex.x = box.max.x;
            farVertex.x = box.min.x;
        }
        if (plane.normal.y > 0.0f) {
            nearVertex.y = box.max.y;
            farVertex.y = box.min.y;
        }
        if (plane.normal.z > 0.0f) {
            nearVertex.z = box.max.z;
            farVertex.z = box.min.z;
        }
        
        // Si el vértice más cercano está fuera, toda la caja está fuera
        if (plane.distanceToPoint(nearVertex) < 0.0f) {
            return CullResult::OUTSIDE;
        }
        
        // Si el vértice más lejano está fuera, la caja intersecta
        if (plane.distanceToPoint(farVertex) < 0.0f) {
            intersecting = true;
        }
    }
    
    return intersecting ? CullResult::INTERSECT : CullResult::INSIDE;
}

CullResult Frustum::testTransformedBoundingBox(const BoundingBox& localBox, 
                                               const glm::mat4& transform) const {
    // Transformar las 8 esquinas de la bounding box
    std::array<glm::vec3, 8> corners = {
        glm::vec3(localBox.min.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.max.z)
    };
    
    // Transformar todas las esquinas
    for (auto& corner : corners) {
        glm::vec4 transformedCorner = transform * glm::vec4(corner, 1.0f);
        corner = glm::vec3(transformedCorner) / transformedCorner.w;
    }
    
    // Crear una nueva AABB de las esquinas transformadas
    glm::vec3 minPoint = corners[0];
    glm::vec3 maxPoint = corners[0];
    
    for (const auto& corner : corners) {
        minPoint = glm::min(minPoint, corner);
        maxPoint = glm::max(maxPoint, corner);
    }
    
    BoundingBox transformedBox(minPoint, maxPoint);
    return testBoundingBox(transformedBox);
} 