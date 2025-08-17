#include "EventSystem.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <SDL.h>
#include "SceneManager.h"
#include "../render/RenderConfig.h"
#include "../render/AssimpGeometry.h"

glm::vec2 EventSystem::ViewportRenderPosition = glm::vec2(0.0f, 0.0f);

glm::vec2 EventSystem::screen_to_viewport(Camera *cam)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Convertir a enteros si es necesario
    int intMouseX = (int)mouseX;
    int intMouseY = (int)mouseY;

    float viewportX = static_cast<float>(mouseX) - ViewportRenderPosition.x;
    float viewportY = static_cast<float>(mouseY) - ViewportRenderPosition.y;

    // Obtener dimensiones del viewport desde RenderConfig
    float viewportWidth = static_cast<float>(SceneManager::getInstance().getActiveScene()->getCamera()->getFramebuffer()->getWidth());
    float viewportHeight = static_cast<float>(SceneManager::getInstance().getActiveScene()->getCamera()->getFramebuffer()->getHeight());

    if (viewportX < 0 || viewportX > viewportWidth ||
        viewportY < 0 || viewportY > viewportHeight)
    {
        return glm::vec2(0.0f, 0.0f);
    }

    float normalizedX = viewportX / viewportWidth;
    float normalizedY = viewportY / viewportHeight;

    float ndcX = (normalizedX * 2.0f) - 1.0f;
    float ndcY = 1.0f - (normalizedY * 2.0f);

    return glm::vec2(ndcX, ndcY);
}

glm::vec2 EventSystem::get_mouse_position_in_viewport(glm::vec2 WindowSize, glm::vec2 ScreenSize)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Convertir a enteros si es necesario
    int intMouseX = (int)mouseX;
    int intMouseY = (int)mouseY;

    // Obtener la posición del mouse relativa al viewport
    double windowMousePosX = mouseX - ViewportRenderPosition.x;
    double windowMousePosY = mouseY - ViewportRenderPosition.y;

    // Dimensiones del viewport
    float viewportWidth = static_cast<float>(SceneManager::getInstance().getActiveScene()->getCamera()->getFramebuffer()->getWidth());
    float viewportHeight = static_cast<float>(SceneManager::getInstance().getActiveScene()->getCamera()->getFramebuffer()->getHeight());

    // Verificar que el mouse esté dentro del viewport
    if (windowMousePosX < 0 || windowMousePosX > viewportWidth ||
        windowMousePosY < 0 || windowMousePosY > viewportHeight) {
        return glm::vec2(0, 0);
    }

    // Normalizar coordenadas del mouse a rango [0, 1]
    double normalizedX = windowMousePosX / viewportWidth;
    double normalizedY = windowMousePosY / viewportHeight;

    // Convertir a NDC (Normalized Device Coordinates) [-1, 1]
    double ndcX = normalizedX * 2.0 - 1.0;
    double ndcY = 1.0 - normalizedY * 2.0; // Invertir Y para que +Y sea hacia arriba

    // Obtener datos de la cámara
    auto camera = SceneManager::getInstance().getActiveScene()->getCamera();
    glm::vec3 cameraPos = camera->getPosition();

    if (camera->getProjectionType() == ProjectionType::Orthographic) {
        // Para proyección ortográfica
        float orthoSize = camera->getOrthographicSize();
        float aspect = camera->getAspectRatio();

        // Calcular dimensiones del mundo visible
        float worldHeight = orthoSize * 2.0f;
        float worldWidth = worldHeight * aspect;

        // Convertir NDC a coordenadas de mundo
        double worldX = (ndcX * worldWidth * 0.5) + cameraPos.x;
        double worldY = (ndcY * worldHeight * 0.5) + cameraPos.y;

        return glm::vec2(worldX, worldY);
    }
    else { // ProjectionType::Perspective
        // Para proyección perspectiva - proyectar al plano Z de la cámara
        float fov = camera->getFOV();
        float aspect = camera->getAspectRatio();

        // Distancia de proyección (puedes ajustar esto según tu necesidad)
        // Por ejemplo, si quieres proyectar al plano z=0:
        float projectionDistance = abs(cameraPos.z);

        // Si la cámara está en z=0, usar una distancia por defecto
        if (projectionDistance < 0.01f) {
            projectionDistance = 10.0f; // Distancia por defecto
        }

        // Convertir FOV de grados a radianes
        float fovRadians = glm::radians(fov);

        // Calcular el tamaño del plano de proyección
        float halfHeight = tan(fovRadians * 0.5f) * projectionDistance;
        float halfWidth = halfHeight * aspect;

        // Convertir NDC a coordenadas de mundo
        double worldX = (ndcX * halfWidth) + cameraPos.x;
        double worldY = (ndcY * halfHeight) + cameraPos.y;

        return glm::vec2(worldX, worldY);
    }
}

glm::vec2 EventSystem::mouse_to_view_port_position(glm::vec2 WindowSize)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Convertir a enteros si es necesario
    int intMouseX = (int)mouseX;
    int intMouseY = (int)mouseY;

    Camera *cam = SceneManager::getInstance().getActiveScene()->getCamera();

    double windowMousePosX = mouseX - ViewportRenderPosition.x;
    double windowMousePosY = mouseY - ViewportRenderPosition.y;

    double NormalMousePosX = windowMousePosX;
    double NormalMousePosY = windowMousePosY;

    // Usar RenderConfig para obtener dimensiones
    float renderWidth = static_cast<float>(RenderConfig::getInstance().getWidth());
    float renderHeight = static_cast<float>(RenderConfig::getInstance().getHeight());

    double centeredMousePosX = (NormalMousePosX * 2.0f - 1.0f) * renderWidth * cam->getFOV();
    double centeredMousePosY = (NormalMousePosY * 2.0f + 1.0f) * renderHeight * cam->getFOV();

    double WorldPointX = centeredMousePosX + cam->getPosition().x;
    double WorldPointY = centeredMousePosY + cam->getPosition().y;

    return glm::vec2(NormalMousePosX, NormalMousePosY);
}

glm::vec2 EventSystem::mouse_to_screen_pos(glm::vec2 WindowSize)
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Convertir a enteros si es necesario
    int intMouseX = (int)mouseX;
    int intMouseY = (int)mouseY;

    Camera* cam = SceneManager::getInstance().getActiveScene()->getCamera();

    double windowMousePosX = mouseX - ViewportRenderPosition.x;
    double windowMousePosY = mouseY - ViewportRenderPosition.y;

    // Usar RenderConfig para obtener dimensiones
    float renderWidth = static_cast<float>(RenderConfig::getInstance().getWidth());
    float renderHeight = static_cast<float>(RenderConfig::getInstance().getHeight());

    double NormalMousePosX = (windowMousePosX / renderWidth);
    double NormalMousePosY = (-windowMousePosY / renderHeight);

    // CALCULAR CENTRO DE LA TEXTURA CON EL MOUSE (0, 0)
    double centeredMousePosX = ((NormalMousePosX * 2.0f - 1.0f) * (renderWidth / 2) * cam->getFOV());
    double centeredMousePosY = ((NormalMousePosY * 2.0f + 1.0f) * (renderHeight / 2) * cam->getFOV());

    double WorldPointX = centeredMousePosX;
    double WorldPointY = centeredMousePosY;

    return glm::vec2(WorldPointX, WorldPointY);
}


bool EventSystem::MouseCast2D(glm::vec2 mouseCoords, CastData* data, Camera* camera)
{
    if (std::isnan(mouseCoords.x) || std::isnan(mouseCoords.y) ||
        std::isinf(mouseCoords.x) || std::isinf(mouseCoords.y))
    {
        return false;
    }

    const float MIN_PICK_DISTANCE = 0.1f;
    const float MAX_PICK_DISTANCE = 1000.0f;
    const float EPSILON = 0.0001f;

    float closestDistance = MAX_PICK_DISTANCE;
    GameObject* closestObject = nullptr;

    glm::mat4 viewMatrix = camera->getViewMatrix();
    glm::mat4 projectionMatrix = camera->getProjectionMatrix();
    glm::vec3 rayOrigin, rayDirection;

    ScreenToWorldRay(mouseCoords, glm::inverse(viewMatrix), glm::inverse(projectionMatrix),
        rayOrigin, rayDirection, camera);
    rayDirection = glm::normalize(rayDirection);

    SceneManager* sceneM = &SceneManager::getInstance();

    for (GameObject* objD : sceneM->getActiveScene()->getGameObjects())
    {
        if (!objD || !objD->hasGeometry()) {
            continue;
        }

        // Obtener la matriz del modelo actualizada (importante para objetos que cambian de tamaño)
        glm::mat4 modelMatrix = objD->getWorldModelMatrix();
        glm::mat4 inverseModel = glm::inverse(modelMatrix);

        // Transformar el rayo al espacio local del objeto
        glm::vec3 localRayOrigin = glm::vec3(inverseModel * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 localRayDirection = glm::normalize(glm::vec3(inverseModel * glm::vec4(rayDirection, 0.0f)));

        AssimpGeometry* geometry = objD->getGeometry();

        // Obtener bounding box en espacio local
        glm::vec3 boxMin = geometry->getBoundingBoxMin();
        glm::vec3 boxMax = geometry->getBoundingBoxMax();

        // Verificar intersección con AABB
        float t_near, t_far;
        if (RayIntersectsAABB_Extended(localRayOrigin, localRayDirection, boxMin, boxMax, t_near, t_far))
        {
            // Usar la distancia real de intersección, no la distancia al centro
            float intersectionDistance = t_near > 0 ? t_near : t_far;

            // Transformar la distancia de vuelta al espacio mundial
            glm::vec3 localIntersectionPoint = localRayOrigin + localRayDirection * intersectionDistance;
            glm::vec3 worldIntersectionPoint = glm::vec3(modelMatrix * glm::vec4(localIntersectionPoint, 1.0f));
            float worldDistance = glm::length(worldIntersectionPoint - rayOrigin);

            if (worldDistance >= MIN_PICK_DISTANCE && worldDistance <= MAX_PICK_DISTANCE &&
                worldDistance < closestDistance - EPSILON)
            {
                closestDistance = worldDistance;
                closestObject = objD;
            }
        }
    }

    if (closestObject != nullptr)
    {
        data->object = closestObject;
        data->distance = closestDistance; // Opcional: guardar la distancia
        return true;
    }

    return false;
}

bool EventSystem::MouseCast2D_Precise(glm::vec2 mouseCoords, CastData* data, Camera* camera)
{
    if (std::isnan(mouseCoords.x) || std::isnan(mouseCoords.y) ||
        std::isinf(mouseCoords.x) || std::isinf(mouseCoords.y))
    {
        return false;
    }

    const float MIN_PICK_DISTANCE = 0.1f;
    const float MAX_PICK_DISTANCE = 1000.0f;
    const float EPSILON = 0.0001f;

    float closestDistance = MAX_PICK_DISTANCE;
    GameObject* closestObject = nullptr;

    glm::mat4 viewMatrix = camera->getViewMatrix();
    glm::mat4 projectionMatrix = camera->getProjectionMatrix();
    glm::vec3 rayOrigin, rayDirection;

    ScreenToWorldRay(mouseCoords, glm::inverse(viewMatrix), glm::inverse(projectionMatrix),
        rayOrigin, rayDirection, camera);
    rayDirection = glm::normalize(rayDirection);

    SceneManager* sceneM = &SceneManager::getInstance();

    for (GameObject* objD : sceneM->getActiveScene()->getGameObjects())
    {
        if (!objD || !objD->hasGeometry()) {
            continue;
        }

        glm::mat4 modelMatrix = objD->getWorldModelMatrix();
        glm::mat4 inverseModel = glm::inverse(modelMatrix);

        glm::vec3 localRayOrigin = glm::vec3(inverseModel * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 localRayDirection = glm::normalize(glm::vec3(inverseModel * glm::vec4(rayDirection, 0.0f)));

        AssimpGeometry* geometry = objD->getGeometry();

        // Primero verificar AABB para optimización
        glm::vec3 boxMin = geometry->getBoundingBoxMin();
        glm::vec3 boxMax = geometry->getBoundingBoxMax();

        float t_near, t_far;
        if (!RayIntersectsAABB_Extended(localRayOrigin, localRayDirection, boxMin, boxMax, t_near, t_far))
        {
            continue; // No intersecta el bounding box
        }

        float minTriangleDistance = MAX_PICK_DISTANCE;
        bool hitTriangle = false;

        // Si es AssimpGeometry y tiene datos de vértices disponibles
        AssimpGeometry* assimpGeo = dynamic_cast<AssimpGeometry*>(geometry);
        if (assimpGeo && assimpGeo->isLoaded())
        {
            // Aquí necesitarías acceso a los vértices e índices de la geometría
            // Esto depende de cómo esté implementada tu clase AssimpGeometry

            // Ejemplo conceptual (necesitarías adaptarlo a tu implementación):
            /*
            const std::vector<Vertex>& vertices = assimpGeo->getVertices();
            const std::vector<unsigned int>& indices = assimpGeo->getIndices();

            for (size_t i = 0; i < indices.size(); i += 3)
            {
                glm::vec3 v0 = vertices[indices[i]].position;
                glm::vec3 v1 = vertices[indices[i+1]].position;
                glm::vec3 v2 = vertices[indices[i+2]].position;

                float t;
                if (RayIntersectsTriangle(localRayOrigin, localRayDirection, v0, v1, v2, t))
                {
                    if (t > 0 && t < minTriangleDistance)
                    {
                        minTriangleDistance = t;
                        hitTriangle = true;
                    }
                }
            }
            */
        }

        // Si no encontramos intersección con triángulos o no están disponibles,
        // usar la intersección con AABB
        float intersectionDistance = hitTriangle ? minTriangleDistance :
            (t_near > 0 ? t_near : t_far);

        if (intersectionDistance > 0)
        {
            glm::vec3 localIntersectionPoint = localRayOrigin + localRayDirection * intersectionDistance;
            glm::vec3 worldIntersectionPoint = glm::vec3(modelMatrix * glm::vec4(localIntersectionPoint, 1.0f));
            float worldDistance = glm::length(worldIntersectionPoint - rayOrigin);

            if (worldDistance >= MIN_PICK_DISTANCE && worldDistance <= MAX_PICK_DISTANCE &&
                worldDistance < closestDistance - EPSILON)
            {
                closestDistance = worldDistance;
                closestObject = objD;
            }
        }
    }

    if (closestObject != nullptr)
    {
        data->object = closestObject;
        data->distance = closestDistance;
        return true;
    }

    return false;
}

bool EventSystem::MouseCast3D(const glm::vec2 &screenCoords, CastData *data)
{

    return false;
}


glm::vec2 EventSystem::RotatePoint(const glm::vec2 &point, const glm::vec2 &center, float angle)
{
    glm::vec2 rotatedPoint;
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);

    rotatedPoint.x = center.x + (point.x - center.x) * cosTheta - (point.y - center.y) * sinTheta;
    rotatedPoint.y = center.y + (point.x - center.x) * sinTheta + (point.y - center.y) * cosTheta;

    return rotatedPoint;
}