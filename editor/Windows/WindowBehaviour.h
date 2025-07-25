#pragma once

class WindowBehaviour
{
public:
	bool isOpen = true;
	void virtual OnRenderGUI() = 0;
};