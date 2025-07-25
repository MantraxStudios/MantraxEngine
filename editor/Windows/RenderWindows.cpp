#include "RenderWindows.h"
#include "Hierarchy.h"
#include "SceneView.h"

RenderWindows::RenderWindows() {
	windows.push_back(std::make_unique<Hierarchy>());
	windows.push_back(std::make_unique<SceneView>());
}

void RenderWindows::RenderUI() {
	for (auto& w : windows) {
		if (w->isOpen)
			w->OnRenderGUI();
	}
}