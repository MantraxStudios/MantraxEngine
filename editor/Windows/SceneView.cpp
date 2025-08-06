#include "SceneView.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"
#include "../EUI/EditorInfo.h"
#include "Selection.h"
#include <imgui/ImGuizmo.h>
#include "components/EventSystem.h"
#include <SDL.h>

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

                // Configurar ImGuizmo según el tipo de proyección de la cámara
                bool isOrthographic = camera->getProjectionType() == ProjectionType::Orthographic;
                ImGuizmo::SetOrthographic(isOrthographic);
                ImGuizmo::SetRect(imagePos.x, imagePos.y, (float)w, (float)h);

                if (Selection::GameObjectSelect) {
                    glm::mat4 view = camera->getViewMatrix();
                    glm::mat4 proj = camera->getProjectionMatrix();
                    glm::mat4 model = Selection::GameObjectSelect->getWorldModelMatrix();

                    bool used = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                        currentGizmoOperation, currentGizmoMode,
                        glm::value_ptr(model));

                    if (used) {
                        glm::vec3 translation, scale;
                        glm::quat rotation;
                        glm::vec3 skew;
                        glm::vec4 perspective;

                        if (glm::decompose(model, scale, rotation, translation, skew, perspective)) {
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
