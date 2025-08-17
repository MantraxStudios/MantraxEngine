#include "frustum.h"
#include <glm/gtc/matrix_transform.hpp>

// Constructor
Frustum::Frustum() {
    // Inicializar planos por defecto
    for (auto& plane : planes) {
        plane = Plane();
    }
}

void Frustum::extractFromMatrix(const glm::mat4& viewProjectionMatrix) {
    const glm::mat4& m = viewProjectionMatrix;

    // Extraer planos del frustum desde la matriz view-projection
    // Plano izquierdo: m[3] + m[0]
    planes[LEFT].normal.x = m[0][3] + m[0][0];
    planes[LEFT].normal.y = m[1][3] + m[1][0];
    planes[LEFT].normal.z = m[2][3] + m[2][0];
    planes[LEFT].distance = m[3][3] + m[3][0];
    normalizePlane(planes[LEFT]);

    // Plano derecho: m[3] - m[0]
    planes[RIGHT].normal.x = m[0][3] - m[0][0];
    planes[RIGHT].normal.y = m[1][3] - m[1][0];
    planes[RIGHT].normal.z = m[2][3] - m[2][0];
    planes[RIGHT].distance = m[3][3] - m[3][0];
    normalizePlane(planes[RIGHT]);

    // Plano inferior: m[3] + m[1]
    planes[BOTTOM].normal.x = m[0][3] + m[0][1];
    planes[BOTTOM].normal.y = m[1][3] + m[1][1];
    planes[BOTTOM].normal.z = m[2][3] + m[2][1];
    planes[BOTTOM].distance = m[3][3] + m[3][1];
    normalizePlane(planes[BOTTOM]);

    // Plano superior: m[3] - m[1]
    planes[TOP].normal.x = m[0][3] - m[0][1];
    planes[TOP].normal.y = m[1][3] - m[1][1];
    planes[TOP].normal.z = m[2][3] - m[2][1];
    planes[TOP].distance = m[3][3] - m[3][1];
    normalizePlane(planes[TOP]);

    // Plano cercano: m[3] + m[2]
    planes[NEAR_PLANE].normal.x = m[0][3] + m[0][2];
    planes[NEAR_PLANE].normal.y = m[1][3] + m[1][2];
    planes[NEAR_PLANE].normal.z = m[2][3] + m[2][2];
    planes[NEAR_PLANE].distance = m[3][3] + m[3][2];
    normalizePlane(planes[NEAR_PLANE]);

    // Plano lejano: m[3] - m[2]
    planes[FAR_PLANE].normal.x = m[0][3] - m[0][2];
    planes[FAR_PLANE].normal.y = m[1][3] - m[1][2];
    planes[FAR_PLANE].normal.z = m[2][3] - m[2][2];
    planes[FAR_PLANE].distance = m[3][3] - m[3][2];
    normalizePlane(planes[FAR_PLANE]);
}

CullResult Frustum::testBoundingBox(const BoundingBox& box) const {
    bool intersect = false;

    // Obtener los 8 vértices del bounding box
    glm::vec3 vertices[8] = {
        glm::vec3(box.min.x, box.min.y, box.min.z),
        glm::vec3(box.max.x, box.min.y, box.min.z),
        glm::vec3(box.min.x, box.max.y, box.min.z),
        glm::vec3(box.max.x, box.max.y, box.min.z),
        glm::vec3(box.min.x, box.min.y, box.max.z),
        glm::vec3(box.max.x, box.min.y, box.max.z),
        glm::vec3(box.min.x, box.max.y, box.max.z),
        glm::vec3(box.max.x, box.max.y, box.max.z)
    };

    // Probar cada plano del frustum
    for (size_t i = 0; i < PLANE_COUNT; ++i) {
        int pointsInside = 0;

        // Contar cuántos vértices están dentro del plano
        for (int j = 0; j < 8; ++j) {
            if (planes[i].distanceToPoint(vertices[j]) >= 0.0f) {
                pointsInside++;
            }
        }

        // Si todos los puntos están fuera de este plano, el box está fuera
        if (pointsInside == 0) {
            return CullResult::OUTSIDE;
        }

        // Si algunos puntos están dentro y otros fuera, hay intersección
        if (pointsInside < 8) {
            intersect = true;
        }
    }

    return intersect ? CullResult::INTERSECT : CullResult::INSIDE;
}

CullResult Frustum::testBoundingSphere(const BoundingSphere& sphere) const {
    bool intersect = false;

    for (size_t i = 0; i < PLANE_COUNT; ++i) {
        float distance = planes[i].distanceToPoint(sphere.center);

        if (distance < -sphere.radius) {
            return CullResult::OUTSIDE;
        }
        else if (distance < sphere.radius) {
            intersect = true;
        }
    }

    return intersect ? CullResult::INTERSECT : CullResult::INSIDE;
}

CullResult Frustum::testPoint(const glm::vec3& point) const {
    for (size_t i = 0; i < PLANE_COUNT; ++i) {
        if (planes[i].distanceToPoint(point) < 0.0f) {
            return CullResult::OUTSIDE;
        }
    }
    return CullResult::INSIDE;
}

CullResult Frustum::testTransformedBoundingBox(const BoundingBox& localBox,
    const glm::mat4& transform) const {
    // Transformar el bounding box al espacio mundial
    glm::vec3 vertices[8] = {
        glm::vec3(localBox.min.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.min.z),
        glm::vec3(localBox.min.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.min.y, localBox.max.z),
        glm::vec3(localBox.min.x, localBox.max.y, localBox.max.z),
        glm::vec3(localBox.max.x, localBox.max.y, localBox.max.z)
    };

    // Transformar cada vértice
    for (int i = 0; i < 8; ++i) {
        glm::vec4 transformedVertex = transform * glm::vec4(vertices[i], 1.0f);
        vertices[i] = glm::vec3(transformedVertex) / transformedVertex.w;
    }

    // Crear un nuevo bounding box con los vértices transformados
    glm::vec3 minPoint = vertices[0];
    glm::vec3 maxPoint = vertices[0];

    for (int i = 1; i < 8; ++i) {
        minPoint = glm::min(minPoint, vertices[i]);
        maxPoint = glm::max(maxPoint, vertices[i]);
    }

    BoundingBox transformedBox(minPoint, maxPoint);
    return testBoundingBox(transformedBox);
}

void Frustum::normalizePlane(Plane& plane) {
    float length = glm::length(plane.normal);
    if (length > 0.0f) {
        plane.normal /= length;
        plane.distance /= length;
    }
}