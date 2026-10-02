#pragma once
// NEON CYBER 主题 -- 深空黑底 + 电光青/霓虹紫 + 顶部导航栏
#include "imgui_settings.h"
#include <Common/Data.h>

namespace MenuTheme {

    // ---------- 布局 ----------
    inline ImVec2 WindowSize       = ImVec2(1760.f, 1080.f);
    inline float  TitleHeight      = 36.f;    // 顶部状态栏
    inline float  TopNavHeight     = 44.f;    // 顶部导航标签行
    inline float  SubTabRowHeight  = 36.f;    // 子标签行
    inline float  WeaponTabRowHeight = 50.f;  // 武器标签行
    inline float  FooterHeight     = 30.f;    // 底部状态栏
    inline float  SidebarWidth     = 350.f;   // 右侧设置栏
    inline float  PanelHeaderHeight = 28.f;
    inline float  ContentTopGap    = 10.f;
    inline float  ItemSpacing      = 10.f;
    inline float  WindowRounding   = 6.f;
    inline float  ColumnGap        = 8.f;

    // ---------- 面板按内容估高 ----------
    inline float ItemRowH() {
        return ImMax(26.f, ImGui::GetFontSize() + 10.f);
    }

    inline float PanelBodyH(int itemCount, int fullWidthRows, float panelWidth) {
        const int cols = (panelWidth > 340.f) ? 2 : 1;
        const int gridRows = ImMax(1, (itemCount + cols - 1) / cols);
        const float sp = 5.f;
        const float pad = 14.f;
        const float rowH = ItemRowH();
        return pad + (float)gridRows * rowH + (float)ImMax(0, gridRows - 1) * sp
            + (float)fullWidthRows * (rowH + sp);
    }

    inline float StyledPanelH(float width, int itemCount, int fullWidthRows = 0) {
        return PanelHeaderHeight + PanelBodyH(itemCount, fullWidthRows, width);
    }

    inline ImVec2 StyledPanel(float width, int itemCount, int fullWidthRows = 0) {
        return ImVec2(width, StyledPanelH(width, itemCount, fullWidthRows));
    }

    inline ImGuiWindowFlags PanelFlags() {
        return ImGuiWindowFlags_AlwaysVerticalScrollbar;
    }

    inline ImGuiWindowFlags ScrollPanelFlags() {
        return ImGuiWindowFlags_AlwaysVerticalScrollbar;
    }

    // ---------- NEON CYBER 配色 ----------
    // #080C14 深空背景 | #00E5FF 电光青 | #B44CFF 霓虹紫 | #FF8C00 琥珀
    inline ImColor Accent          = ImColor(0, 229, 255, 255);       // 电光青 #00E5FF
    inline ImColor AccentHover     = ImColor(60, 240, 255, 255);
    inline ImColor AccentLow       = ImColor(0, 180, 210, 100);
    inline ImColor AccentPurple    = ImColor(180, 76, 255, 255);      // 霓虹紫 #B44CFF
    inline ImColor AccentAmber     = ImColor(255, 140, 0, 255);        // 琥珀 #FF8C00
    inline ImColor PageBg          = ImColor(8, 12, 20, 255);          // 深空 #080C14
    inline ImColor PanelBg         = ImColor(20, 24, 36, 255);         // 面板 #141824         // 面板 #0C101C
    inline ImColor PanelBgLight    = ImColor(22, 26, 42, 255);         // 面板亮 #121624
    inline ImColor InputBg         = ImColor(14, 18, 28, 255);         // 输入框 #0A0E16
    inline ImColor Border          = ImColor(0, 180, 210, 220);        // 边框青（亮+高Alpha）
    inline ImColor BorderGlow      = ImColor(0, 200, 230, 50);         // 边框辉光
    inline ImColor BorderPurple    = ImColor(160, 60, 240, 180);       // 紫色边框
    inline ImColor TextBright      = ImColor(240, 242, 250, 255);      // 亮白文字(未选中状态)
    inline ImColor TextMuted       = ImColor(180, 190, 210, 255);      // 暗文字(未选中状态)
    inline ImColor TextDark        = ImColor(60, 70, 90, 255);         // 深色文字(选中/开启状态)
    inline ImColor TextCyan        = ImColor(0, 229, 255, 255);        // 青色文字
    inline ImColor TextOnAccent    = ImColor(8, 12, 20, 255);          // 强调色上文字
    inline ImColor ToggleOff       = ImColor(45, 55, 80, 255);         // 开关关(明显可见)
    inline ImColor ToggleOn        = ImColor(0, 229, 255, 255);        // 开关开
    inline ImColor DividerColor   = ImColor(0, 140, 180, 60);          // 分割线

    // 内容区域起始Y（顶部导航在标题栏下面）
    inline float ContentStartY(bool hasSubTabs = false) {
        return TitleHeight + TopNavHeight + (hasSubTabs ? SubTabRowHeight : 0.f) + ContentTopGap;
    }

    inline ImVec2 ContentOrigin(bool hasSubTabs = false) {
        return ImVec2(12.f, ContentStartY(hasSubTabs));
    }

    inline ImVec2 ContentSize(const ImVec2& region, bool hasSubTabs = false) {
        return ImVec2(
            region.x - 24.f - SidebarWidth - 8.f,
            region.y - ContentStartY(hasSubTabs) - FooterHeight - 4.f);
    }

    inline ImVec2 ColumnSizeFromContent(const ImVec2& content, int columns = 3) {
        const float w = (content.x - ColumnGap * (float)(columns - 1)) / (float)columns;
        return ImVec2(w, content.y);
    }

    inline ImVec2 ColumnSize(const ImVec2& region, int columns = 3, bool hasSubTabs = false) {
        return ColumnSizeFromContent(ContentSize(region, hasSubTabs), columns);
    }

    inline ImVec2 AimbotContentSize(const ImVec2& region) {
        const ImVec2 content = ContentSize(region, false);
        return ImVec2(content.x, content.y - WeaponTabRowHeight);
    }

    inline ImVec2 AnimatedContentPos(const ImVec2& region, float tab_alpha, float anim, bool hasSubTabs = false, float baseY = -1.f) {
        const float y = baseY < 0.f ? ContentStartY(hasSubTabs) : baseY;
        (void)tab_alpha;
        (void)anim;
        return ImVec2(12.f, y);
    }

    inline ImVec2 StackPanel(const ImVec2& col, float totalH, float ratio) {
        return ImVec2(col.x, totalH * ratio);
    }

    inline ImVec2 StackRemain(const ImVec2& col, float totalH, float usedTop, int gaps = 1) {
        return ImVec2(col.x, totalH - usedTop - ColumnGap * (float)gaps);
    }

    inline float StackH(float totalH, float ratio) {
        return totalH * ratio;
    }

    inline float UiFontScale(float uiFontSize) {
        return uiFontSize / 18.f;
    }

    inline void ApplyUiScale(ImGuiStyle* style, float uiFontSize) {
        const int idx = font::MenuFontIndex(uiFontSize);
        const int fontIdx = font::menu_ui_sizes[1] ? 1 : idx;
        if (font::menu_ui_sizes[fontIdx]) {
            font::menu_ui = font::menu_ui_sizes[fontIdx];
            font::menu_ui_bold = font::menu_header_sizes[fontIdx] ? font::menu_header_sizes[fontIdx] : font::menu_ui;
            if (font::menu_ui_bold) font::calibri_bold = font::menu_ui_bold;
            if (font::menu_ui) {
                font::calibri_regular = font::menu_ui;
                ImGui::GetIO().FontDefault = font::menu_ui;
            }
        }
        ImGui::GetIO().FontGlobalScale = 1.f;
        const float px = 16.f + idx * 2.f;
        const float s = px / 18.f;
        style->ItemSpacing = ImVec2(ItemSpacing * s, ItemSpacing * 0.85f * s);
        style->FramePadding = ImVec2(6.f * s, 5.f * s);
        style->ScrollbarSize = 8.f * s;
    }

    inline void ResetUiScale(ImGuiStyle* style) {
        ImGui::GetIO().FontGlobalScale = 1.f;
        style->ItemSpacing = ImVec2(ItemSpacing, ItemSpacing);
        style->FramePadding = ImVec2(4.f, 3.f);
    }

    inline void BeginFullWidthRow() {
        ImGui::Columns(1);
    }

    inline void ApplyPluto() {
        c::accent = ImVec4(Accent.Value.x / 255.f, Accent.Value.y / 255.f, Accent.Value.z / 255.f, 1.f);
        c::accent1 = c::accent;
        c::accent_low = ImVec4(AccentLow.Value.x / 255.f, AccentLow.Value.y / 255.f, AccentLow.Value.z / 255.f, AccentLow.Value.w / 255.f);
        c::bg::size = WindowSize;
        c::bg::rounding = WindowRounding;

        c::shadow = ImColor(0, 180, 230, 15);
        c::bg::background = PageBg;
        c::bg::outline = Border;
        c::bg::top_bg = Accent;
        c::child::background = PanelBg;
        c::child::border = Border;
        c::child::lines = DividerColor;
        c::child::rounding = 4.f;

        c::checkbox::background = ToggleOff;
        c::checkbox::outline = Border;
        c::checkbox::mark = Accent;
        c::checkbox::rounding = 8.f;

        c::slider::background = ImColor(20, 26, 40, 255);
        c::button::background = ImColor(35, 45, 65, 255);
        c::button::outline = AccentHover;
        c::combo::background = ImColor(25, 32, 48, 255);
        c::combo::outline = ImColor(0, 210, 240, 255);
        c::keybind::background = ImColor(40, 55, 80, 255);
        c::input::background = InputBg;
        c::input::outline = Border;
        c::picker::background = InputBg;
        c::tabs::line = DividerColor;

        c::text::text_active = TextDark;
        c::text::text_hov = ImColor(180, 190, 210, 255);
        c::text::text = TextMuted;
        c::text::text_active1 = TextDark;
        c::text::text1 = TextMuted;

        c::scrollbar::bar_active = Accent;
        c::scrollbar::bar_hov = AccentHover;
        c::scrollbar::bar = ImColor(0, 100, 140, 150);
        c::popup_elements::filling = ImColor(0, 0, 0, 240);
    }

    inline ImU32 ToU32(const ImColor& c) {
        return ImGui::ColorConvertFloat4ToU32(ImVec4(c.Value.x, c.Value.y, c.Value.z, c.Value.w));
    }

    // ---- NEON CYBER Chrome ----
    inline void DrawChrome(ImDrawList* dl, const ImVec2& pos, const ImVec2& region) {
        // 整体背景
        dl->AddRectFilled(pos, pos + region, ToU32(PageBg), WindowRounding);

        // 顶部标题栏 - 科技感扫描线
        dl->AddRectFilled(pos, pos + ImVec2(region.x, TitleHeight), ToU32(ImColor(10, 15, 24, 255)), WindowRounding, ImDrawFlags_RoundCornersTop);
        // 顶栏底部辉光线
        dl->AddRectFilled(pos + ImVec2(0.f, TitleHeight - 1.f), pos + ImVec2(region.x, TitleHeight),
            ToU32(ImColor(0, 160, 200, 80)));

        // 标题文字
        if (font::menu_ui_bold) {
            const float titleSize = font::menu_ui_bold->FontSize;
            dl->AddText(font::menu_ui_bold, titleSize, pos + ImVec2(16.f, (TitleHeight - titleSize) * 0.5f),
                ToU32(TextCyan), U8("// N E O N . C Y B E R  |  PUBG  |  \u54D2\u54D2\u54D2\u516C\u76CA"));
        } else {
            dl->AddText(pos + ImVec2(16.f, 8.f), ToU32(TextCyan), U8("// N E O N . C Y B E R  |  PUBG  |  \u54D2\u54D2\u54D2\u516C\u76CA"));
        }

        // 顶部导航栏背景
        dl->AddRectFilled(
            pos + ImVec2(0.f, TitleHeight),
            pos + ImVec2(region.x, TitleHeight + TopNavHeight),
            ToU32(ImColor(6, 10, 18, 240)));

        // 导航栏底部分割辉光
        dl->AddRectFilled(
            pos + ImVec2(0.f, TitleHeight + TopNavHeight),
            pos + ImVec2(region.x, TitleHeight + TopNavHeight + 1.f),
            ToU32(ImColor(0, 120, 160, 100)));

        // 四个角装饰线 (左上/右上/左下/右下)
        const float cornerLen = 16.f;
        const float cornerThick = 2.f;
        const ImU32 cornerCol = ToU32(ImColor(0, 180, 220, 120));
        // 左上
        dl->AddLine(pos, pos + ImVec2(cornerLen, 0), cornerCol, cornerThick);
        dl->AddLine(pos, pos + ImVec2(0, cornerLen), cornerCol, cornerThick);
        // 右上
        dl->AddLine(pos + ImVec2(region.x - cornerLen, 0), pos + ImVec2(region.x, 0), cornerCol, cornerThick);
        dl->AddLine(pos + ImVec2(region.x, 0), pos + ImVec2(region.x, cornerLen), cornerCol, cornerThick);
        // 左下
        dl->AddLine(pos + ImVec2(0, region.y - cornerLen), pos + ImVec2(0, region.y), cornerCol, cornerThick);
        dl->AddLine(pos, pos + ImVec2(region.x, region.y), cornerCol, cornerThick);
        dl->AddLine(pos + ImVec2(cornerLen, region.y), pos + ImVec2(0, region.y), cornerCol, cornerThick);
        // 右下
        dl->AddLine(pos + ImVec2(region.x - cornerLen, region.y), pos + ImVec2(region.x, region.y), cornerCol, cornerThick);

        // 底部状态栏
        dl->AddRectFilled(
            pos + ImVec2(0.f, region.y - FooterHeight),
            pos + ImVec2(region.x, region.y),
            ToU32(ImColor(10, 15, 24, 255)), WindowRounding, ImDrawFlags_RoundCornersBottom);
        dl->AddRectFilled(
            pos + ImVec2(0.f, region.y - FooterHeight),
            pos + ImVec2(region.x, region.y - FooterHeight + 1.f),
            ToU32(ImColor(0, 140, 180, 80)));

        // 底部信息 - 终端风格
        char footer[512];
        const char* mapName = GameData.MapName.empty() ? "N/A" : GameData.MapName.c_str();
        snprintf(footer, sizeof(footer), U8("> DMA:%s  |  KEY:ACTIVE  |  MAP:%s  |  CFG:%d  |  TEAM:%d  |  VER:%s"),
            GameData.PID != 0 ? U8("ONLINE") : U8("OFFLINE"),
            mapName,
            GameData.Config.AimBot.ConfigIndex + 1,
            GameData.LocalPlayerTeamID,
            GameData.Version.c_str());
        if (font::menu_ui) {
            const float footerSize = ImMax(12.f, font::menu_ui->FontSize - 3.f);
            dl->AddText(font::menu_ui, footerSize, pos + ImVec2(16.f, region.y - FooterHeight + 7.f), ToU32(ImColor(0, 200, 140, 220)), footer);
        } else {
            dl->AddText(pos + ImVec2(16.f, region.y - FooterHeight + 8.f), ToU32(ImColor(0, 200, 140, 220)), footer);
        }
    }

} // namespace MenuTheme
