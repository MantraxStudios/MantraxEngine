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
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

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
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    double windowMousePosX = mouseX - ViewportRenderPosition.x;
    double windowMousePosY = mouseY - ViewportRenderPosition.y;

    double NormalMousePosX = windowMousePosX / WindowSize.x;
    double NormalMousePosY = -windowMousePosY / WindowSize.y;

    return glm::vec2(NormalMousePosX, NormalMousePosY);
}

glm::vec2 EventSystem::mouse_to_view_port_position(glm::vec2 WindowSize)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
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
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
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


bool EventSystem::MouseCast2D(glm::vec2 mouseCoords, CastData *data, Camera *camera)
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
    GameObject *closestObject = nullptr;

    glm::mat4 viewMatrix = camera->getViewMatrix();
    glm::mat4 projectionMatrix = camera->getProjectionMatrix();

    glm::vec3 rayOrigin, rayDirection;
    ScreenToWorldRay(mouseCoords, glm::inverse(viewMatrix), glm::inverse(projectionMatrix), rayOrigin, rayDirection, camera);

    rayDirection = glm::normalize(rayDirection);

    SceneManager *sceneM = &SceneManager::getInstance();

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
        
        // Usar bounding box para detección rápida primero
        glm::vec3 boxMin = geometry->getBoundingBoxMin();
        glm::vec3 boxMax = geometry->getBoundingBoxMax();
        
        if (RayIntersectsAABB(localRayOrigin, localRayDirection, boxMin, boxMax))
        {
            // Si es AssimpGeometry, hacer raycast con triángulos
            AssimpGeometry* assimpGeo = dynamic_cast<AssimpGeometry*>(geometry);
            if (assimpGeo && assimpGeo->isLoaded())
            {
                // Para AssimpGeometry, usar bounding box como aproximación
                // ya que no tenemos acceso directo a los vértices desde aquí
                glm::vec3 boxCenter = (boxMin + boxMax) * 0.5f;
                float distance = glm::length(boxCenter - localRayOrigin);
                
                if (distance < closestDistance - EPSILON)
                {
                    closestDistance = distance;
                    closestObject = objD;
                }
            }
            else
            {
                		// Para geometría simple, usar el centro del bounding box
                glm::vec3 boxCenter = (boxMin + boxMax) * 0.5f;
                float distance = glm::length(boxCenter - localRayOrigin);
                
                if (distance < closestDistance - EPSILON)
                {
                    closestDistance = distance;
                    closestObject = objD;
                }
            }
        }
    }

    if (closestObject != nullptr && closestDistance >= MIN_PICK_DISTANCE && closestDistance <= MAX_PICK_DISTANCE)
    {
        data->object = closestObject;
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