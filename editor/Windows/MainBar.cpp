#include "MainBar.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/RenderConfig.h"
#include "render/RenderPipeline.h"
#include "render/DefaultShaders.h"
#include "render/Camera.h"
#include <core/FileSystem.h>
#include "../EUI/EditorInfo.h"
#include "Selection.h"
#include "RenderWindows.h"
#include "../SceneSaver.h"
#include <iostream>
#include "../Windows/FileExplorer.h"
#include <mpak/MantraxCorePackBuilder.h>

// Declaración de la variable externa
extern volatile bool g_running;

void MainBar::OnRenderGUI() {
	static bool boolOpenedPopup = false;

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Empty Scene")) {
			SceneSaver::MakeNewScene("New Scene");
			std::cout << "Created new empty scene with RenderPipeline" << std::endl;
		}

		if (ImGui::MenuItem("Save")) {

			if (!FileSystem::fileExists(EditorInfo::currentScenePath)) {
				boolOpenedPopup = true;
			}
			else {
				auto& sceneManager = SceneManager::getInstance();
				Scene* activeScene = sceneManager.getActiveScene();
				if (activeScene) {
					std::string filename = EditorInfo::currentScenePath;
					if (SceneSaver::SaveScene(activeScene, filename)) {
						std::cout << "Scene saved successfully!" << std::endl;
					}
					else {
						std::cerr << "Failed to save scene!" << std::endl;
					}
				}
			}
		}

		if (ImGui::MenuItem("Save As...")) {
			boolOpenedPopup = true;
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Exit")) {
			g_running = false;
		}
		ImGui::EndMenu();
	}

	if (boolOpenedPopup) {
		ImGui::OpenPopup("Save File");
		boolOpenedPopup = false;
	}

	std::string selectedPath;
	if (FileExplorer::ShowSavePopup(FileSystem::getProjectPath() + "\\Content\\", selectedPath, ".scene")) {
		auto& sceneManager = SceneManager::getInstance();
		Scene* activeScene = sceneManager.getActiveScene();
		if (activeScene) {
			EditorInfo::currentScenePath = FileSystem::getProjectPath() + "\\Content\\" + selectedPath;
			std::string filename = EditorInfo::currentScenePath;
			if (SceneSaver::SaveScene(activeScene, filename)) {
				activeScene->setName(FileSystem::getFileNameWithoutExtension(filename));
				std::cout << "Scene saved successfully! " << EditorInfo::currentScenePath << std::endl;
				std::cout << "Scene Current Path! " << EditorInfo::currentScenePath << std::endl;
			}
			else {
				std::cerr << "Failed to save scene!" << std::endl;
			}
		}
	}

	if (ImGui::BeginMenu("GameObject")) {
		if (ImGui::MenuItem("New Object"))
		{
			auto& sceneManager = SceneManager::getInstance();
			Scene* activeScene = sceneManager.getActiveScene();
			RenderPipeline* pipeline = activeScene->getRenderPipeline();
			
			if (pipeline) {
				// Crear el objeto
				GameObject* NewObject = new GameObject();
				NewObject->Name = "New Object";
				NewObject->setLocalPosition({ 0.0f, 0.0f, 0.0f });
				NewObject->setMaterial(pipeline->getMaterial("red_material"));
				
				// Agregar a la escena (sincronización automática con RenderPipeline)
				activeScene->addGameObject(NewObject);
				
				std::cout << "Created new object: " << NewObject->Name << std::endl;
			} else {
				std::cerr << "Error: No RenderPipeline available" << std::endl;
			}
		}
		
		if (ImGui::MenuItem("New Empty Object"))
		{
			auto& sceneManager = SceneManager::getInstance();
			Scene* activeScene = sceneManager.getActiveScene();
			
			// Crear objeto vacío
			GameObject* NewObject = new GameObject();
			NewObject->Name = "Empty Object";
			NewObject->setLocalPosition({ 2.0f, 0.0f, 0.0f });
			
			// Agregar a la escena
			activeScene->addGameObject(NewObject);
			
			std::cout << "Created empty object: " << NewObject->Name << std::endl;
		}
		
		if (ImGui::MenuItem("New Object from Model"))
		{
			auto& sceneManager = SceneManager::getInstance();
			Scene* activeScene = sceneManager.getActiveScene();
			RenderPipeline* pipeline = activeScene->getRenderPipeline();
			
			if (pipeline) {
				// Crear objeto con carga automática de modelo
				GameObject* NewObject = new GameObject("models/cube.obj");
				NewObject->Name = "Model Object";
				NewObject->setLocalPosition({ -2.0f, 0.0f, 0.0f });
				NewObject->setMaterial(pipeline->getMaterial("blue_material"));
				
				// Solo agregar si se cargó correctamente
				if (NewObject->hasGeometry()) {
					activeScene->addGameObject(NewObject);
					std::cout << "Created model object: " << NewObject->Name << std::endl;
				} else {
					std::cout << "Warning: Model failed to load, creating fallback object" << std::endl;
					delete NewObject;
					
					// Crear objeto con geometría por defecto
					NewObject = new GameObject();
					NewObject->Name = "Fallback Object";
					NewObject->setLocalPosition({ -2.0f, 0.0f, 0.0f });
					NewObject->setMaterial(pipeline->getMaterial("green_material"));
					activeScene->addGameObject(NewObject);
				}
			}
		}
		ImGui::EndMenu();
	}
	
	if (ImGui::BeginMenu("Scene")) {
		if (ImGui::MenuItem("Clear All Objects"))
		{
			auto& sceneManager = SceneManager::getInstance();
			Scene* activeScene = sceneManager.getActiveScene();
			RenderPipeline* pipeline = activeScene->getRenderPipeline();
			
			if (pipeline) {
				// Limpiar objetos del RenderPipeline
				pipeline->clearGameObjects();
				
				// Limpiar objetos de la escena
				activeScene->cleanup();
				
				// Reconfigurar la escena
				activeScene->initialize();
				activeScene->setInitialized(true);
				
				std::cout << "Cleared all objects from scene" << std::endl;
			}
		}
		
		if (ImGui::MenuItem("Show Object Count"))
		{
			auto& sceneManager = SceneManager::getInstance();
			Scene* activeScene = sceneManager.getActiveScene();
			RenderPipeline* pipeline = activeScene->getRenderPipeline();
			
			if (pipeline) {
				std::cout << "Scene Objects: " << activeScene->getGameObjects().size() << std::endl;
				std::cout << "Pipeline Objects: " << pipeline->getTotalObjectsCount() << std::endl;
				std::cout << "Visible Objects: " << pipeline->getVisibleObjectsCount() << std::endl;
			}
		}
		ImGui::EndMenu();
	}
	
	if (ImGui::BeginMenu("Rendering")) {
		auto& sceneManager = SceneManager::getInstance();
		Scene* activeScene = sceneManager.getActiveScene();
		RenderPipeline* pipeline = activeScene->getRenderPipeline();
		
		if (pipeline) {
			bool usePBR = pipeline->getUsePBR();
			if (ImGui::MenuItem("Toggle PBR/Phong", nullptr, &usePBR)) {
				pipeline->setUsePBR(usePBR);
				std::cout << "Switched to " << (usePBR ? "PBR" : "Blinn-Phong") << " lighting" << std::endl;
			}

			if (ImGui::MenuItem("Toggle Ortho/Pov Camera", nullptr, &usePBR)) {
				if (sceneManager.getActiveScene()->getCamera()->getProjectionType() == ProjectionType::Perspective)
				{
					sceneManager.getActiveScene()->getCamera()->setProjectionType(ProjectionType::Orthographic, true);

				}
				else 
				{
					sceneManager.getActiveScene()->getCamera()->setProjectionType(ProjectionType::Perspective, true);
				}
			}

			static float fovCamera = sceneManager.getActiveScene()->getCamera()->getOrthographicSize();
			if (ImGui::SliderFloat("Camera Fov", &fovCamera, 0.0f, 200.0f, "%.2f")) {
				sceneManager.getActiveScene()->getCamera()->setOrthographicSize(fovCamera);
			}
			
			bool lowAmbient = pipeline->getLowAmbient();
			if (ImGui::MenuItem("Toggle Low Ambient", nullptr, &lowAmbient)) {
				pipeline->setLowAmbient(lowAmbient);
				std::cout << (lowAmbient ? "Enabled" : "Disabled") << " low ambient lighting" << std::endl;
			}
			
			ImGui::Separator();
			
			static float ambientIntensity = pipeline->getAmbientIntensity();
			if (ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 2.0f, "%.2f")) {
				pipeline->setAmbientIntensity(ambientIntensity);
			}
			
			bool frustumCulling = pipeline->getFrustumCulling();
			if (ImGui::MenuItem("Toggle Frustum Culling", nullptr, &frustumCulling)) {
				pipeline->setFrustumCulling(frustumCulling);
				std::cout << (frustumCulling ? "Enabled" : "Disabled") << " frustum culling" << std::endl;
			}
		}
		
		ImGui::Separator();
		
		// Antialiasing controls
		RenderConfig& config = RenderConfig::getInstance();
		static int currentAntialiasing = config.getAntialiasing();
		
		ImGui::Text("Antialiasing Settings:");
		
		if (ImGui::RadioButton("Disabled (0x)", currentAntialiasing == 0)) {
			currentAntialiasing = 0;
			config.setAntialiasing(0);
			std::cout << "Antialiasing disabled" << std::endl;
		}
		
		if (ImGui::RadioButton("2x MSAA", currentAntialiasing == 2)) {
			currentAntialiasing = 2;
			config.setAntialiasing(2);
			std::cout << "Antialiasing set to 2x MSAA" << std::endl;
		}
		
		if (ImGui::RadioButton("4x MSAA", currentAntialiasing == 4)) {
			currentAntialiasing = 4;
			config.setAntialiasing(4);
			std::cout << "Antialiasing set to 4x MSAA" << std::endl;
		}
		
		if (ImGui::RadioButton("8x MSAA", currentAntialiasing == 8)) {
			currentAntialiasing = 8;
			config.setAntialiasing(8);
			std::cout << "Antialiasing set to 8x MSAA" << std::endl;
		}
		
		if (ImGui::RadioButton("16x MSAA", currentAntialiasing == 16)) {
			currentAntialiasing = 16;
			config.setAntialiasing(16);
			std::cout << "Antialiasing set to 16x MSAA" << std::endl;
		}
		
		ImGui::Separator();
		
		// Current antialiasing status
		ImGui::Text("Current: %dx MSAA", config.getAntialiasing());
		if (config.isAntialiasingEnabled()) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Enabled");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disabled");
		}
		
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Windows")) {
		auto& windowManager = RenderWindows::getInstance();

		// Hierarchy
		if (auto* hierarchyWindow = windowManager.GetWindow<Hierarchy>()) {
			if (ImGui::MenuItem("Hierarchy", nullptr, &hierarchyWindow->isOpen)) {
				// El estado se actualiza automáticamente por ImGui
			}
		}

		// Scene View
		if (auto* sceneViewWindow = windowManager.GetWindow<SceneView>()) {
			if (ImGui::MenuItem("Scene View", nullptr, &sceneViewWindow->isOpen)) {
				// El estado se actualiza automáticamente por ImGui
			}
		}

		// Inspector
		if (auto* inspectorWindow = windowManager.GetWindow<Inspector>()) {
			if (ImGui::MenuItem("Inspector", nullptr, &inspectorWindow->isOpen)) {
				// El estado se actualiza automáticamente por ImGui
			}
		}

		// MaterialEditor
		if (auto* MaterialEditorWindow = windowManager.GetWindow<MaterialEditor>()) {
			if (ImGui::MenuItem("Material Editor", nullptr, &MaterialEditorWindow->isOpen)) {
				// El estado se actualiza automáticamente por ImGui
			}
		}



		// Gizmos
		if (auto* gizmosWindow = windowManager.GetWindow<Gizmos>()) {
			if (ImGui::MenuItem("Gizmos", nullptr, &gizmosWindow->isOpen)) {
				// El estado se actualiza automáticamente por ImGui
			}
		}

		ImGui::Separator();

		// Layout options
		if (ImGui::MenuItem("Reset Layout")) {
			if (auto* hierarchy = windowManager.GetWindow<Hierarchy>()) hierarchy->isOpen = true;
			if (auto* sceneView = windowManager.GetWindow<SceneView>()) sceneView->isOpen = true;
			if (auto* inspector = windowManager.GetWindow<Inspector>()) inspector->isOpen = true;
			if (auto* gizmos = windowManager.GetWindow<Gizmos>()) gizmos->isOpen = true;
		}

		if (ImGui::MenuItem("Minimize All")) {
			if (auto* hierarchy = windowManager.GetWindow<Hierarchy>()) hierarchy->isOpen = false;
			if (auto* sceneView = windowManager.GetWindow<SceneView>()) sceneView->isOpen = false;
			if (auto* inspector = windowManager.GetWindow<Inspector>()) inspector->isOpen = false;
			if (auto* gizmos = windowManager.GetWindow<Gizmos>()) gizmos->isOpen = false;
		}

		ImGui::EndMenu();
	}

	// ==== POPUP DE CREAR/EDITAR MATERIAL ====
	static bool materialesCargados = false;
	static std::vector<std::string> materialNames;
	static int selectedMaterial = 0;
	static nlohmann::json materialsConfigJson;
	static std::string lastKey = "";

	ImGui::EndMainMenuBar();
}
