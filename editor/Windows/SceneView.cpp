#include "SceneView.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"
#include "../EUI/EditorInfo.h"
#include "Selection.h"
#include <imgui/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

using namespace ImGuizmo;
static MODE currentGizmoMode = LOCAL;
static OPERATION currentGizmoOperation = TRANSLATE;

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

                EditorInfo::RenderPositionX = imagePos.x;
                EditorInfo::RenderPositionY = imagePos.y;

                ImGuiIO& io = ImGui::GetIO();
                bool hovered = ImGui::IsItemHovered();
                bool focused = ImGui::IsWindowFocused();

                int w = fb->getWidth(), h = fb->getHeight();
                camera->setAspectRatio((float)w / h);

                if (Selection::GameObjectSelect && hovered) {
                    if (ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver()) {
                    }
                }

                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetRect(imagePos.x, imagePos.y, (float)w, (float)h);

                if (Selection::GameObjectSelect) {
                    GameObject* obj = Selection::GameObjectSelect;
                    glm::mat4 model = obj->getWorldModelMatrix();
                    glm::mat4 view = camera->getViewMatrix();
                    glm::mat4 proj = camera->getProjectionMatrix();

                    bool used = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                        currentGizmoOperation, currentGizmoMode,
                        glm::value_ptr(model));

                    if (used) {
                        glm::vec3 translation, scale;
                        glm::quat rotation;
                        glm::vec3 skew;
                        glm::vec4 perspective;
                        
                        if (glm::decompose(model, scale, rotation, translation, skew, perspective)) {
                            if (obj->hasParent()) {
                                glm::mat4 parentWorldMatrix = obj->getParent()->getWorldModelMatrix();
                                glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
                                glm::mat4 localMatrix = parentWorldInverse * model;
                                
                                glm::vec3 localTranslation, localScale;
                                glm::quat localRotation;
                                glm::vec3 localSkew;
                                glm::vec4 localPerspective;
                                
                                if (glm::decompose(localMatrix, localScale, localRotation, localTranslation, localSkew, localPerspective)) {
                                    obj->setLocalPosition(localTranslation);
                                    obj->setLocalRotationQuat(localRotation);
                                    obj->setLocalScale(localScale);
                                }
                            } else {
                                obj->setWorldPosition(translation);
                                obj->setWorldRotationQuat(rotation);
                                obj->setWorldScale(scale);
                            }
                        } else {
                            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model),
                                &translation.x, &rotation.x, &scale.x);
                            
                            if (obj->hasParent()) {
                                glm::mat4 parentWorldMatrix = obj->getParent()->getWorldModelMatrix();
                                glm::mat4 parentWorldInverse = glm::inverse(parentWorldMatrix);
                                glm::mat4 localMatrix = parentWorldInverse * model;
                                
                                glm::vec3 localTranslation, localScale;
                                glm::quat localRotation;
                                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localMatrix),
                                    &localTranslation.x, &localRotation.x, &localScale.x);
                                
                                obj->setLocalPosition(localTranslation);
                                obj->setLocalRotationQuat(localRotation);
                                obj->setLocalScale(localScale);
                            } else {
                                obj->setWorldPosition(translation);
                                obj->setWorldRotationQuat(rotation);
                                obj->setWorldScale(scale);
                            }
                        }
                    }

                    if (!ImGui::IsMouseDown(1)) {
                        if (ImGui::IsKeyPressed(ImGuiKey_T)) currentGizmoOperation = TRANSLATE;
                        if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoOperation = ROTATE;
                        if (ImGui::IsKeyPressed(ImGuiKey_S)) currentGizmoOperation = SCALE;
                        if (ImGui::IsKeyPressed(ImGuiKey_L))
                            currentGizmoMode = (currentGizmoMode == LOCAL) ? WORLD : LOCAL;
                    }
                }
            }
        }
    }

    ImGui::End();
}
