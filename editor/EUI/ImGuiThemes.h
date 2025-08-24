#pragma once
#include <imgui/imgui.h>

class ImGuiThemes
{
public:
    static void setProfessionalTheme()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        ImVec4 bg = ImVec4(45 / 255.0f, 45 / 255.0f, 45 / 255.0f, 1.0f);     // Fondo general
        ImVec4 border = ImVec4(60 / 255.0f, 60 / 255.0f, 60 / 255.0f, 1.0f); // Bordes
        ImVec4 textWhite = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);                   // Texto blanco

        ImVec4 button = ImVec4(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 1.0f);       // Botón normal
        ImVec4 buttonHover = ImVec4(60 / 255.0f, 60 / 255.0f, 60 / 255.0f, 1.0f);  // Botón hover
        ImVec4 buttonActive = ImVec4(90 / 255.0f, 90 / 255.0f, 90 / 255.0f, 1.0f); // Botón presionado

        // === TEXTOS ===
        colors[ImGuiCol_Text] = textWhite;
        colors[ImGuiCol_TextDisabled] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

        // === FONDOS ===
        colors[ImGuiCol_WindowBg] = bg;
        colors[ImGuiCol_ChildBg] = bg;
        colors[ImGuiCol_PopupBg] = bg;
        colors[ImGuiCol_MenuBarBg] = bg;
        colors[ImGuiCol_FrameBg] = button;
        colors[ImGuiCol_FrameBgHovered] = buttonHover;
        colors[ImGuiCol_FrameBgActive] = buttonActive;

        // === BORDES ===
        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_BorderShadow] = border;

        // === TITULOS ===
        colors[ImGuiCol_TitleBg] = bg;
        colors[ImGuiCol_TitleBgActive] = button;
        colors[ImGuiCol_TitleBgCollapsed] = border;

        // === BOTONES ===
        colors[ImGuiCol_Button] = button;
        colors[ImGuiCol_ButtonHovered] = buttonHover;
        colors[ImGuiCol_ButtonActive] = buttonActive;

        // === HEADERS ===
        colors[ImGuiCol_Header] = button;
        colors[ImGuiCol_HeaderHovered] = buttonHover;
        colors[ImGuiCol_HeaderActive] = buttonActive;

        // === TABS ===
        colors[ImGuiCol_Tab] = button;
        colors[ImGuiCol_TabHovered] = buttonHover;
        colors[ImGuiCol_TabActive] = buttonActive;
        colors[ImGuiCol_TabUnfocused] = border;
        colors[ImGuiCol_TabUnfocusedActive] = button;

        // === SEPARADORES ===
        colors[ImGuiCol_Separator] = border;
        colors[ImGuiCol_SeparatorHovered] = buttonHover;
        colors[ImGuiCol_SeparatorActive] = buttonActive;

        // === SCROLLBARS ===
        colors[ImGuiCol_ScrollbarBg] = bg;
        colors[ImGuiCol_ScrollbarGrab] = button;
        colors[ImGuiCol_ScrollbarGrabHovered] = buttonHover;
        colors[ImGuiCol_ScrollbarGrabActive] = buttonActive;

        // === GRIPS ===
        colors[ImGuiCol_ResizeGrip] = button;
        colors[ImGuiCol_ResizeGripHovered] = buttonHover;
        colors[ImGuiCol_ResizeGripActive] = buttonActive;

        // === DOCKING ===
        colors[ImGuiCol_DockingPreview] = buttonHover;
        colors[ImGuiCol_DockingEmptyBg] = bg;

        // === PLOTS ===
        colors[ImGuiCol_PlotLines] = textWhite;
        colors[ImGuiCol_PlotLinesHovered] = buttonHover;
        colors[ImGuiCol_PlotHistogram] = textWhite;
        colors[ImGuiCol_PlotHistogramHovered] = buttonHover;

        // === ESTILO GENERAL ===
        style.WindowRounding = 4.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 14.0f;

        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(6, 4);
        style.ItemInnerSpacing = ImVec2(4, 4);
        style.IndentSpacing = 20;
        style.ColumnsMinSpacing = 6;

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6f;
    }
};
