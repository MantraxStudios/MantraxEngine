#pragma once
#include <imgui/imgui.h>

class ImGuiThemes
{
public:
    static void setProfessionalTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // === COLORES BASE INSPIRADOS EN UNITY ===
        // Paleta de colores profesional
        const ImVec4 darkBg = ImVec4(0.23f, 0.24f, 0.25f, 1.00f);          // Gris oscuro principal
        const ImVec4 mediumBg = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);        // Gris medio
        const ImVec4 lightBg = ImVec4(0.32f, 0.33f, 0.34f, 1.00f);         // Gris claro
        const ImVec4 accentBlue = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);      // Azul Unity
        const ImVec4 accentOrange = ImVec4(1.00f, 0.63f, 0.00f, 1.00f);    // Naranja Unity
        const ImVec4 textWhite = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);       // Blanco suave
        const ImVec4 textGray = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);        // Gris para texto secundario

        // === TEXTO ===
        colors[ImGuiCol_Text] = textWhite;
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.35f);
        colors[ImGuiCol_TextLink] = accentBlue;

        // === FONDOS PRINCIPALES ===
        colors[ImGuiCol_WindowBg] = darkBg;
        colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);     // M�s oscuro para contraste
        colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.26f, 0.27f, 0.98f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.19f, 0.20f, 0.21f, 1.00f);

        // === BORDES ===
        colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

        // === T�TULOS DE VENTANA ===
        colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.20f, 0.21f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.23f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.18f, 0.19f, 0.75f);

        // === FRAMES E INPUTS ===
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(accentBlue.x * 0.8f, accentBlue.y * 0.8f, accentBlue.z * 0.8f, 0.6f);

        // === SCROLLBAR ===
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.36f, 0.37f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.46f, 0.47f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.8f);

        // === ELEMENTOS INTERACTIVOS ===
        colors[ImGuiCol_CheckMark] = accentOrange;
        colors[ImGuiCol_SliderGrab] = accentBlue;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accentBlue.x * 1.2f, accentBlue.y * 1.2f, accentBlue.z * 1.2f, 1.00f);

        // === BOTONES ===
        colors[ImGuiCol_Button] = mediumBg;
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.36f, 0.37f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(accentBlue.x * 0.7f, accentBlue.y * 0.7f, accentBlue.z * 0.7f, 1.00f);

        // === HEADERS ===
        colors[ImGuiCol_Header] = ImVec4(0.26f, 0.27f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.31f, 0.32f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accentBlue.x * 0.8f, accentBlue.y * 0.8f, accentBlue.z * 0.8f, 1.00f);

        // === SEPARADORES ===
        colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.40f, 0.62f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 1.00f);

        // === RESIZE GRIP ===
        colors[ImGuiCol_ResizeGrip] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.95f);

        // === TABS ===
        colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.21f, 0.22f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);
        colors[ImGuiCol_TabSelected] = ImVec4(0.25f, 0.26f, 0.27f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = accentOrange;
        colors[ImGuiCol_TabDimmed] = ImVec4(0.16f, 0.17f, 0.18f, 1.00f);
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.22f, 0.23f, 0.24f, 1.00f);
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(accentOrange.x * 0.7f, accentOrange.y * 0.7f, accentOrange.z * 0.7f, 1.00f);

        // === DOCKING ===
        colors[ImGuiCol_DockingPreview] = ImVec4(accentBlue.x, accentBlue.y, accentBlue.z, 0.70f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);

        // === PLOTS ===
        colors[ImGuiCol_PlotLines] = accentBlue;
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(accentBlue.x * 1.2f, accentBlue.y * 1.2f, accentBlue.z * 1.2f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = accentOrange;
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(accentOrange.x * 1.2f, accentOrange.y * 1.2f, accentOrange.z * 1.2f, 1.00f);

        // === TABLAS ===
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.20f, 0.21f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);

        // === OTROS ELEMENTOS ===
        colors[ImGuiCol_DragDropTarget] = ImVec4(accentOrange.x, accentOrange.y, accentOrange.z, 0.90f);
        colors[ImGuiCol_NavCursor] = accentBlue;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        // === CONFIGURACI�N DE ESTILO PROFESIONAL ===
        // Bordes redondeados suaves
        style.WindowRounding = 8.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        // Tama�os y espacio
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 14.0f;

        // Padding y espaciado m�s generoso
        style.WindowPadding = ImVec2(16.0f, 16.0f);
        style.FramePadding = ImVec2(12.0f, 6.0f);
        style.CellPadding = ImVec2(8.0f, 4.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
        style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
        style.IndentSpacing = 24.0f;
        style.ColumnsMinSpacing = 8.0f;

        // Configuraci�n de ventana
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        // Separadores y docking
        style.SeparatorTextBorderSize = 3.0f;
        style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
        style.SeparatorTextPadding = ImVec2(24.0f, 4.0f);
        style.DockingSeparatorSize = 2.0f;

        // Configuraci�n de transparencia (Alpha)
        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6f;
    }
};