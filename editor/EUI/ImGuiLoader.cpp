#include "ImGuiLoader.h"
#include "ImGuiThemes.h" 

void ImGuiLoader::StartContext (SDL_Window* _Window, SDL_GLContext _Context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("engine/fonts/Ubuntu-Regular.ttf", 16.0f);

    ImGuiThemes::setProfessionalTheme();
    
    ImGui_ImplSDL3_InitForOpenGL(_Window, _Context);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ImGuiLoader::ImGuiEventPoll (SDL_Event* _Event) {
    ImGui_ImplSDL3_ProcessEvent(_Event);
}

void ImGuiLoader::MakeFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();
}

void ImGuiLoader::SendToRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLoader::CleanEUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}