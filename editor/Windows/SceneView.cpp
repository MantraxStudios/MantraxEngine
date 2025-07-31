#include "SceneView.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"
#include "../EUI/EditorInfo.h"
#include "Selection.h"
#include <imgui/ImGuizmo.h>
#include "components/EventSystem.h"
#include <SDL.h> // Added for SDL_SetRelativeMouseMode

using namespace ImGuizmo;
static MODE currentGizmoMode = LOCAL;
static OPERATION currentGizmoOperation = TRANSLATE;

bool SceneView::mouseCaptured = false;

void SceneView::OnRenderGUI() {
    ImGui::Begin("Scene", &isOpen);
    Scene* scene = SceneManager::getInstance().getActiveScene();
    Camera* camera = scene ? scene->getCamera() : nullptr;

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist();

    if (camera && camera->isFramebufferEnabled()) {
        Framebuffer* fb = camera->getFramebuffer();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x > 0 && avail.y > 0) {
            camera->setFramebufferSize((int)avail.x, (int)avail.y);
            fb = camera->getFramebuffer();

            if (fb && fb->getColorTexture() > 0) {
                ImVec2 imagePos = ImGui::GetCursorScreenPos();
                ImGui::Image((void*)(intptr_t)fb->getColorTexture(),
                    ImVec2(fb->getWidth(), fb->getHeight()),
                    ImVec2(0, 1), ImVec2(1, 0));

                EditorInfo::IsHoveringScene = ImGui::IsItemHovered();
                EditorInfo::RenderPositionX = imagePos.x;
                EditorInfo::RenderPositionY = imagePos.y;

                if (ImGui::IsWindowHovered()) {
                    if (ImGui::IsMouseClicked(0)) {
                        CastData* data = new CastData();
                        EventSystem::ViewportRenderPosition = glm::vec2(imagePos.x, imagePos.y);
                        WorldPoint = EventSystem::screen_to_viewport(camera);

                        if (EventSystem::MouseCast2D_Precise(WorldPoint, data, camera) && !ImGuizmo::IsOver()) {
                            if (Selection::GameObjectSelect != data->object) {
                                Selection::GameObjectSelect = data->object;
                                Selection::LightSelect = nullptr; // Deseleccionar luz si hay una seleccionada
                            }
                        }
                        else {
                            if (!ImGuizmo::IsOver()) {
                                Selection::GameObjectSelect = nullptr;
                            }
                        }
                    }
                }

                ImGuiIO& io = ImGui::GetIO();
                bool hovered = ImGui::IsItemHovered();
                bool focused = ImGui::IsWindowFocused();

                int w = fb->getWidth(), h = fb->getHeight();
                camera->setAspectRatio((float)w / h);

                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetRect(imagePos.x, imagePos.y, (float)w, (float)h);

                // Manejar manipulación de objetos o luces
                if (Selection::GameObjectSelect || Selection::LightSelect) {
                    glm::mat4 view = camera->getViewMatrix();
                    glm::mat4 proj = camera->getProjectionMatrix();
                    glm::mat4 model;

                    // Preparar matriz de modelo según si es un objeto o una luz
                    if (Selection::GameObjectSelect) {
                        model = Selection::GameObjectSelect->getWorldModelMatrix();
                    }
                    else if (Selection::LightSelect) {
                        // Para luces, crear matriz de modelo basada en su posición
                        glm::vec3 position;
                        glm::vec3 scale(0.5f); // Tamaño del gizmo para luces

                        if (Selection::LightSelect->getType() == LightType::Directional) {
                            // Para luces direccionales, usar una posición fija y mostrar solo rotación
                            position = camera->getPosition() + camera->getForward() * 5.0f;
                            currentGizmoOperation = ROTATE; // Forzar modo rotación para luces direccionales
                        }
                        else {
                            position = Selection::LightSelect->getPosition();
                            // Permitir tanto traslación como rotación para otros tipos de luces
                            if (currentGizmoOperation == SCALE) {
                                currentGizmoOperation = TRANSLATE; // No permitir escalado para luces
                            }
                        }

                        model = glm::translate(glm::mat4(1.0f), position);
                        
                        // Aplicar rotación basada en la dirección de la luz
                        if (Selection::LightSelect->getType() != LightType::Point) {
                            glm::vec3 direction = Selection::LightSelect->getDirection();
                            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                            glm::vec3 right = glm::normalize(glm::cross(direction, up));
                            up = glm::normalize(glm::cross(right, direction));
                            
                            glm::mat3 rotationMatrix(right, up, -direction);
                            model = model * glm::mat4(rotationMatrix);
                        }

                        model = glm::scale(model, scale);
                    }

                    bool used = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                        currentGizmoOperation, currentGizmoMode,
                        glm::value_ptr(model));

                    if (used) {
                        glm::vec3 translation, scale;
                        glm::quat rotation;
                        glm::vec3 skew;
                        glm::vec4 perspective;

                        if (glm::decompose(model, scale, rotation, translation, skew, perspective)) {
                            if (Selection::GameObjectSelect) {
                                // Actualizar transformación del objeto
                                if (Selection::GameObjectSelect->hasParent()) {
                                    glm::mat4 parentWorldMatrix = Selection::GameObjectSelect->getParent()->getWorldModelMatrix();
                                    glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
                                    glm::mat4 localMatrix = parentWorldInverse * model;
                                    
                                    glm::vec3 localTranslation, localScale;
                                    glm::quat localRotation;
                                    glm::vec3 localSkew;
                                    glm::vec4 localPerspective;
                                    
                                    if (glm::decompose(localMatrix, localScale, localRotation, localTranslation, localSkew, localPerspective)) {
                                        Selection::GameObjectSelect->setLocalPosition(localTranslation);
                                        Selection::GameObjectSelect->setLocalRotationQuat(localRotation);
                                        Selection::GameObjectSelect->setLocalScale(localScale);
                                    }
                                } else {
                                    Selection::GameObjectSelect->setWorldPosition(translation);
                                    Selection::GameObjectSelect->setWorldRotationQuat(rotation);
                                    Selection::GameObjectSelect->setWorldScale(scale);
                                }
                            }
                            else if (Selection::LightSelect) {
                                // Actualizar transformación de la luz
                                if (Selection::LightSelect->getType() == LightType::Directional) {
                                    // Para luces direccionales, solo actualizar dirección
                                    glm::vec3 forward = -glm::normalize(glm::mat3(model) * glm::vec3(0.0f, 0.0f, 1.0f));
                                    Selection::LightSelect->setDirection(forward);
                                }
                                else {
                                    // Para otros tipos de luces, actualizar posición y dirección
                                    Selection::LightSelect->setPosition(translation / scale.x); // Deshacer el escalado aplicado al gizmo
                                    
                                    if (Selection::LightSelect->getType() == LightType::Spot) {
                                        glm::vec3 forward = -glm::normalize(glm::mat3(model) * glm::vec3(0.0f, 0.0f, 1.0f));
                                        Selection::LightSelect->setDirection(forward);
                                    }
                                }
                            }
                        }
                    }

                    if (!ImGui::IsMouseDown(1)) {
                        if (ImGui::IsKeyPressed(ImGuiKey_T)) currentGizmoOperation = TRANSLATE;
                        if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoOperation = ROTATE;
                        if (ImGui::IsKeyPressed(ImGuiKey_S) && Selection::GameObjectSelect) currentGizmoOperation = SCALE;
                        if (ImGui::IsKeyPressed(ImGuiKey_L))
                            currentGizmoMode = (currentGizmoMode == LOCAL) ? WORLD : LOCAL;
                    }
                }
            }
        }
    }

    ImGui::End();
}
