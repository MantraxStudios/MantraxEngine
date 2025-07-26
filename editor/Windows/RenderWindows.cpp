#include "RenderWindows.h"
#include "Hierarchy.h"
#include "SceneView.h"
#include "Inspector.h"
#include "Gizmos.h"
#include "MainBar.h"


RenderWindows::RenderWindows() {
	windows.push_back(std::make_unique<Hierarchy>());
	windows.push_back(std::make_unique<SceneView>());
	windows.push_back(std::make_unique<Inspector>());
	windows.push_back(std::make_unique<Gizmos>());
	windows.push_back(std::make_unique<MainBar>());

}

void RenderWindows::RenderUI() {
	for (auto& w : windows) {
		if (w->isOpen)
			w->OnRenderGUI();
	}
}