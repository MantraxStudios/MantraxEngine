#pragma once
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <SDL.h>

// ==== ImGui ====
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_opengl3.h>


class ImGuiLoader {
public:
	void static StartContext(SDL_Window* _Window, SDL_GLContext _Context);
	void static ImGuiEventPoll(SDL_Event* _Event);
	void static MakeFrame();
	void static SendToRender();
	void static CleanEUI();
};