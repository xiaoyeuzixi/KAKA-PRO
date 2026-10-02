#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <atomic>
#include "Texture.h"
#include "Utils/Utils.h"

// ============================================
// 性能优化缓存系统
// 目的：减少重复计算，提高渲染性能
// ============================================

namespace RenderCache {

    // 全局帧计数器（替代GameData.FrameCount）
    extern uint64_t GlobalFrameCount;

    // 更新全局帧计数
    inline void UpdateFrameCount() {
        GlobalFrameCount++;
    }

    // ============================================
    // 1. 颜色转换缓存
    // ============================================
    struct CachedPlayerColor {
        ImColor infoUseColor;
        ImColor skeletonUseColor;
        bool isUseTeamNumberColor;
        ImColor teamNumberColor;
        uint64_t lastUpdateFrame;
    };

    // 颜色缓存
    extern std::unordered_map<uint64_t, CachedPlayerColor> playerColorCache;
    extern uint64_t lastConfigVersion;

    // 清除颜色缓存
    inline void ClearColorCache() {
        playerColorCache.clear();
    }

    // 检查配置是否变化
    inline bool ShouldUpdateColorCache() {
        uint64_t currentConfigVersion = reinterpret_cast<uint64_t>(&GameData.Config);
        if (lastConfigVersion != currentConfigVersion) {
            lastConfigVersion = currentConfigVersion;
            return true;
        }
        return false;
    }

    // 缓存玩家颜色 (简化版本,不依赖RenderHelper)
    inline CachedPlayerColor* GetPlayerColorCached(uint64_t entityID) {
        auto it = playerColorCache.find(entityID);
        if (it != playerColorCache.end()) {
            return &it->second;
        }

        // 新增缓存条目
        auto& cache = playerColorCache[entityID];
        cache.lastUpdateFrame = GlobalFrameCount;
        return &cache;
    }

    // ============================================
    // 2. 文本尺寸缓存
    // ============================================
    struct TextSizeCacheEntry {
        char text[128];
        float size;
        uint32_t fontSize;
        uint64_t lastUseFrame;
    };

    extern TextSizeCacheEntry textSizeCache[256];
    extern int textSizeCacheIndex;

    inline ImVec2 GetTextSizeCached(const char* text, float fontSize) {
        // 快速查找缓存
        for (int i = 0; i < 256; i++) {
            auto& entry = textSizeCache[i];
            if (entry.lastUseFrame == GlobalFrameCount &&
                entry.fontSize == fontSize &&
                strcmp(entry.text, text) == 0) {
                return ImVec2(entry.size, 0); // 注意：这里简化了
            }
        }

        // 计算新尺寸
        ImVec2 size = font::myth_bold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text);

        // 存入缓存 (LRU)
        auto& entry = textSizeCache[textSizeCacheIndex];
        strncpy_s(entry.text, text, 127);
        entry.text[127] = '\0';
        entry.size = size.x;
        entry.fontSize = static_cast<uint32_t>(fontSize);
        entry.lastUseFrame = GlobalFrameCount;
        textSizeCacheIndex = (textSizeCacheIndex + 1) % 256;

        return size;
    }

    // ============================================
    // 3. 纹理信息缓存
    // ============================================
    struct TextureInfoCache {
        ID3D11ShaderResourceView* texture;
        float width;
        float height;
        float scaleX;
        float scaleY;
        bool valid;
    };

    extern std::unordered_map<std::string, TextureInfoCache> textureInfoCache;

    inline TextureInfoCache* GetTextureInfoCached(const std::string& iconPath, float targetHeight = 14.f) {
        auto it = textureInfoCache.find(iconPath);
        if (it != textureInfoCache.end()) {
            return &it->second;
        }

        // 新建缓存条目
        TextureInfoCache cache = { nullptr, 0, 0, 0, 0, false };
        if (GImGuiTextureMap.count(iconPath) && GImGuiTextureMap[iconPath].Width > 0) {
            auto& tex = GImGuiTextureMap[iconPath];
            cache.texture = tex.Texture;
            cache.width = tex.Width;
            cache.height = tex.Height;
            cache.scaleX = tex.Width / targetHeight;
            cache.scaleY = tex.Height / targetHeight;
            cache.valid = true;
        }

        textureInfoCache[iconPath] = cache;
        return &textureInfoCache[iconPath];
    }

    inline void ClearTextureCache() {
        textureInfoCache.clear();
    }

    // ============================================
    // 4. 字符串缓冲区 (避免频繁的格式化)
    // ============================================
    extern thread_local char textBuffer[512];
    extern thread_local char numberBuffer[64];

    // 快速整数转字符串
    inline const char* FastIntToString(int value) {
        sprintf_s(numberBuffer, "%d", value);
        return numberBuffer;
    }

    // 快速浮点数转字符串
    inline const char* FastFloatToString(float value, int decimals = 1) {
        sprintf_s(numberBuffer, "%.*f", decimals, value);
        return numberBuffer;
    }

    // 快速字符串拼接 (距离文本)
    inline const char* GetDistanceText(float distance, const char* suffix = "M") {
        sprintf_s(textBuffer, "[%d%s]", static_cast<int>(distance), suffix);
        return textBuffer;
    }

    // ============================================
    // 5. DrawList 批量渲染器
    // ============================================
    class BatchRenderer {
    private:
        ImDrawList* drawList;
        bool isForeground;

    public:
        BatchRenderer(bool foreground = false) {
            isForeground = foreground;
            drawList = foreground ? ImGui::GetForegroundDrawList() : ImGui::GetBackgroundDrawList();
        }

        ImDrawList* GetDrawList() { return drawList; }

        // 委托绘制方法
        inline void AddLine(const ImVec2& p1, const ImVec2& p2, ImU32 col, float thickness = 1.0f) {
            drawList->AddLine(p1, p2, col, thickness);
        }

        inline void AddText(const char* text, const ImVec2& pos, ImU32 col, float fontSize = 14.f) {
            drawList->AddText(font::myth_bold, fontSize, pos, col, text);
        }

        inline void AddRect(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0, float thickness = 1.0f) {
            drawList->AddRect(p_min, p_max, col, rounding, flags, thickness);
        }

        inline void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding = 0.0f, ImDrawFlags flags = 0) {
            drawList->AddRectFilled(p_min, p_max, col, rounding, flags);
        }

        inline void AddCircle(const ImVec2& center, float radius, ImU32 col, int num_segments = 0, float thickness = 1.0f) {
            drawList->AddCircle(center, radius, col, num_segments, thickness);
        }

        inline void AddCircleFilled(const ImVec2& center, float radius, ImU32 col, int num_segments = 0) {
            drawList->AddCircleFilled(center, radius, col, num_segments);
        }

        inline void AddImage(ImTextureID user_texture_id, const ImVec2& p_min, const ImVec2& p_max,
            const ImVec2& uv_min = ImVec2(0, 0), const ImVec2& uv_max = ImVec2(1, 1),
            ImU32 col = IM_COL32(255, 255, 255, 255)) {
            drawList->AddImage(user_texture_id, p_min, p_max, uv_min, uv_max, col);
        }

        inline void AddTriangle(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col, float thickness = 1.0f) {
            drawList->AddTriangle(p1, p2, p3, col, thickness);
        }

        inline void AddTriangleFilled(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, ImU32 col) {
            drawList->AddTriangleFilled(p1, p2, p3, col);
        }
    };

    // ============================================
    // 6. LineTrace 频率控制
    // ============================================
    inline int lineTraceFrameCounter = 0;
    constexpr int LINE_TRACE_FREQUENCY = 3; // 每3帧检测一次

    inline bool ShouldDoLineTrace() {
        return (lineTraceFrameCounter++ % LINE_TRACE_FREQUENCY) == 0;
    }

    // ============================================
    // 7. 排序优化标记
    // ============================================
    struct SortedContainer {
        bool needsSort;
        uint64_t lastUpdateFrame;

        SortedContainer() : needsSort(true), lastUpdateFrame(0) {}

        void MarkNeedsUpdate() {
            needsSort = true;
            lastUpdateFrame = GlobalFrameCount;
        }

        bool NeedsSort() const {
            return needsSort;
        }

        void MarkSorted() {
            needsSort = false;
        }
    };

    // ============================================
    // 8. 坐标转换优化
    // ============================================
    struct ScreenPositionCache {
        ImVec2 position;
        float distance;
        uint64_t lastUpdateFrame;
        bool valid;
    };

    extern std::unordered_map<uint64_t, ScreenPositionCache> screenPositionCache;

    inline void ClearScreenPositionCache() {
        screenPositionCache.clear();
    }

    inline bool GetCachedScreenPosition(uint64_t entityID, const FVector& worldPos, ImVec2& outScreenPos, float maxDistance = 0.f) {
        // 如果提供了最大距离，先检查距离
        if (maxDistance > 0.f) {
            float distance = worldPos.Distance(GameData.Camera.Location) / 100.f;
            if (distance > maxDistance) {
                return false; // 超出距离，不转换
            }
        }

        auto it = screenPositionCache.find(entityID);
        if (it != screenPositionCache.end() && it->second.lastUpdateFrame == GlobalFrameCount) {
            outScreenPos = it->second.position;
            return it->second.valid;
        }

        // 执行坐标转换
        FVector2D screenPos = VectorHelper::WorldToScreen(worldPos);
        outScreenPos = ImVec2(screenPos.X, screenPos.Y);

        // 缓存结果
        screenPositionCache[entityID] = {
            outScreenPos,
            0.f,
            GlobalFrameCount,
            screenPos.X > 0 && screenPos.X < GameData.Config.Overlay.ScreenWidth &&
            screenPos.Y > 0 && screenPos.Y < GameData.Config.Overlay.ScreenHeight
        };

        return screenPositionCache[entityID].valid;
    }

    // ============================================
    // 9. 血条样式预定义 (避免重复创建)
    // ============================================
    struct HealthBarStyle {
        ImColor color1;
        ImColor color2;
        ImColor color3;
        ImColor color4;
    };

    // 预定义的血条样式
    extern HealthBarStyle RAINBOW_STYLE;
    extern HealthBarStyle SOLID_GREEN_STYLE;
    extern HealthBarStyle SOLID_RED_STYLE;

    inline void InitializeHealthBarStyles() {
        // 彩虹血条
        RAINBOW_STYLE = {
            ImColor(255, 0, 0),
            ImColor(255, 128, 0),
            ImColor(0, 255, 0),
            ImColor(0, 255, 0)
        };

        // 单色绿色血条
        SOLID_GREEN_STYLE = {
            ImColor(0, 255, 0),
            ImColor(0, 255, 0),
            ImColor(0, 255, 0),
            ImColor(0, 255, 0)
        };

        // 单色红色血条
        SOLID_RED_STYLE = {
            ImColor(255, 0, 0),
            ImColor(255, 0, 0),
            ImColor(255, 0, 0),
            ImColor(255, 0, 0)
        };
    }

    // ============================================
    // 10. 缓存清理函数
    // ============================================
    inline void ClearAllCaches() {
        ClearColorCache();
        ClearTextureCache();
        ClearScreenPositionCache();
    }

    // 每帧清理过期的缓存
    inline void CleanupExpiredCaches(uint64_t maxAgeFrames = 60) {
        uint64_t minFrame = GlobalFrameCount - maxAgeFrames;

        // 清理过期的屏幕位置缓存
        for (auto it = screenPositionCache.begin(); it != screenPositionCache.end();) {
            if (it->second.lastUpdateFrame < minFrame) {
                it = screenPositionCache.erase(it);
            } else {
                ++it;
            }
        }

        // 清理过期的玩家颜色缓存
        for (auto it = playerColorCache.begin(); it != playerColorCache.end();) {
            if (it->second.lastUpdateFrame < minFrame) {
                it = playerColorCache.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// 初始化全局变量
namespace RenderCache {
    // 全局帧计数器
    uint64_t GlobalFrameCount = 0;

    // 颜色缓存
    std::unordered_map<uint64_t, CachedPlayerColor> playerColorCache;
    uint64_t lastConfigVersion = 0;

    // 文本尺寸缓存
    TextSizeCacheEntry textSizeCache[256] = {};
    int textSizeCacheIndex = 0;

    // 纹理信息缓存
    std::unordered_map<std::string, TextureInfoCache> textureInfoCache;

    // 字符串缓冲区
    thread_local char textBuffer[512];
    thread_local char numberBuffer[64];

    // 屏幕位置缓存
    std::unordered_map<uint64_t, ScreenPositionCache> screenPositionCache;

    // 血条样式
    HealthBarStyle RAINBOW_STYLE;
    HealthBarStyle SOLID_GREEN_STYLE;
    HealthBarStyle SOLID_RED_STYLE;
}
