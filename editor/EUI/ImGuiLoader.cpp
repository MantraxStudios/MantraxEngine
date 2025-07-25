#include "ImGuiLoader.h"

void ImGuiLoader::StartContext (SDL_Window* _Window, SDL_GLContext _Context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL2_InitForOpenGL(_Window, _Context);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ImGuiLoader::ImGuiEventPoll (SDL_Event* _Event) {
    ImGui_ImplSDL2_ProcessEvent(_Event);
}

void ImGuiLoader::MakeFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();
}

void ImGuiLoader::SendToRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLoader::CleanEUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}