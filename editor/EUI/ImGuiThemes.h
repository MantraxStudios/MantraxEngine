#pragma once
#include <imgui/imgui.h>

class ImGuiThemes
{
public:
	static void setProfessionalTheme() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		// Colores de texto - texto claro sobre fondo oscuro
		colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);

		// Fondos principales - basado en #57564F
		colors[ImGuiCol_WindowBg] = ImVec4(0.34f, 0.34f, 0.31f, 1.00f);        // #57564F
		colors[ImGuiCol_ChildBg] = ImVec4(0.38f, 0.37f, 0.34f, 1.00f);         // Un poco m�s claro
		colors[ImGuiCol_PopupBg] = ImVec4(0.30f, 0.30f, 0.28f, 0.98f);         // Un poco m�s oscuro

		// Bordes definidos
		colors[ImGuiCol_Border] = ImVec4(0.22f, 0.22f, 0.20f, 0.80f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.15f, 0.15f, 0.13f, 0.30f);

		// Frames/Inputs - variaciones del color base
		colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.41f, 0.38f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.46f, 0.50f, 0.56f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.54f, 0.60f, 1.00f);

		// T�tulos de ventana
		colors[ImGuiCol_TitleBg] = ImVec4(0.28f, 0.28f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.32f, 0.32f, 0.29f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.30f, 0.30f, 0.28f, 0.75f);

		// Barra de men�
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.32f, 0.32f, 0.29f, 1.00f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.30f, 0.30f, 0.28f, 0.75f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.47f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.57f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.70f, 0.67f, 1.00f);

		// Elementos interactivos - accent color azul profesional
		colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.20f, 0.50f, 0.87f, 1.00f);

		// Botones
		colors[ImGuiCol_Button] = ImVec4(0.46f, 0.45f, 0.42f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.54f, 0.60f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.54f, 0.58f, 0.64f, 1.00f);

		// Headers
		colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.37f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.48f, 0.54f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.52f, 0.58f, 1.00f);

		// Separadores
		colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.23f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

		// Resize grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.38f, 0.37f, 0.34f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.46f, 0.52f, 1.00f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.34f, 0.34f, 0.31f, 1.00f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.32f, 0.32f, 0.29f, 1.00f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.36f, 0.36f, 0.33f, 1.00f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.70f, 0.90f, 1.00f);

		// Docking
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.30f, 0.30f, 0.28f, 1.00f);

		// Plots
		colors[ImGuiCol_PlotLines] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.50f, 0.87f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.20f, 0.50f, 0.87f, 1.00f);

		// Tablas
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.30f, 0.30f, 0.28f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.22f, 0.22f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.26f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.40f, 0.40f, 0.37f, 0.25f);

		// Otros elementos
		colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.90f);
		colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

		// Configuraci�n de estilo para un look m�s profesional
		style.WindowRounding = 6.0f;
		style.FrameRounding = 4.0f;
		style.GrabRounding = 3.0f;
		style.PopupRounding = 4.0f;
		style.TabRounding = 4.0f;
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ScrollbarSize = 12.0f;
		style.GrabMinSize = 12.0f;
		style.DockingSeparatorSize = 2.0f;
		style.SeparatorTextBorderSize = 1.0f;

		// Padding y espaciado m�s profesional
		style.WindowPadding = ImVec2(12.0f, 12.0f);
		style.FramePadding = ImVec2(8.0f, 4.0f);
		style.ItemSpacing = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
		style.IndentSpacing = 20.0f;
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
	}

	// Funci�n original mantenida para compatibilidad
	static void setDarkTheme() {
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.04f, 0.04f, 0.04f, 0.99f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.03f, 0.03f, 0.04f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.17f, 0.17f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.43f, 0.90f, 0.11f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.21f, 0.22f, 0.23f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.54f, 0.55f, 0.55f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.38f, 0.51f, 0.51f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.16f, 0.16f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.23f, 0.23f, 0.24f, 0.80f);
		colors[ImGuiCol_Tab] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.13f, 0.78f, 0.07f, 1.00f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.10f, 0.60f, 0.12f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.14f, 0.87f, 0.05f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 0.60f, 0.10f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.23f, 0.78f, 0.02f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.46f, 0.47f, 0.46f, 0.06f);
		colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.78f, 0.69f, 0.69f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		style.WindowRounding = 4.0f;
		style.FrameRounding = 4.0f;
		style.GrabRounding = 3.0f;
		style.PopupRounding = 4.0f;
		style.TabRounding = 4.0f;
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ScrollbarSize = 10.0f;
		style.GrabMinSize = 10.0f;
		style.DockingSeparatorSize = 1.0f;
		style.SeparatorTextBorderSize = 2.0f;
	}
};