#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Texture.h"
#include <Common/Data.h>

namespace RenderHelper {
	inline ImU32 GetColorForNumber(int number) {
		switch (number) {
		case 1:  return IM_COL32(247, 248, 19, 255);    // Yellow
		case 2:  return IM_COL32(250, 127, 73, 255);    // Orange
		case 3:  return IM_COL32(90, 198, 227, 255);    // Light Blue
		case 4:  return IM_COL32(90, 189, 77, 255);     // Green
		case 5:  return IM_COL32(225, 99, 120, 255);    // Pink
		case 6:  return IM_COL32(115, 129, 168, 255);   // Purple
		case 7:  return IM_COL32(159, 126, 105, 255);   // Indigo
		case 8:  return IM_COL32(255, 134, 200, 255);   // Light Cyan
		case 9:  return IM_COL32(210, 224, 191, 255);   // Pale Green
		case 10: return IM_COL32(154, 52, 142, 255);    // Violet
		case 11: return IM_COL32(98, 146, 158, 255);       // Red
		case 12: return IM_COL32(226, 214, 239, 255);       // Green
		case 13: return IM_COL32(4, 167, 119, 255);       // Blue
		case 14: return IM_COL32(115, 113, 252, 255);     // Yellow
		case 15: return IM_COL32(255, 0, 255, 255);     // Magenta
		case 16: return IM_COL32(93, 46, 140, 255);     // Cyan
		case 17: return IM_COL32(0, 255, 0, 255);       // Lime
		case 18: return IM_COL32(0, 0, 255, 255);       // Blue
		case 19: return IM_COL32(255, 165, 0, 255);     // Orange
		case 20: return IM_COL32(128, 0, 128, 255);     // Purple
		case 21: return IM_COL32(255, 192, 203, 255);   // Pink
		case 22: return IM_COL32(128, 128, 0, 255);     // Olive
		case 23: return IM_COL32(255, 215, 0, 255);     // Gold
		case 24: return IM_COL32(75, 0, 130, 255);      // Indigo
		case 25: return IM_COL32(0, 191, 255, 255);     // Deep Sky Blue
		case 26: return IM_COL32(255, 105, 180, 255);   // Hot Pink
		case 27: return IM_COL32(139, 69, 19, 255);     // Saddle Brown
		case 28: return IM_COL32(220, 20, 60, 255);     // Crimson
		case 29: return IM_COL32(0, 255, 127, 255);     // Spring Green
		case 30: return IM_COL32(0, 250, 154, 255);     // Medium Spring Green
		case 31: return IM_COL32(72, 61, 139, 255);     // Dark Slate Blue
		case 32: return IM_COL32(143, 188, 143, 255);   // Dark Sea Green
		case 33: return IM_COL32(178, 34, 34, 255);     // Firebrick
		case 34: return IM_COL32(153, 50, 204, 255);    // Dark Orchid
		case 35: return IM_COL32(233, 150, 122, 255);   // Dark Salmon
		case 36: return IM_COL32(148, 0, 211, 255);     // Dark Violet
		case 37: return IM_COL32(95, 158, 160, 255);    // Cadet Blue
		case 38: return IM_COL32(127, 255, 212, 255);   // Aquamarine
		case 39: return IM_COL32(218, 112, 214, 255);   // Orchid
		case 40: return IM_COL32(244, 164, 96, 255);    // Sandy Brown
		case 41: return IM_COL32(210, 105, 30, 255);    // Chocolate
		case 42: return IM_COL32(222, 184, 135, 255);   // Burlywood
		case 43: return IM_COL32(255, 228, 181, 255);   // Moccasin
		case 44: return IM_COL32(255, 239, 213, 255);   // Papaya Whip
		case 45: return IM_COL32(175, 238, 238, 255);   // Pale Turquoise
		case 46: return IM_COL32(100, 149, 237, 255);   // Cornflower Blue
		case 47: return IM_COL32(219, 112, 147, 255);   // Pale Violet Red
		case 48: return IM_COL32(173, 216, 230, 255);   // Light Blue
		case 49: return IM_COL32(240, 128, 128, 255);   // Light Coral
		case 50: return IM_COL32(255, 248, 220, 255);   // Cornsilk
		case 51: return IM_COL32(255, 99, 71, 255);      // Tomato
		case 52: return IM_COL32(255, 140, 0, 255);      // Dark Orange
		case 53: return IM_COL32(154, 205, 50, 255);     // Yellow Green
		case 54: return IM_COL32(34, 139, 34, 255);      // Forest Green
		case 55: return IM_COL32(70, 130, 180, 255);     // Steel Blue
		case 56: return IM_COL32(123, 104, 238, 255);    // Medium Slate Blue
		case 57: return IM_COL32(255, 20, 147, 255);     // Deep Pink
		case 58: return IM_COL32(65, 105, 225, 255);     // Royal Blue
		case 59: return IM_COL32(240, 230, 140, 255);    // Khaki
		case 60: return IM_COL32(0, 128, 128, 255);      // Teal
		case 61: return IM_COL32(216, 191, 216, 255);    // Thistle
		case 62: return IM_COL32(176, 224, 230, 255);    // Powder Blue
		case 63: return IM_COL32(0, 100, 0, 255);        // Dark Green
		case 64: return IM_COL32(255, 69, 0, 255);       // Orange Red
		case 65: return IM_COL32(199, 21, 133, 255);     // Medium Violet Red
		case 66: return IM_COL32(0, 206, 209, 255);      // Dark Turquoise
		case 67: return IM_COL32(128, 0, 0, 255);        // Maroon
		case 68: return IM_COL32(135, 206, 235, 255);    // Sky Blue
		case 69: return IM_COL32(255, 228, 225, 255);    // Misty Rose
		case 70: return IM_COL32(210, 180, 140, 255);    // Tan
		case 71: return IM_COL32(32, 178, 170, 255);     // Light Sea Green
		case 72: return IM_COL32(255, 250, 205, 255);    // Lemon Chiffon
		case 73: return IM_COL32(138, 43, 226, 255);     // Blue Violet
		case 74: return IM_COL32(124, 252, 0, 255);      // Lawn Green
		case 75: return IM_COL32(176, 196, 222, 255);    // Light Steel Blue
		case 76: return IM_COL32(210, 105, 30, 255);     // Chocolate
		case 77: return IM_COL32(255, 182, 193, 255);    // Light Pink
		case 78: return IM_COL32(152, 251, 152, 255);    // Pale Green
		case 79: return IM_COL32(221, 160, 221, 255);    // Plum
		case 80: return IM_COL32(50, 205, 50, 255);      // Lime Green
		case 81: return IM_COL32(0, 0, 139, 255);        // Dark Blue
		case 82: return IM_COL32(255, 215, 0, 255);      // Gold
		case 83: return IM_COL32(0, 191, 255, 255);      // Deep Sky Blue
		case 84: return IM_COL32(218, 112, 214, 255);    // Orchid
		case 85: return IM_COL32(255, 99, 71, 255);      // Tomato
		case 86: return IM_COL32(72, 209, 204, 255);     // Medium Turquoise
		case 87: return IM_COL32(139, 0, 139, 255);      // Dark Magenta
		case 88: return IM_COL32(240, 128, 128, 255);    // Light Coral
		case 89: return IM_COL32(255, 250, 240, 255);    // Floral White
		case 90: return IM_COL32(127, 255, 212, 255);    // Aquamarine
		case 91: return IM_COL32(245, 222, 179, 255);    // Wheat
		case 92: return IM_COL32(255, 160, 122, 255);    // Light Salmon
		case 93: return IM_COL32(32, 178, 170, 255);     // Light Sea Green
		case 94: return IM_COL32(70, 130, 180, 255);     // Steel Blue
		case 95: return IM_COL32(106, 90, 205, 255);     // Slate Blue
		case 96: return IM_COL32(0, 128, 128, 255);      // Teal
		case 97: return IM_COL32(255, 105, 180, 255);    // Hot Pink
		case 98: return IM_COL32(244, 164, 96, 255);     // Sandy Brown
		case 99: return IM_COL32(255, 239, 213, 255);    // Papaya Whip
		case 100:return IM_COL32(173, 216, 230, 255);    // Light Blue
		default: return IM_COL32(102, 102, 102, 255);   // Gray
		}
	}

	ImColor GetTeamColor(int TeamID)
	{
		switch (TeamID)
		{
		case 0:
			return ImColor(0, 229, 255);    // Cyan
		case 1:
			return ImColor(180, 76, 255);   // Purple
		case 2:
			return ImColor(0, 255, 136);    // Neon Green
		case 3:
			return ImColor(255, 140, 0);    // Amber
		case 4:
			return ImColor(255, 60, 80);    // Red
		}
		return ImColor(0, 229, 255);
	}

	struct PlayerColor {
		ImColor infoUseColor;
		ImColor skeletonUseColor;
		bool isUseTeamNumberColor;
		ImColor teamNumberColor;
	};

	inline PlayerColor GetPlayerColor(const Player player)
	{
		auto teamNumberColor = GetColorForNumber(player.TeamID);
		bool isUseTeamNumberColor = false;
		auto infoColor = GameData.Config.ESP.Color.Default.Info;
		auto skeletonColor = GameData.Config.ESP.Color.Default.Skeleton;

		if (player.GroggyHealth < 99)
		{
			infoColor = GameData.Config.ESP.Color.Groggy.Info;
			skeletonColor = GameData.Config.ESP.Color.Groggy.Skeleton;
		}
		else if (player.ListType == 1)
		{
			infoColor = GameData.Config.ESP.Color.Blacklist.Info;
			skeletonColor = GameData.Config.ESP.Color.Blacklist.Skeleton;
		}
		else if (player.ListType == 2)
		{
			infoColor = GameData.Config.ESP.Color.Whitelist.Info;
			skeletonColor = GameData.Config.ESP.Color.Whitelist.Skeleton;
		}
		else if (player.PartnerLevel > 0)
		{
			infoColor = GameData.Config.ESP.Color.Partner.Info;
			skeletonColor = GameData.Config.ESP.Color.Partner.Skeleton;
		}
		else if (player.KillCount > 3)
		{
			infoColor = GameData.Config.ESP.Color.Dangerous.Info;
			skeletonColor = GameData.Config.ESP.Color.Dangerous.Skeleton;
		}
		else if (player.SpectatedCount > 2)
		{
			infoColor = GameData.Config.ESP.Color.Dangerous.Info;;
			skeletonColor = GameData.Config.ESP.Color.Dangerous.Skeleton;
		}

		else if (player.Type == EntityType::AI)
		{
			infoColor = GameData.Config.ESP.Color.AI.Info;;
			skeletonColor = GameData.Config.ESP.Color.AI.Skeleton;
		}
		else if (player.IsVisible && GameData.Config.ESP.VisibleCheck)
		{
			infoColor = GameData.Config.ESP.Color.Visible.Info;;
			skeletonColor = GameData.Config.ESP.Color.Visible.Skeleton;
		}
		else {
			isUseTeamNumberColor = true;
		}
		//
		auto infoUseColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(infoColor));
		auto skeletonUseColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(skeletonColor));

		PlayerColor playerColor = {
			infoUseColor,
			skeletonUseColor,
			isUseTeamNumberColor,
			teamNumberColor
		};
		return playerColor;
	}

	inline void DrawDashboardProgress(ImVec2 center, float radius, float thickness, int num_ticks, float tick_length, float progress, ImColor color)
	{
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		float radiu_num = 5.0f;
		float min_angle = -PI / radiu_num; // 起始角度（弧度）
		float max_angle = PI / radiu_num; // 结束角度（弧度）

		// 调整进度条半径，使其在视觉上更接近于刻度线
		float progress_radius = radius - tick_length / 2;

		// 计算进度条覆盖的弧度范围，基于进度
		float progress_min_angle = PI / radiu_num; // 进度条起始角度保持不变
		float progress_max_angle = progress_min_angle + (-PI / (radiu_num / 2)) * progress; // 根据进度调整结束角度

		// 绘制进度条
		draw_list->PathArcTo(center, progress_radius, progress_min_angle, progress_max_angle, 100); // 使用100个分段来平滑圆弧
		draw_list->PathStroke(color, 0, tick_length - 2); // 绘制进度条，颜色为红色

		// 大刻度的长度和厚度
		float major_tick_length = tick_length; // 大刻度的长度
		float major_tick_thickness = thickness; // 大刻度的厚度

		// 计算每个大刻度的位置
		int ticks_per_major_tick = num_ticks / 4; // 因为需要在起始和结束位置之间均匀分布4个大刻度，所以我们将总刻度数分成3段

		for (int i = 0; i <= num_ticks; ++i) {
			float fraction = (float)i / (float)num_ticks;
			float angle = min_angle + (max_angle - min_angle) * fraction;

			// 判断当前刻度是否为大刻度的位置
			bool is_major_tick = i % ticks_per_major_tick == 0 || i == num_ticks;
			float current_tick_length = is_major_tick ? major_tick_length : tick_length;
			float current_thickness = is_major_tick ? major_tick_thickness : thickness;

			// 刻度的起点和终点
			ImVec2 tick_start(
				center.x + cos(angle) * (radius - current_tick_length),
				center.y + sin(angle) * (radius - current_tick_length)
			);
			ImVec2 tick_end(
				center.x + cos(angle) * radius,
				center.y + sin(angle) * radius
			);

			// 绘制刻度线
			draw_list->AddLine(tick_start, tick_end, is_major_tick ? IM_COL32(0, 229, 255, 255) : IM_COL32(0, 150, 200, 100), current_thickness);
		}

		// 绘制右半圆形的外围轮廓
		draw_list->PathArcTo(center, radius, min_angle, max_angle, num_ticks);
		draw_list->PathStroke(IM_COL32(0, 200, 240, 180), 0, thickness);
	}

	inline void DrawInvertedTriangle(ImVec2 position, float size, ImColor color) {
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		auto DrawList = ImGui::GetBackgroundDrawList();

		ImVec2 p1 = ImVec2(position.x, position.y + size / 2);
		ImVec2 p2 = ImVec2(position.x + size / 2, position.y - size / 2);
		ImVec2 p3 = ImVec2(position.x - size / 2, position.y - size / 2);

		draw_list->AddTriangle(p1, p2, p3, ImColor(8, 12, 20, 220), 1.0f);

		draw_list->AddTriangleFilled(p1, p2, p3, color);
	};

	inline void window_texture(const ImVec2& position, const ImVec2& size, const ImTextureID& texture, const ImColor& color = { 255,255,255 })
	{
		auto DrawList = ImGui::GetBackgroundDrawList();
		DrawList->AddImage(texture, { position.x, position.y }, { position.x + size.x, position.y + size.y }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, color);
	}
	inline void Line(ImVec2 Pos1, ImVec2 Pos2, ImU32 Color, float Thickness)
	{
		ImGui::GetForegroundDrawList()->AddLine({ Pos1.x, Pos1.y }, { Pos2.x, Pos2.y }, Color, Thickness);
	}

	void AddImageRotated(ImTextureID tex_id, ImVec2 center, ImVec2 size, float angle) {
		ImDrawList* DrawList;

		DrawList = ImGui::GetBackgroundDrawList();

		float sin_a = sinf(angle), cos_a = cosf(angle);
		ImVec2 pos[4] = {
			ImRotate(ImVec2(-size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
			ImRotate(ImVec2(+size.x * 0.5f, -size.y * 0.5f), cos_a, sin_a),
			ImRotate(ImVec2(+size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a),
			ImRotate(ImVec2(-size.x * 0.5f, +size.y * 0.5f), cos_a, sin_a)
		};

		for (int n = 0; n < 4; n++) {
			pos[n].x += center.x;
			pos[n].y += center.y;
		}

		DrawList->AddImageQuad(tex_id, pos[0], pos[1], pos[2], pos[3]);
	}

	inline void Image(const ImTextureID& Texture, ImVec2 Pos, ImVec2 Size, bool UseColorOverlay = false, const ImU32& Color = IM_COL32(255, 255, 255, 255))
	{
		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

		Size = ImVec2(Pos.x + Size.x, Pos.y + Size.y);
		if (UseColorOverlay)
		{
			DrawList->AddImage((void*)Texture, Pos, Size, ImVec2(0, 0), ImVec2(1, 1), Color);
		}
		else {
			DrawList->AddImage((void*)Texture, Pos, Size);
		}
	}
	inline void Circle(ImVec2 Pos, float Radius, ImColor Color, float Thickness, int Num)
	{
		auto DrawList = ImGui::GetForegroundDrawList();

		DrawList->AddCircle(Pos, Radius, Color, Num, Thickness);
	}

	inline void CircleFilled(ImVec2 Pos, float Radius, ImColor Color, int Num)
	{
		auto DrawList = ImGui::GetForegroundDrawList();

		DrawList->AddCircleFilled(Pos, Radius, Color, Num);
	}

	inline void window_filled_rect(const ImVec2& position, const ImVec2& size, const ImColor& color, const float& rounding = 0.0f, const ImDrawFlags_& flags = ImDrawFlags_RoundCornersNone)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();

		DrawList->AddRectFilled({ position.x, position.y }, { position.x + size.x, position.y + size.y }, color, rounding, flags);
	}
	inline void window_filled_rect_multicolor_horizontal(const ImVec2& position, const ImVec2& size, const ImColor& color1, const ImColor& color2)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();

		DrawList->AddRectFilledMultiColor({ position.x, position.y }, { position.x + size.x, position.y + size.y }, color1, color2, color2, color1);
	}
	inline ImVec2 OSText(const char* Text, ImVec2 Pos, ImColor Color, float FontSize, bool Centered, bool AdjustHeight)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();

		if (!Centered && !AdjustHeight)
		{
			const ImVec2 TextSize = font::myth_bold->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text);
			DrawList->AddText(font::myth_bold, FontSize, { Pos.x, Pos.y }, Color, Text);
			return TextSize;
		}
		else
		{
			float TextWidth = font::myth_bold->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text).x;
			const ImVec2 TextSize = font::myth_bold->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text);

			const auto HorizontalOffset = Centered ? TextSize.x / 2 : 0.0f;
			const auto VerticalOffset = AdjustHeight ? TextSize.y : 0.0f;

			ImVec2 Pos_ = { Pos.x - HorizontalOffset, Pos.y - VerticalOffset };
			DrawList->AddText(font::myth_bold, FontSize, Pos_, Color, Text);
			return TextSize;
		}
	}

	inline ImVec2 StrokeText(const char* Text, ImVec2 Pos, ImColor Color, float FontSize, bool Centered = true, bool AdjustHeight = true)
	{
		if (GameData.Config.ESP.Stroke)
		{
			ImColor shadowCol = ImColor(8, 12, 20, 220);
			OSText(Text, ImVec2(Pos.x - 1, Pos.y + 1), shadowCol, FontSize, Centered, AdjustHeight);
			OSText(Text, ImVec2(Pos.x - 1, Pos.y - 1), shadowCol, FontSize, Centered, AdjustHeight);
			OSText(Text, ImVec2(Pos.x + 1, Pos.y + 1), shadowCol, FontSize, Centered, AdjustHeight);
			OSText(Text, ImVec2(Pos.x + 1, Pos.y - 1), shadowCol, FontSize, Centered, AdjustHeight);
		}
		return OSText(Text, Pos, Color, FontSize, Centered, AdjustHeight);
	}

	inline void DrawTriangle(ImVec2& p1, ImVec2& p2, ImVec2& p3, ImU32 col, bool filled = false) {
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		if (filled) {
			draw_list->AddTriangleFilled(p1, p2, p3, col);
		}
		else {
			draw_list->AddTriangle(p1, p2, p3, col);
		}
	}
	inline void DrawRadarPlayerCircleWithText(const char* text,ImU32 circle_col,float radius,ImVec2 position,float angle,ECharacterIconType RadarState,bool bIsTeamID = false)
	{
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		const float fontSize = radius * 1.5f;
		const ImVec2 textSize = font::myth_bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text);
		ImVec2 textPos = ImVec2(
			position.x - textSize.x / 2,
			position.y - textSize.y / 2
		);

		// 如果不是绘制队标，则显示箭头图标
		if (!bIsTeamID) {
			std::string IconUrl = "Assets/image/Map/arrow.png";
			if (GImGuiTextureMap[IconUrl].Width > 0) {
				ImVec2 IconPos = ImVec2(
					position.x,
					position.y
				);
				float IconSize = radius * 10.0f;
				RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, IconPos, ImVec2(IconSize, IconSize), angle);
			}
		}

		draw_list->AddCircleFilled(position, radius, circle_col, radius * 2);
		draw_list->AddCircle(position, radius + 1, ImColor(0, 180, 220, 200), (radius + 1) * 2, 1.5);

		if (RadarState != ECharacterIconType::Normal)
		{
			std::string RadarStateIconUrl = "";
			switch (RadarState)
			{
			case ECharacterIconType::Quitter:
				RadarStateIconUrl = "Assets/image/Map/indicator_onscreen_status_disconnect.png";
				break;
			case ECharacterIconType::Groggy:
				RadarStateIconUrl = "Assets/image/Map/indicator_onscreen_status_DBNO.png";
				break;
			case ECharacterIconType::Parachute:
				RadarStateIconUrl = "Assets/image/Map/indicator_onscreen_status_parachute.png";
				break;
			default:
				break;
			}

			if (GImGuiTextureMap[RadarStateIconUrl].Width > 0) {
				ImVec2 IconPos = ImVec2(
					position.x,
					position.y
				);
				float IconSize = radius * 7.3f;
				if (RadarState == ECharacterIconType::Quitter) IconSize = radius * 4.f;
				RenderHelper::AddImageRotated(GImGuiTextureMap[RadarStateIconUrl].Texture, IconPos, ImVec2(IconSize, IconSize), 0);
			}
			else {
				draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y + 1), ImColor(45, 45, 45, 220), text);
				draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y - 1), ImColor(45, 45, 45, 220), text);
				draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), ImColor(45, 45, 45, 220), text);
				draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y - 1), ImColor(45, 45, 45, 220), text);
				draw_list->AddText(font::myth_bold, fontSize, textPos, IM_COL32_WHITE, text);
			}
		}
		else {
			ImColor shadowColor = ImColor(8, 12, 20, 220);
			draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y + 1), shadowColor, text);
			draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y - 1), shadowColor, text);
			draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), shadowColor, text);
			draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y - 1), shadowColor, text);
			draw_list->AddText(font::myth_bold, fontSize, textPos, IM_COL32_WHITE, text);
		}
	}
	inline void DrawRadarPlayerCircleWithText(const char* text, ImU32 circle_col, float radius, ImVec2 position, float angle) {
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		const float fontSize = radius * 1.5f;

		const ImVec2 textSize = font::myth_bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text);

		ImVec2 textPos = ImVec2(
			position.x - textSize.x / 2,
			position.y - textSize.y / 2
		);

		std::string IconUrl = "Assets/image/arrow.png";
		if (GImGuiTextureMap[IconUrl].Width > 0) {
			ImVec2 IconPos = ImVec2(
				position.x,
				position.y
			);
			float IconSize = radius * 7.0f;
			RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, IconPos, ImVec2(IconSize, IconSize), angle);
		}

		draw_list->AddCircleFilled(position, radius, circle_col, radius * 2);
		draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y + 1), ImColor(45, 45, 45, 220), text);
		draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x - 1, textPos.y - 1), ImColor(45, 45, 45, 220), text);
		draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), ImColor(45, 45, 45, 220), text);
		draw_list->AddText(font::myth_bold, fontSize, ImVec2(textPos.x + 1, textPos.y - 1), ImColor(45, 45, 45, 220), text);
		draw_list->AddText(font::myth_bold, fontSize, textPos, IM_COL32_WHITE, text);
	}

	inline ImVec2 Text(const char* text, ImVec2 pos, ImU32 color = IM_COL32(255, 255, 255, 255), int size = 14, bool centered = true, bool adjustHeight = true) noexcept
	{
		//const auto textSize = ImGui::CalcTextSize(text);
		const ImVec2 textSize = font::myth_bold->CalcTextSizeA(size, FLT_MAX, 0.f, text);

		const auto horizontalOffset = centered ? textSize.x / 2 : 0.0f;
		const auto verticalOffset = adjustHeight ? textSize.y : 0.0f;

		unsigned int uintColor = color;
		unsigned int alphaOnlyColor = uintColor & IM_COL32_A_MASK;

		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

		DrawList->AddText(font::myth_bold, size, { pos.x - horizontalOffset + 1.0f, pos.y - verticalOffset + 1.0f }, uintColor & IM_COL32_A_MASK, text);
		DrawList->AddText(font::myth_bold, size, { pos.x - horizontalOffset, pos.y - verticalOffset }, color, text);

		return textSize;
	}
	void window_text(const ImVec2& position, const float& font_size, const ImColor& color, const char* text, const ImColor& color_outline = { 0, 0, 0 }, const bool& outline = true)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();
		if (outline)
		{
			DrawList->AddText(font::myth_bold, font_size, { position.x + 1.0f, position.y + 1.0f }, ImColor{ color_outline.Value.x, color_outline.Value.y, color_outline.Value.z, color.Value.w }, text);
			DrawList->AddText(font::myth_bold, font_size, { position.x - 1.0f, position.y - 1.0f }, ImColor{ color_outline.Value.x, color_outline.Value.y, color_outline.Value.z, color.Value.w }, text);
			DrawList->AddText(font::myth_bold, font_size, { position.x + 1.0f, position.y - 1.0f }, ImColor{ color_outline.Value.x, color_outline.Value.y, color_outline.Value.z, color.Value.w }, text);
			DrawList->AddText(font::myth_bold, font_size, { position.x - 1.0f, position.y + 1.0f }, ImColor{ color_outline.Value.x, color_outline.Value.y, color_outline.Value.z, color.Value.w }, text);
		}

		DrawList->AddText(font::myth_bold, font_size, { position.x, position.y }, color, text);
	}
	ImVec2 get_text_size(const float& font_size, const char* text)
	{
		return font::myth_bold->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text);
	}
	inline void window_draw_labels(const ImVec2& position, const float& font_size, const ImColor& text_color, ImVector<std::string> labels, const float& max_width, const float& padding = 2.0f, const float& rounding = 3.0f)
	{
		ImVec2 offset;

		const auto fixed_position = ImVec2{ position.x - max_width / 2.0f, position.y };

		for (const auto label : labels)
		{
			const auto text_size = get_text_size(font_size, label.c_str());
			const auto rect_size = ImVec2{ text_size.x + padding * 2.0f, text_size.y + padding * 2.0f };
			const auto rect_pos = ImVec2{ fixed_position.x + offset.x, fixed_position.y + offset.y };

			window_text({ rect_pos.x + padding, rect_pos.y + padding }, font_size, text_color, label.c_str());
			offset.x += rect_size.x + padding;

			if (offset.x > max_width)
			{
				offset.x = 0.0f;
				offset.y += rect_size.y + padding;
			}
		}
	}
	inline ImVec2 window_draw_label(const ImVec2& position, const float& font_size, const ImColor& text_color, const char* text, const float& padding = 3.0f, const float& rounding = 3.0f)
	{
		const auto text_size = get_text_size(font_size, text);
		const auto rect_size = ImVec2{ text_size.x + padding * 2.0f, text_size.y + padding * 2.0f };
		const auto rect_pos = ImVec2{ position.x - rect_size.x / 2.0f, position.y - rect_size.y / 2.0f };
		window_text({ rect_pos.x + padding, rect_pos.y + padding }, font_size, text_color, text);

		return rect_size;
	}
	void window_rect(const ImVec2& position, const ImVec2& size, const ImColor& color, const float& thickness, const float& rounding)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();

		DrawList->AddRect({ position.x, position.y }, { position.x + size.x, position.y + size.y }, color, rounding, 15, thickness);
	}
	inline void window_box(const ImVec2& position, const ImVec2& size, const ImColor& color, const float& thickness = 1.0f, const float& rounding = 0.0f)
	{
		window_rect(position, size, color, thickness, rounding);
	}
	inline void window_line(const ImVec2& position_start, const ImVec2& position_end, const ImColor& color, const float& thickness = 1.0f)
	{
		auto DrawList = ImGui::GetBackgroundDrawList();

		DrawList->AddLine({ position_start.x, position_start.y }, { position_end.x, position_end.y }, color, thickness);
	}
	inline void AddLineGradient(ImVec2 p1, ImVec2 p2, ImU32 col1, ImU32 col2, float thickness)
	{
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
		const int steps = 60;
		for (int i = 0; i < steps; ++i) {
			float t0 = (float)i / (float)steps;
			float t1 = (float)(i + 1) / (float)steps;
			ImVec2 s = ImVec2(p1.x + (p2.x - p1.x) * t0, p1.y + (p2.y - p1.y) * t0);
			ImVec2 e = ImVec2(p1.x + (p2.x - p1.x) * t1, p1.y + (p2.y - p1.y) * t1);
			float a = t0;
			int r = (int)((((col2 >> IM_COL32_R_SHIFT) & 0xFF) - ((col1 >> IM_COL32_R_SHIFT) & 0xFF)) * a + ((col1 >> IM_COL32_R_SHIFT) & 0xFF));
			int g = (int)((((col2 >> IM_COL32_G_SHIFT) & 0xFF) - ((col1 >> IM_COL32_G_SHIFT) & 0xFF)) * a + ((col1 >> IM_COL32_G_SHIFT) & 0xFF));
			int b = (int)((((col2 >> IM_COL32_B_SHIFT) & 0xFF) - ((col1 >> IM_COL32_B_SHIFT) & 0xFF)) * a + ((col1 >> IM_COL32_B_SHIFT) & 0xFF));
			int aa = (int)((((col2 >> IM_COL32_A_SHIFT) & 0xFF) - ((col1 >> IM_COL32_A_SHIFT) & 0xFF)) * a + ((col1 >> IM_COL32_A_SHIFT) & 0xFF));
			ImU32 cc = IM_COL32(r, g, b, aa);
			draw_list->AddLine(s, e, cc, thickness);
		}
	}

	inline void DrawExplosiveRange(FVector center, float radius, ImColor color, float thickness) {
		auto DrawList = ImGui::GetBackgroundDrawList();
		FVector Vertices[46];
		bool VerticesValid[46]{};
		float angle = 0;
		const float angleStep = (M_PI * 2) / 45;

		for (int i = 0; i <= 45; ++i, angle += angleStep)
		{
			FVector pos = FVector(radius * cosf(angle) + center.X, radius * sinf(angle) + center.Y, center.Z);
			FVector2D w2sPos = VectorHelper::WorldToScreen(pos);
			VerticesValid[i] = w2sPos.X > 0 && w2sPos.X < GameData.Config.Overlay.ScreenWidth && w2sPos.Y > 0 && w2sPos.Y < GameData.Config.Overlay.ScreenHeight;
			Vertices[i] = FVector(w2sPos.X, w2sPos.Y, 0);
		}

		for (int i = 0; i < 45; ++i) {
			if (Vertices[i].Distance(Vertices[i + 1]) < radius)
				if ((VerticesValid[i] && VerticesValid[i + 1]))
					DrawList->AddLine({ Vertices[i].X, Vertices[i].Y }, { Vertices[i + 1].X,  Vertices[i + 1].Y }, color, thickness);
		}
	}
};
