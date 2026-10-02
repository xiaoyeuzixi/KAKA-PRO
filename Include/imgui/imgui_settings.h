#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui.h"
#include "string"
; using namespace ImGui;


// ========== ???????????? ==========
inline int menu_state = 0;
inline char search_all_widgets[120] = { "" };
inline std::string search_all_widgets_;
inline float anim_speed;
inline int rotation_start_index;

// ========== ????G?? ==========
inline ImFont* default_font;
inline ImFont* big_icon;
inline ImFont* big_font;
inline ImFont* icon_font;
inline ImFont* medium_icon_font;
inline ImFont* bold_font;
inline ImFont* bold_big_font;
inline ImFont* icon_small;
inline ImFont* dot_font;

namespace font {
	extern ImFont* menu_ui;
	extern ImFont* menu_ui_bold;
	inline constexpr int kMenuFontCount = 5;
	extern ImFont* menu_ui_sizes[kMenuFontCount];
	extern ImFont* menu_header_sizes[kMenuFontCount];
	extern ImFont* calibri_bold;
	extern ImFont* calibri_regular;

	inline int MenuFontIndex(float uiFontSize) {
		uiFontSize = ImClamp(uiFontSize, 16.f, 22.f);
		if (uiFontSize < 17.f) return 0;
		if (uiFontSize < 19.f) return 1;
		if (uiFontSize < 21.f) return 2;
		return 3;
	}
}

// ========== ??????? ==========
inline float random_float(float min, float max)
{
    return min + float(rand() / float(RAND_MAX)) * (max - min);
}

inline void rect_glow(ImDrawList* draw, ImVec2 start, ImVec2 end, ImColor col, float rounding, float intensity) {
    while (true) {
        if (col.Value.w < 0.0019f)
            break;

        draw->AddRectFilled(start, end, col, rounding);

        col.Value.w -= col.Value.w / intensity;
        start = ImVec2(start.x - 1, start.y - 1);
        end = ImVec2(end.x + 1, end.y + 1);
    }
}

inline void ImRotateStart()
{
    rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
}

inline ImVec2 center_text(ImVec2 min, ImVec2 max, const char* text)
{
    return min + (max - min) / 2 - ImGui::CalcTextSize(text) / 2;
}

inline ImVec2 ImRotationCenter()
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);

    const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
}

inline ImVec4 ImColorToImVec4(const ImColor& color)
{
    return ImVec4(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
}

inline void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
{
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}

// ????????????????
#include <map>

inline bool button_text(const char* first_text, const char* label)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImVec2 pos = window->DC.CursorPos;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    // ?????map?????????????
    static std::map<ImGuiID, std::pair<ImVec4, ImVec4>> button_colors;

    // ??????ID???????????????????
    auto it = button_colors.find(id);
    if (it == button_colors.end()) {
        button_colors[id] = std::make_pair(ImVec4(0, 0, 0, 0), ImVec4(0, 0, 0, 0));
        it = button_colors.find(id);
    }

    // ??????????????????????????
    ImVec4& color_text = it->second.first;
    ImVec4& color_shadow = it->second.second;

    ImVec2 size = CalcTextSize(label);
    ImVec2 first_text_size = CalcTextSize(first_text);
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    const ImRect bb_text(ImVec2(pos.x + first_text_size.x, pos.y),
        ImVec2(pos.x + first_text_size.x + size.x, pos.y + size.y));
    ItemSize(size, style.FramePadding.y);
    ItemAdd(bb, id);
    bool hovered, held;
    bool pressed = ButtonBehavior(bb_text, id, &hovered, &held, 0);

    // ?????????????????
    ImColor main_color(80, 129, 201, 255);
    ImColor text_color[2]{ ImColor(214, 214, 214, 255), ImColor(214, 214, 214, 65) };

    // ??????????????
    color_text = ImLerp(color_text, hovered ? main_color : text_color[0], anim_speed);
    color_shadow = ImLerp(color_shadow, hovered ? ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.1f) : ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.f), anim_speed);

    rect_glow(window->DrawList,
        ImVec2(pos.x + first_text_size.x, pos.y + 2),
        ImVec2(pos.x + first_text_size.x + size.x, pos.y + size.y - 2),
        color_shadow, 360.f, 5.f);
    window->DrawList->AddText(pos, text_color[1], first_text);
    window->DrawList->AddText(ImVec2(pos.x + first_text_size.x, pos.y), GetColorU32(color_text), label);

    return pressed;
}

// ========== ??????????? ==========

// ???? A - ???????????
namespace theme_a {
    inline ImColor main_color(0, 229, 255, 255);       // Cyan #00E5FF
    inline ImColor main_color_shadow(0, 200, 240, 120);
    inline ImColor main_color2(180, 76, 255, 120);      // Purple glow
    inline ImColor main_color_outline(0, 200, 230, 255);
    inline ImColor accent = ImColor(180, 76, 255);       // Purple #B44CFF

    inline ImColor background_color(8, 12, 20, 255);     // Deep space #080C14
    inline ImColor second_color(12, 16, 28, 255);
    inline ImColor winbg(8, 12, 20, 255);                // Deep space

    inline ImColor text_color[2]{ ImColor(235, 240, 250, 255), ImColor(120, 140, 170, 65) };
    inline ImVec4 inputtext_color[3] = { background_color, main_color, text_color[0] };

    namespace background {
        inline ImVec4 filling = ImColor(8, 12, 20);
        inline ImVec4 stroke = ImColor(0, 180, 220, 120);
        inline ImVec4 left = ImColor(0, 200, 230);
        inline ImVec4 left_glow = ImColor(0, 100, 140);
        inline ImVec4 top = ImColor(10, 15, 25);
        inline ImVec4 bottom = ImColor(8, 12, 18);
        inline ImVec2 size = ImVec2(1050, 780);
        inline ImVec2 Loginsize = ImVec2(600, 400);
        inline float rounding = 12;
    }

    namespace checkbox {
        inline ImVec4 circle_inactive = ImColor(20, 25, 38, 255);
        inline ImVec4 background = ImColor(12, 16, 28, 255);
        inline ImVec4 outline_background = ImColor(0, 140, 180, 180);
        inline float rounding = 4;
    }

    namespace elements {
        inline ImVec4 mark = ImColor(0, 229, 255);
        inline ImVec4 child_bg = ImColor(12, 16, 28);
        inline ImVec4 child_top = ImColor(16, 20, 35);
        inline ImVec4 stroke = ImColor(0, 140, 180, 120);
        inline ImVec4 background = ImColor(8, 12, 20);
        inline ImVec4 background_hov = ImColor(18, 24, 40);
        inline ImVec4 background_widget = ImColor(10, 14, 22);
        inline ImVec4 background_widget_stroke = ImColor(0, 160, 200, 150);
        inline ImVec4 checkbox = ImColor(0, 229, 255);
        inline ImVec4 checkbox_active = ImColor(8, 12, 20);
        inline ImVec4 combo_stroke = ImColor(0, 160, 200, 150);
        inline ImVec4 text_active = ImColor(235, 240, 250);
        inline ImVec4 text_hov = ImColor(0, 200, 230);
        inline ImVec4 text = ImColor(100, 120, 150);
        inline ImVec4 tab_active = ImColor(0, 140, 180, 180);
        inline ImVec4 slider_bg = ImColor(20, 26, 40);
        inline float rounding = 4;
    }

    namespace tab {
        inline ImVec4 tab_active_child = ImColor(12, 18, 30);
        inline ImVec4 tab_hov_child = ImColor(0, 140, 180, 100);
        inline ImVec4 tab_child_active = ImColor(10, 15, 25);
        inline ImVec4 tab_active = ImColor(0, 140, 180, 150);
        inline ImVec4 tab_hov = ImColor(0, 100, 140, 80);
        inline ImVec4 tab = ImColor(8, 12, 20, 0);
        inline ImVec4 acc_active = ImColor(0, 229, 255, 60);
        inline ImVec4 acc_hov = ImColor(0, 180, 220, 50);
        inline ImVec4 border = ImColor(0, 120, 160, 80);
    }
}

namespace c {

	inline ImVec4 accent = ImColor(0, 229, 255);            // Cyan #00E5FF
	inline ImVec4 accent1 = ImColor(0, 200, 240, 255);
	inline ImVec4 shadow = ImColor(0, 80, 120);
	inline ImVec4 accent_low = ImColor(0, 160, 200, 120);
	inline ImVec4 logocolor = ImColor(0, 229, 255, 255);

	inline ImVec4 white_light = ImColor(190, 200, 220);
	inline ImVec4 image = ImColor(255, 255, 255, 255);
	inline ImVec4 accent_transparent = ImColor(0, 160, 200, 0);

	namespace bg
	{
		inline ImVec4 background = ImColor(8, 12, 20, 255);
		inline ImVec4 roughness = ImColor(0, 200, 240, 10);
		inline ImVec4 outline = ImColor(0, 160, 200, 150);
		inline ImVec4 top_bg = ImColor(10, 15, 24, 255);

		inline ImVec4 background1 = ImColor(8, 12, 20, 235);

		inline ImVec4 gradient_line0 = ImColor(0, 160, 200, 100);
		inline ImVec4 gradient_line1 = ImColor(0, 160, 200, 0);

		inline ImVec2 size = ImVec2(1160, 770);

		inline ImVec2 size1 = ImVec2(500, 650);
		inline float rounding1 = 8.f;

		inline float rounding = 6;
	}

	namespace child
	{
		inline ImVec4 background = ImColor(16, 20, 32, 250);
		inline ImVec4 border = ImColor(0, 200, 240, 200);
		inline ImVec4 lines = ImColor(0, 160, 200, 120);
		inline float rounding = 4;
	}

	namespace popup_elements
	{
		inline ImVec4 filling = ImColor(0, 0, 0, 240);
		inline ImVec4 cog = ImColor(0, 229, 255, 255);
	}

	namespace checkbox
	{
		inline ImVec4 background = ImColor(45, 55, 80, 255);
		inline ImVec4 outline = ImColor(0, 210, 240, 255);
		inline ImVec4 mark = ImColor(0, 229, 255, 255);
		inline float rounding = 3;


		inline ImVec4 background2 = ImColor(20, 28, 45, 255);
		inline ImVec4 outline2 = ImColor(0, 200, 230, 235);
		inline ImVec4 mark2 = ImColor(0, 229, 255, 255);
		inline float rounding2 = 40;
		inline float rounding3 = 4;

	}



	namespace slider
	{
		inline ImVec4 background = ImColor(20, 24, 36, 255);
		inline float rounding = 4;
	}

	namespace button
	{
		inline ImVec4 background = ImColor(30, 35, 50, 255);
		inline ImVec4 outline = ImColor(0, 180, 200, 180);
		inline float rounding = 10.f;
		inline float rounding1 = 4.f;

	}

	namespace combo
	{
		inline ImVec4 background = ImColor(25, 32, 48, 255);
		inline ImVec4 outline = ImColor(0, 210, 240, 255);
		inline float rounding = 10.f;
	}

	namespace keybind
	{
		inline ImVec4 background = ImColor(40, 55, 80, 255);
		inline float rounding = 10.f;
	}

	namespace input
	{
		inline ImVec4 background = ImColor(24, 28, 40, 255);
		inline ImVec4 outline = ImColor(0, 170, 190, 200);
		inline float rounding = 4;
		inline ImVec4 background1 = ImColor(12, 16, 28, 245);
		inline ImVec4 outline1 = ImColor(0, 200, 240, 180);

		inline float rounding1 = 4.f;
	}

	namespace exit_panel
	{
		inline ImVec4 background1 = ImColor(12, 16, 28, 245);
		inline ImVec4 outline = ImColor(0, 160, 200, 180);
	}

	namespace picker
	{
		inline ImVec4 background = ImColor(12, 16, 28, 255);
		inline float rounding = 2;
	}

	namespace tabs
	{
		inline ImVec4 line = ImColor(0, 120, 160, 100);

	}

	namespace knobs
	{
		inline ImVec4 background = ImColor(14, 18, 30, 255);

	}

	namespace checkbox1
	{
		inline ImVec4 checkmark_active = ImColor(0, 229, 255, 245);
		inline ImVec4 checkmark_inactive = ImColor(0, 229, 255, 0);


		inline ImVec4 background = ImColor(12, 16, 28, 245);
		inline float rounding = 3.f;
	}

	namespace text
	{
		inline ImVec4 text_active = ImColor(60, 70, 90, 255);
		inline ImVec4 text_hov = ImColor(180, 190, 210, 255);
		inline ImVec4 text = ImColor(180, 190, 210, 255);
		inline ImVec4 text_active1 = ImColor(70, 80, 100, 255);
		inline ImVec4 text1 = ImColor(170, 180, 200, 255);

	}

	namespace scrollbar
	{
		inline ImVec4 bar_active = ImColor(0, 200, 240, 180);
		inline ImVec4 bar_hov = ImColor(0, 160, 200, 150);
		inline ImVec4 bar = ImColor(0, 100, 140, 80);
	}

}
class c_custom {

public:
	float col_buf[4] = { 1.f, 1.f, 1.f, 1.f };
};
inline c_custom custom2;