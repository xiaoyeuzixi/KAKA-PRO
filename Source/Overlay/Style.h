#pragma once
#include "imgui/imgui.h"

inline void ImGuiSetStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.4f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.WindowRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 4.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupRounding = 4.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.FrameRounding = 3.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 3.0f);
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 8.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.TabBorderSize = 0.0f;
    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    // ---- Neon Cyber Theme: Deep Space Background ----
    // Base: #0A0C14 (ultra-dark blue-black)
    // Primary: #00E5FF (electric cyan)
    // Secondary: #B44CFF (neon purple)
    // Warning: #FF8C00 (amber)
    // Success: #00FF88 (neon green)

    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.97f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.45f, 0.48f, 0.55f, 0.70f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.04f, 0.05f, 0.08f, 0.98f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.06f, 0.07f, 0.12f, 0.98f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.97f);
    colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.65f, 0.75f, 1.00f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.16f, 0.19f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.16f, 0.18f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.00f, 0.50f, 0.60f, 0.40f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.03f, 0.04f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.04f, 0.05f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.03f, 0.04f, 0.07f, 0.80f);
    colors[ImGuiCol_MenuBarBg]            = ImVec4(0.06f, 0.07f, 0.11f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.04f, 0.05f, 0.08f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.00f, 0.60f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.75f, 0.90f, 0.60f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.00f, 0.90f, 1.00f, 0.80f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.00f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.00f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.00f, 0.80f, 0.90f, 1.00f);
    colors[ImGuiCol_Button]              = ImVec4(0.14f, 0.17f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonHovered]       = ImVec4(0.18f, 0.22f, 0.34f, 1.00f);
    colors[ImGuiCol_ButtonActive]        = ImVec4(0.00f, 0.55f, 0.65f, 0.90f);
    colors[ImGuiCol_Header]              = ImVec4(0.10f, 0.13f, 0.20f, 0.80f);
    colors[ImGuiCol_HeaderHovered]       = ImVec4(0.14f, 0.17f, 0.26f, 0.90f);
    colors[ImGuiCol_HeaderActive]        = ImVec4(0.00f, 0.45f, 0.55f, 0.80f);
    colors[ImGuiCol_Separator]          = ImVec4(0.00f, 0.35f, 0.45f, 0.40f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.00f, 0.50f, 0.60f, 0.60f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.00f, 0.65f, 0.75f, 0.80f);
    colors[ImGuiCol_ResizeGrip]         = ImVec4(0.00f, 0.40f, 0.50f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]  = ImVec4(0.00f, 0.55f, 0.65f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.00f, 0.70f, 0.80f, 0.90f);
    colors[ImGuiCol_Tab]                = ImVec4(0.06f, 0.08f, 0.13f, 0.95f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.12f, 0.15f, 0.24f, 0.90f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.00f, 0.45f, 0.55f, 0.95f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.05f, 0.06f, 0.10f, 0.90f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.25f, 0.35f, 0.75f);
    colors[ImGuiCol_PlotLines]          = ImVec4(0.00f, 0.70f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]   = ImVec4(0.50f, 0.20f, 0.80f, 1.00f);
    colors[ImGuiCol_PlotHistogram]      = ImVec4(0.00f, 0.65f, 0.75f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]= ImVec4(0.50f, 0.22f, 0.80f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]      = ImVec4(0.10f, 0.14f, 0.22f, 0.95f);
    colors[ImGuiCol_TableBorderStrong]  = ImVec4(0.00f, 0.50f, 0.60f, 0.70f);
    colors[ImGuiCol_TableBorderLight]   = ImVec4(0.00f, 0.35f, 0.45f, 0.50f);
    colors[ImGuiCol_TableRowBg]         = ImVec4(0.14f, 0.17f, 0.26f, 0.90f);
    colors[ImGuiCol_TableRowBgAlt]      = ImVec4(0.08f, 0.22f, 0.32f, 0.80f);
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.00f, 0.40f, 0.50f, 0.50f);
    colors[ImGuiCol_DragDropTarget]     = ImVec4(0.00f, 0.70f, 0.80f, 0.90f);
    colors[ImGuiCol_NavHighlight]       = ImVec4(0.00f, 0.55f, 0.65f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_NavWindowingDimBg]  = ImVec4(0.04f, 0.05f, 0.08f, 0.50f);
    colors[ImGuiCol_ModalWindowDimBg]   = ImVec4(0.04f, 0.05f, 0.08f, 0.70f);
}