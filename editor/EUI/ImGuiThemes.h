#pragma once
#include <imgui/imgui.h>

class ImGuiThemes
{
public:
    static void setProfessionalTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        ImVec4 bg = ImVec4(23/255.0f, 23/255.0f, 23/255.0f, 1.0f);      // Fondo general
        ImVec4 border = ImVec4(30/255.0f, 30/255.0f, 30/255.0f, 1.0f);  // Bordes
        ImVec4 textWhite = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);               // Texto blanco
        ImVec4 button = ImVec4(30/255.0f, 30/255.0f, 30/255.0f, 1.0f);   // Botón normal
        ImVec4 buttonHover = ImVec4(35/255.0f, 35/255.0f, 35/255.0f, 1.0f); // Botón hover

        // === TEXTOS ===
        colors[ImGuiCol_Text] = textWhite;
        colors[ImGuiCol_TextDisabled] = ImVec4(0.7f,0.7f,0.7f,1.0f);

        // === FONDOS ===
        colors[ImGuiCol_WindowBg] = bg;
        colors[ImGuiCol_ChildBg] = bg;
        colors[ImGuiCol_PopupBg] = bg;
        colors[ImGuiCol_MenuBarBg] = bg;
        colors[ImGuiCol_FrameBg] = bg;
        colors[ImGuiCol_FrameBgHovered] = bg;
        colors[ImGuiCol_FrameBgActive] = bg;

        // === BORDES ===
        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_BorderShadow] = border;

        // === TITULOS ===
        colors[ImGuiCol_TitleBg] = bg;
        colors[ImGuiCol_TitleBgActive] = bg;
        colors[ImGuiCol_TitleBgCollapsed] = bg;

        // === BOTONES ===
        colors[ImGuiCol_Button] = button;
        colors[ImGuiCol_ButtonHovered] = buttonHover;
        colors[ImGuiCol_ButtonActive] = bg;

        // === HEADERS, TABS, SEPARADORES, SCROLLBARS ===
        colors[ImGuiCol_Header] = bg;
        colors[ImGuiCol_HeaderHovered] = bg;
        colors[ImGuiCol_HeaderActive] = bg;

        colors[ImGuiCol_Tab] = bg;
        colors[ImGuiCol_TabHovered] = bg;
        colors[ImGuiCol_TabActive] = bg;
        colors[ImGuiCol_TabUnfocused] = bg;
        colors[ImGuiCol_TabUnfocusedActive] = bg;

        colors[ImGuiCol_Separator] = border;
        colors[ImGuiCol_SeparatorHovered] = border;
        colors[ImGuiCol_SeparatorActive] = border;

        colors[ImGuiCol_ScrollbarBg] = bg;
        colors[ImGuiCol_ScrollbarGrab] = button;
        colors[ImGuiCol_ScrollbarGrabHovered] = bg;
        colors[ImGuiCol_ScrollbarGrabActive] = bg;

        // === GRAB, DOCK, PLOTS ===
        colors[ImGuiCol_ResizeGrip] = bg;
        colors[ImGuiCol_ResizeGripHovered] = bg;
        colors[ImGuiCol_ResizeGripActive] = bg;

        colors[ImGuiCol_DockingPreview] = bg;
        colors[ImGuiCol_DockingEmptyBg] = bg;

        colors[ImGuiCol_PlotLines] = textWhite;
        colors[ImGuiCol_PlotLinesHovered] = textWhite;
        colors[ImGuiCol_PlotHistogram] = textWhite;
        colors[ImGuiCol_PlotHistogramHovered] = textWhite;

        // === ESTILO ===
        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 0.0f;
        style.PopupRounding = 0.0f;
        style.ScrollbarRounding = 0.0f;
        style.GrabRounding = 0.0f;
        style.TabRounding = 0.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 14.0f;

        style.WindowPadding = ImVec2(8,8);
        style.FramePadding = ImVec2(6,4);
        style.ItemSpacing = ImVec2(6,4);
        style.ItemInnerSpacing = ImVec2(4,4);
        style.IndentSpacing = 20;
        style.ColumnsMinSpacing = 6;

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6f;
    }
};
