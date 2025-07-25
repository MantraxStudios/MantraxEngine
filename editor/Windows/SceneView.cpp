#include "SceneView.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"

void SceneView::OnRenderGUI() {
	ImGui::Begin("Scene", &isOpen);

    Scene* GetScene = SceneManager::getInstance().getActiveScene();
	
	for (size_t i = 0; i < SceneManager::getInstance().getActiveScene()->getGameObjects().size(); i++)
	{
		ImGui::PushID(SceneManager::getInstance().getActiveScene()->getGameObjects()[i]->ObjectID.c_str());
        Camera* camera = GetScene->getCamera();
        if (camera) {
            Framebuffer* cameraFramebuffer = camera->getFramebuffer();
            bool framebufferEnabled = camera->isFramebufferEnabled();

            if (cameraFramebuffer && framebufferEnabled) {
                // Get the size of the viewport window content region
                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

                // Resize camera framebuffer if needed
                if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
                    int newWidth = (int)viewportPanelSize.x;
                    int newHeight = (int)viewportPanelSize.y;

                    if (newWidth != cameraFramebuffer->getWidth() || newHeight != cameraFramebuffer->getHeight()) {
                        camera->setFramebufferSize(newWidth, newHeight);
                        cameraFramebuffer = camera->getFramebuffer(); // Refresh pointer after resize
                    }

                    if (cameraFramebuffer && cameraFramebuffer->getColorTexture() > 0) {
                        // Display the camera framebuffer texture in ImGui - full size
                        ImGui::Image((void*)(intptr_t)cameraFramebuffer->getColorTexture(),
                            viewportPanelSize, // Use full available size
                            ImVec2(0, 1), ImVec2(1, 0)); // Flip Y coordinate
                        
                        // Get the image dimensions and update camera aspect ratio
                        int imageWidth = cameraFramebuffer->getWidth();
                        int imageHeight = cameraFramebuffer->getHeight();
                        
                        // Update camera aspect ratio to match the framebuffer dimensions
                        if (imageWidth > 0 && imageHeight > 0) {
                            float newAspectRatio = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
                            camera->setAspectRatio(newAspectRatio);
                        }
                        
                        // Keep the RenderConfig resizeViewport function for other uses
                        try {
                            RenderConfig& renderConfig = RenderConfig::getInstance();
                            // renderConfig.resizeViewport(imageWidth, imageHeight); // Commented out - not needed for camera projection
                        } catch (const std::exception& e) {
                            // Silent error handling - no text display
                        }
                    }
                }
            }
        }
		ImGui::PopID();
	}

	ImGui::End();
}