#pragma once

#include <windows.h>
#include <sstream>
#include <iomanip>
#include "common/Data.h"
#include "common/VectorHelper.h"
#include "RenderHelper.h"
#include <imgui/imgui.h>
#include <DMALibrary/Memory/Memory.h>
#include "Texture.h"
#include "HealthBar.h"
#include <Hack/Radar.h>
#include "Menu.h"
#include <Hack/LineTrace.h>
#include <Hack/Mortar.h>
#include "RenderCache.h"  // 添加渲染缓存系统

int GetMainScreenWidth() {
    return GetSystemMetrics(SM_CXSCREEN);
}

int GetMainScreenHeight() {
    return GetSystemMetrics(SM_CYSCREEN);
}

// 获取主屏幕的宽度和高度
int mainScreenWidth = GetMainScreenWidth();  // 假设这是一个获取主屏幕宽度的函数
int mainScreenHeight = GetMainScreenHeight();  // 假设这是一个获取主屏幕高度的函数

// 射线起点：主屏幕的顶部中间
ImVec2 screenTopCenter(mainScreenWidth / 2, 0);

std::unordered_map<uint64_t, std::vector<FVector>> GrenadeTrajectories; // 每个手雷的轨迹
static std::unordered_set<uint64_t> ExplodedGrenades;
static std::unordered_map<uint64_t, uint64_t> GrenadeUniqueIds;
static std::atomic<uint64_t> NextUniqueId = 1;

// 获取屏幕宽度
int GetScreenWidth() {
    return ImGui::GetIO().DisplaySize.x;  // 使用 ImGui 获取屏幕宽度
}

// 获取屏幕高度
int GetScreenHeight() {
    return ImGui::GetIO().DisplaySize.y;  // 使用 ImGui 获取屏幕高度
}

// 实现 DrawLine 函数
namespace RenderHelper {
    void DrawLine(const ImVec2& start, const ImVec2& end, const ImColor& color, float thickness) {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();  // 获取背景绘制列表
        if (drawList) {
            drawList->AddLine(start, end, color, thickness);  // 绘制直线
        }
    }
}

class ESP {
public:
    // 工具函数：本地化文本
    static std::string Localize(const std::string& chineseText, const std::string& englishText) {
        return Languages == 1 ? chineseText : englishText;
    }

    // 工具函数：从配置获取颜色
    static ImColor GetColorFromConfig(const float* colorConfig) {
        return ImColor(
            colorConfig[0],
            colorConfig[1],
            colorConfig[2],
            colorConfig[3]
        );
    }

    // 工具函数：绘制文本并更新位置
    static ImVec2 DisplayInfoText(const std::string& text, ImVec2& position, ImColor color, int fontSize, bool centered = true, bool updatePosition = true) {
        if (text.empty()) return ImVec2(0, 0);

        ImVec2 textSize = RenderHelper::StrokeText(
            Utils::StringToUTF8(text).c_str(),
            position,
            color,
            fontSize,
            centered,
            false
        );

        if (updatePosition) position.y += textSize.y - 3; // 更新下一项的位置
        return textSize;
    }

    // 工具函数：绘制图标
    static void DrawIcon(const std::string& iconPath, ImVec2 position, float scale, ImColor color, bool centered = true) {
        if (GImGuiTextureMap.count(iconPath) && GImGuiTextureMap[iconPath].Width > 0) {
            float targetHeight = 14.f * scale;
            float heightZoom = targetHeight / GImGuiTextureMap[iconPath].Height;
            float iconWidth = GImGuiTextureMap[iconPath].Width * heightZoom;
            float iconHeight = targetHeight;

            if (centered) position.x -= iconWidth / 2;

            RenderHelper::Image(
                GImGuiTextureMap[iconPath].Texture,
                position,
                ImVec2(iconWidth, iconHeight),
                true,
                color
            );
        }
    }

    // 工具函数：绘制进度条
    static void DrawBar(ImVec2 position, float maxWidth, float fillPercent, ImColor fillColor, float height = 4.0f) {
        float fillWidth = maxWidth * fillPercent;

        // 绘制背景
        RenderHelper::window_filled_rect(position, ImVec2(maxWidth, height), ImColor(30, 30, 30, 255));

        // 绘制填充部分
        RenderHelper::window_filled_rect(position, ImVec2(fillWidth, height), fillColor);

        // 绘制边框
        RenderHelper::window_box(position - ImVec2(1, 1), ImVec2(maxWidth + 2, height + 2), IM_COL32(28, 28, 28, 255));
    }

    // 文本构建器类
    class InfoTextBuilder {
    private:
        std::vector<std::string> components;
        std::string separator;

    public:
        InfoTextBuilder(std::string sep = " ") : separator(sep) {}

        void add(const std::string& text) {
            if (!text.empty()) components.push_back(text);
        }

        void addIf(bool condition, const std::string& text) {
            if (condition && !text.empty()) components.push_back(text);
        }

        std::string build() {
            std::string result;
            for (size_t i = 0; i < components.size(); i++) {
                if (i > 0) result += separator;
                result += components[i];
            }
            return result;
        }

        bool isEmpty() const { return components.empty(); }
    };

    // 最近点查找函数
    static std::pair<float, float> find_closest_pitch(float target_distance) {
        // Data points {Pitch: Distance}
        std::map<float, float> pitch_map = {
            {0.0f, 700.0f}, {0.5f, 699.0f}, {1.0f, 699.0f}, {1.5f, 699.0f}, {2.0f, 698.0f}, {2.5f, 697.0f}, {3.0f, 696.0f}, {3.5f, 695.0f},
            {4.0f, 693.0f}, {4.5f, 691.0f}, {5.0f, 689.0f}, {5.5f, 687.0f}, {6.0f, 685.0f},
            {6.5f, 682.0f}, {7.0f, 679.0f}, {7.5f, 676.0f}, {8.0f, 673.0f}, {8.5f, 669.0f},
            {9.0f, 666.0f}, {9.5f, 662.0f}, {10.0f, 658.0f}, {10.5f, 653.0f}, {11.0f, 649.0f},
            {11.5f, 644.0f}, {12.0f, 639.0f}, {12.5f, 634.0f}, {13.0f, 629.0f}, {13.5f, 624.0f},
            {14.0f, 618.0f}, {14.5f, 612.0f}, {15.0f, 606.0f}, {15.5f, 600.0f}, {16.0f, 593.0f},
            {16.5f, 587.0f}, {17.0f, 580.0f}, {17.5f, 573.0f}, {18.0f, 566.0f}, {18.5f, 559.0f},
            {19.0f, 551.0f}, {19.5f, 544.0f}, {20.0f, 536.0f}, {20.5f, 528.0f}, {21.0f, 520.0f},
            {21.5f, 512.0f}, {22.0f, 503.0f}, {22.5f, 495.0f}, {23.0f, 486.0f}, {23.5f, 477.0f},
            {24.0f, 468.0f}, {24.5f, 459.0f}, {25.0f, 450.0f}, {25.5f, 440.0f}, {26.0f, 431.0f},
            {26.5f, 421.0f}, {27.0f, 411.0f}, {27.5f, 401.0f}, {28.0f, 391.0f}, {28.5f, 381.0f},
            {29.0f, 371.0f}, {29.5f, 360.0f}, {30.0f, 350.0f}, {30.5f, 339.0f}, {31.0f, 328.0f},
            {31.5f, 317.0f}, {32.0f, 307.0f}, {32.5f, 295.0f}, {33.0f, 284.0f}, {33.5f, 273.0f},
            {34.0f, 262.0f}, {34.5f, 250.0f}, {35.0f, 239.0f}, {35.5f, 228.0f}, {36.0f, 216.0f},
            {36.5f, 204.0f}, {37.0f, 193.0f}, {37.5f, 181.0f}, {38.0f, 169.0f}, {38.5f, 157.0f},
            {39.0f, 145.0f}, {39.5f, 133.0f}, {40.0f, 121.0f}
        };

        float closest_pitch = 0.0f;
        float closest_distance = 0.0f;
        float min_diff = (std::numeric_limits<float>::max)();

        // 遍历所有Pitch-距离对，找到最接近目标距离的项
        for (const auto& pair : pitch_map) {
            float current_diff = std::abs(pair.second - target_distance);
            if (current_diff < min_diff) {
                min_diff = current_diff;
                closest_pitch = pair.first;
                closest_distance = pair.second;
            }
        }

        return { closest_pitch, closest_distance };
    }

    // 绘制迫击炮HUD
    static void DrawMortaring(const std::unordered_map<uint64_t, Player>& Players, const std::unordered_map<uint64_t, VehicleInfo>& Vehicles)
    {
  
        if (!GameData.Config.ESP.Enable) return;
        if (GameData.LocalPlayerInfo.PreEvalPawnState != EAnimPawnState::PS_MortarDriver) return;

        const uint64_t mortarLocationOffset = GameData.Offset["MortarLocation"];
        const uint64_t mortarRotationOffset = GameData.Offset["MortarRotation"];
        if (mortarLocationOffset == 0 || mortarRotationOffset == 0) return;

        // 性能优化的静态变量
        static uint64_t lastMortarCheck = 0;
        static uint64_t lastTargetSearch = 0;
        static uint64_t lastRotationRead = 0;
        static uint64_t lastAutoAdjust = 0;
        static uint64_t lastHorizontalAdjust = 0; // 水平调整时间戳

        static uint64_t MortarEntity = 0;
        static FRotator MortarRotation;
        static FRotator cachedCurrentRotation; // 缓存当前旋转值

        static int frameCounter = 0;
        static bool mortarActive = false;
        static bool isAdjusting = false;
        static bool isAdjustingHorizontal = false; // 水平调整状态

        // 缓存的计算结果
        static float cachedHorizontalDistance = 0.0f;
        static float cachedMortarDistance = 0.0f;
        static float cachedRecommendedPitch = 0.0f;
        static bool calculationValid = false;

        struct Target {
            FVector Location;
            FVector2D ScreenPos;
            float Distance;
            std::string Name;
            bool IsPlayer;
            uint64_t Entity;
            uint64_t lastUpdateTime;

            Target(const Player& player) :
                Location(player.Location),
                Distance(player.Distance),
                Name(player.Name),
                IsPlayer(true),
                Entity(player.Entity),
                lastUpdateTime(GetTickCount64()) {
                ScreenPos = VectorHelper::WorldToScreen(Location);
            }

            Target(const VehicleInfo& vehicle) :
                Location(vehicle.Location),
                Distance(vehicle.Distance),
                Name(vehicle.Name),
                IsPlayer(false),
                Entity(vehicle.Entity),
                lastUpdateTime(GetTickCount64()) {
                ScreenPos = VectorHelper::WorldToScreen(Location);
            }
        };

        static std::unique_ptr<Target> currentTarget = nullptr;

        frameCounter++;
        uint64_t currentTime = GetTickCount64();

        // 减少迫击炮状态检查频率：每60帧检查一次（约1秒）
        if (frameCounter % 60 == 0 || currentTime - lastMortarCheck > 1000) {
            lastMortarCheck = currentTime;
            MortarEntity = mem.Read<uint64_t>(GameData.LocalPlayerInfo.Entity + mortarLocationOffset);
            mortarActive = !Utils::ValidPtr(MortarEntity);

            if (!mortarActive) {
                currentTarget.reset();
                calculationValid = false;
                return;
            }

            // 读取迫击炮初始旋转数据
            MortarRotation = mem.Read<FRotator>(MortarEntity + mortarRotationOffset);
            if (MortarRotation == FRotator()) {
                mortarActive = false;
                currentTarget.reset();
                calculationValid = false;
                return;
            }
        }

        if (!mortarActive) return;

        // 强制显示车辆透视
        if (mortarActive) {
            DrawVehicles(Vehicles);
        }

        // 优化的目标搜索：减少搜索频率到每5帧一次，提高刷新率以解决目标移动不更新问题
        if (frameCounter % 5 == 0 || !currentTarget || currentTime - lastTargetSearch > 166) {
            lastTargetSearch = currentTime;

            float minDistance = FLT_MAX;
            std::unique_ptr<Target> bestTarget = nullptr;

            // 搜索玩家目标
            for (const auto& Item : Players) {
                const Player& p = Item.second;
                if (p.IsMe || p.IsMyTeam || p.State == CharacterState::Dead) continue;
                if (p.Distance <= 121 || p.Distance >= 700) continue;

                FVector2D screenPos = VectorHelper::WorldToScreen(p.Location);
                float centerDist = fabs(screenPos.X - GameData.Config.Overlay.ScreenWidth / 2.0f);

                if (centerDist < minDistance) {
                    minDistance = centerDist;
                    bestTarget = std::make_unique<Target>(p);
                }
            }

            // 搜索车辆目标
            for (const auto& Item : Vehicles) {
                const VehicleInfo& v = Item.second;
                if (v.Distance <= 121 || v.Distance >= 700) continue;

                FVector2D screenPos = VectorHelper::WorldToScreen(v.Location);
                float centerDist = fabs(screenPos.X - GameData.Config.Overlay.ScreenWidth / 2.0f);

                if (centerDist < minDistance) {
                    minDistance = centerDist;
                    bestTarget = std::make_unique<Target>(v);
                }
            }

            // 每次都更新当前目标，确保目标移动时能够刷新
            if (bestTarget) {
                // 如果是同一个目标，则只更新位置信息
                if (currentTarget && currentTarget->Entity == bestTarget->Entity) {
                    currentTarget->Location = bestTarget->Location;
                    currentTarget->ScreenPos = bestTarget->ScreenPos;
                    currentTarget->Distance = bestTarget->Distance;
                    currentTarget->lastUpdateTime = bestTarget->lastUpdateTime;
                }
                else {
                    // 如果是新目标，则替换整个目标对象
                    currentTarget = std::move(bestTarget);
                    calculationValid = false; // 标记需要重新计算
                }
            }
        }

        // 缓存的距离和角度计算
        if (!calculationValid && currentTarget) {
            // 计算XY平面距离
            float deltaX = currentTarget->Location.X - GameData.LocalPlayerInfo.Location.X;
            float deltaY = currentTarget->Location.Y - GameData.LocalPlayerInfo.Location.Y;

            // 计算平面距离
            cachedHorizontalDistance = sqrt(deltaX * deltaX + deltaY * deltaY) / 100.0f;

            // 计算高度差，使用相同的比例因子
            float heightDifference = (currentTarget->Location.Z - GameData.LocalPlayerInfo.Location.Z) / 100.0f;

            // 将平面距离和高度差传递给迫击炮计算函数
            double mortarDistance = Mortar::getDistance(cachedHorizontalDistance, heightDifference);
            if (mortarDistance >= 0) {
                cachedMortarDistance = (float)mortarDistance;
                auto [pitch, dist] = find_closest_pitch(mortarDistance);
                cachedRecommendedPitch = pitch;
                calculationValid = true; // 标记计算完成
            }
        }

        // 简化的UI绘制
        if (!currentTarget) {
            FVector2D screenCenter = { GameData.Config.Overlay.ScreenWidth / 2.0f, GameData.Config.Overlay.ScreenHeight / 2.0f };

            // 简化的十字准线
            ImU32 color = IM_COL32(0, 200, 255, 180);
            RenderHelper::Line({ screenCenter.X - 5, screenCenter.Y }, { screenCenter.X + 5, screenCenter.Y }, color, 2.0f); // 水平线长度减半
            RenderHelper::Line({ screenCenter.X, screenCenter.Y - 5 }, { screenCenter.X, screenCenter.Y + 5 }, color, 2.0f); // 垂直线长度减半

            // 简化的文本
            RenderHelper::Text(
                Utils::StringToUTF8("搜索目标中...").c_str(),
                { screenCenter.X - 60, screenCenter.Y - 40 },
                color, 20, false, false
            );
            return;
        }

        // 简化的UI绘制
        FVector2D screenCenter = { GameData.Config.Overlay.ScreenWidth / 2.0f, GameData.Config.Overlay.ScreenHeight / 2.0f };
        FVector2D targetScreenPos = VectorHelper::WorldToScreen(currentTarget->Location);
        FVector2D screenTop = { GameData.Config.Overlay.ScreenWidth / 2.0f, 0.0f }; // 屏幕顶部中间点

        bool isVertical = fabs(targetScreenPos.X - screenCenter.X) < 2.5f;
        bool isLocked = isVertical && calculationValid;

        // 简化的十字准线
        ImU32 crossColor = isLocked ? IM_COL32(0, 255, 100, 255) : IM_COL32(0, 150, 255, 200);
        RenderHelper::Line({ screenCenter.X - 5, screenCenter.Y }, { screenCenter.X + 5, screenCenter.Y }, crossColor, 2.0f);
        RenderHelper::Line({ screenCenter.X, screenCenter.Y - 5 }, { screenCenter.X, screenCenter.Y + 5 }, crossColor, 2.0f);

        // 修改1: 根据目标类型选择不同的标记方式
        ImU32 targetColor = isLocked ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 165, 0, 200);

        if (currentTarget->IsPlayer) {
            // 玩家目标用圆形标记
            RenderHelper::Circle({ targetScreenPos.X, targetScreenPos.Y }, 12.0f, targetColor, 2.5f, 12);
        }
        else {
            // 车辆目标用方框标记
            float boxSize = 24.0f; // 方框大小
            RenderHelper::window_box(
                { targetScreenPos.X - boxSize / 2, targetScreenPos.Y - boxSize / 2 },
                { boxSize, boxSize },
                targetColor
            );
        }

        // 修改: 连接线从屏幕顶部中间划到目标
        RenderHelper::Line({ screenTop.X, screenTop.Y }, { targetScreenPos.X, targetScreenPos.Y }, targetColor, 2.0f);

        // 检测连接状态
        bool isConnected = GameData.Config.AimBot.Connected;
        if (!isConnected) {
            return;
        }

        // 修改: 右键按下时进行水平调整（替换原来的Shift键）
        bool isRightMousePressed = GameData.Keyboard.IsKeyDown(GameData.Config.AimBot.Mortar2);
        // 水平调整逻辑
        if (isRightMousePressed && currentTarget && calculationValid) {
            // 降低调整频率
            if (currentTime - lastHorizontalAdjust > 30) {
                lastHorizontalAdjust = currentTime;
                // 计算水平偏差
                float horizontalDiff = targetScreenPos.X - screenCenter.X;
                // 提高阈值，减少小幅调整
                if (fabs(horizontalDiff) > 2.0f) { 
                    // 计算移动方向和强度
                    int moveX = (horizontalDiff < 0) ? -1 : 1;
                    // 降低水平移动速度
                    moveX *= (fabs(horizontalDiff) > 20.0f) ? 10 : 2;

                    // 执行水平移动
                    switch (GameData.Config.AimBot.Controller) {
                    case 0:
                        KmBox::Move(moveX, 0);
                        break;
                    case 1:
                        KmBoxNet::Move(moveX, 0);
                        break;
                    case 2:
                        Lurker::Move(moveX, 0);
                        break;
                    case 3:
                        MoBox::Move(moveX, 0);
                        break;
                    default:
                        break;
                    }
                    isAdjustingHorizontal = true;
                }
                else {
                    isAdjustingHorizontal = false;
                }
            }
        }
        else {
            isAdjustingHorizontal = false;
        }

        // 垂直角度调整 - 使用用户提供的新代码
        if (GameData.Config.AimBot.Connected && isRightMousePressed) {
            static uint64_t lastAdjustTime = 0;
            // 限制调整频率，防止过度消耗资源
            if (currentTime - lastAdjustTime > 25) { // 每25ms最多调整一次
                lastAdjustTime = currentTime;
                if (MortarEntity) {
                    // 重新读取当前旋转值，确保准确性
                    FRotator currentRotation = mem.Read<FRotator>(MortarEntity + mortarRotationOffset);

                    float adjustment = cachedRecommendedPitch - currentRotation.Pitch;
                    if (fabs(adjustment) > 0.01f) {
                        // 修正滚轮方向逻辑：
                        // adjustment > 0: 需要增加pitch（目标pitch更大），滚轮向上
                        // adjustment < 0: 需要减少pitch（目标pitch更小），滚轮向下
                        int wheelInput = (adjustment > 0) ? 1 : -1;
                        // 直接发送输入，不使用睡眠
                        if (GameData.Config.AimBot.Controller == 1) {
                            kmNet_mouse_wheel(wheelInput);
                        }
                        else if (GameData.Config.AimBot.Controller == 0) {
                            KmBox::MortarWheel(wheelInput);
                        }
                        isAdjusting = true;
                    }
                    else {
                        isAdjusting = false;
                    }
                }
            }
        }
        else {
            isAdjusting = false;
        }

        // 状态显示
        if (calculationValid) {
            std::string statusText;

            if (isAdjustingHorizontal && isAdjusting) {
                statusText = "全自动校准中"; // 同时水平和垂直校准
            }
            else if (isAdjustingHorizontal) {
                statusText = "水平校准中";
            }
            else if (isAdjusting) {
                statusText = "垂直校准中";
            }
            else if (isLocked) {
                statusText = "已锁定";
            }
            else {
                statusText = "校准中";
            }

            std::string distText = "距离: " + std::to_string((int)cachedMortarDistance) + "M";
            std::string pitchText = "角度: " + std::to_string((int)cachedRecommendedPitch) + "°";
            std::string hintText = "按住右键进行全自动校准(水平+垂直)"; // 更新提示

            ImU32 textColor = isLocked ? IM_COL32(0, 255, 100, 255) : IM_COL32(255, 165, 0, 255);
            ImU32 hintColor = IM_COL32(180, 180, 255, 200);

            RenderHelper::Text(Utils::StringToUTF8(statusText).c_str(),
                { screenCenter.X - 100, screenCenter.Y + 150 }, textColor, 24, false, false);
            RenderHelper::Text(Utils::StringToUTF8(distText).c_str(),
                { screenCenter.X - 100, screenCenter.Y + 180 }, textColor, 20, false, false);
            RenderHelper::Text(Utils::StringToUTF8(pitchText).c_str(),
                { screenCenter.X + 50, screenCenter.Y + 180 }, textColor, 20, false, false);
            RenderHelper::Text(Utils::StringToUTF8(hintText).c_str(),
                { screenCenter.X - 120, screenCenter.Y + 210 }, hintColor, 18, false, false);
        }
    }

    // 绘制FPS和游戏信息
    static void DrawFPS(ImGuiIO& io)
    {
        if (!GameData.Config.Overlay.ShowFPS)
        {
            return;
        }

        auto GameScene = GameData.Scene;
        std::string Scene = Localize("未知", "Unknown");
        std::string Items = Localize("隐藏", "Hide");

        if (GameScene == Scene::FindProcess) {
            Scene = Localize("查找进程", "FindProcess");
        }
        else if (GameScene == Scene::Lobby) {
            Scene = Localize("大厅中", "Lobby");
        }
        else if (GameScene == Scene::Gaming) {
            Scene = Localize("游戏中", "Gaming");
        }

        if (GameData.Config.ESP.FocusMode)
        {
            Items = "战斗模式";
        }
        else {
            if (!GameData.Config.Item.ShowGroups.empty()) {
                std::vector<int> v(GameData.Config.Item.ShowGroups.begin(), GameData.Config.Item.ShowGroups.end());
                std::sort(v.begin(), v.end());
                std::string s;
                for (size_t i = 0; i < v.size(); ++i) {
                    char c = v[i] == 1 ? 'A' : v[i] == 2 ? 'B' : v[i] == 3 ? 'C' : 'D';
                    s += c;
                    if (i + 1 < v.size()) s += "+";
                }
                Items = s;
            } else {
                if (GameData.Config.Item.ShowGroup == 2)
                {
                    Items = "A";
                }
                else if (GameData.Config.Item.ShowGroup == 3) {
                    Items = "B";
                }
                else if (GameData.Config.Item.ShowGroup == 4) {
                    Items = "C";
                }
                else if (GameData.Config.Item.ShowGroup == 5) {
                    Items = "D";
                }
                else if (GameData.Config.Item.ShowGroup == 1) {
                    Items = "All";
                }
                Items = Items;
            }
        }

        //信息开关
        if (GameData.Config.ESP.DataSwitch)
        {
            // 调整信息位置
            auto TextPosition = ImVec2(0, 15);
            int spectatedCount = GameData.LocalPlayerInfo.SpectatedCount;
            float DamageDealtOnEnemy = static_cast<float>(GameData.LocalPlayerInfo.DamageDealtOnEnemy);

            ImColor color = GetColorFromConfig(GameData.Config.ESP.Color.Info.Skeleton);

            // 构建格式化字符串
            std::string infoText = std::format("{} {}\n{}:{}\n{}: {}\n{}: {}{}\n{}: {}\n{}: {:.2f}\n{}: {}\n{}: {}",
                Scene,
                (int)io.Framerate,
                Localize("自瞄开关", "Aimbot enable"),
                GameData.Config.AimBot.Enable ? "开" : "关",
                Localize("战斗模式", "Battle Mode"),
                GameData.Config.ESP.FocusMode ? Localize("开启", "On") : Localize("关闭", "Off"),
                Localize("自瞄配置", "AimBot Profile"),
                Localize("配置", "Profile"),
                GameData.Config.AimBot.ConfigIndex + 1,
                Localize("分组显示", "Team Display"), Items,
                Localize("当前伤害", "Current Damage"), DamageDealtOnEnemy,
                Localize("当前排名", "Current Rank"), GameData.NumAliveTeams,
                Localize("迷雾数量", "Blind Zone Count"), GameData.FogPlayerCount
            );

            // 渲染信息文本
            RenderHelper::StrokeText(Utils::StringToUTF8(infoText).c_str(), TextPosition, color, 17, false, false);

            // 根据需要调整位置
            TextPosition.y += 135;

            // 根据观战数值决定颜色和字体大小
            ImU32 spectatedColor = (spectatedCount > 0) ? IM_COL32(255, 0, 0, 255) : ImU32(color);
            int fontSize = 17 + (spectatedCount > 0 ? 5 : 0); // 大于0时字体大小增加5

            // 渲染"观战"文本
            std::string spectatedText = std::format("{}: {}", Localize("观众数量", "Spectated Count"), spectatedCount);
            RenderHelper::StrokeText(Utils::StringToUTF8(spectatedText).c_str(), TextPosition, spectatedColor, fontSize, false, false);
            //观战屏幕中间提示

            float posX = GameData.Config.Overlay.ScreenWidth / 2;
            //float posY = GameData.Config.Overlay.ScreenHeight - 150;
            float posY = 150;
            ImU32 textColor = (GameData.LocalPlayerInfo.SpectatedCount > 0) ? true : false;
            if (textColor) {
                RenderHelper::StrokeText(Utils::StringToUTF8(std::format(
                    "有菜逼偷学技术   当前菜逼数量 {}  ",
                    GameData.LocalPlayerInfo.SpectatedCount
                    //)).c_str(), ImVec2(posX, posY), IM_COL32(255, 0, 0, 255), 18, true, true);
                )).c_str(), ImVec2(posX, posY), IM_COL32(255, 0, 0, 255), 24, true, true);
            }
        }
    }

    

    // 绘制骨骼（不带物理可见性检测）- 批量绘制版本
    static void DrawSkeleton(Player Player, const ImU32 SkeletonUseColor, const float Thickness)
    {
        FVector2D neckpos = Player.Skeleton.ScreenBones[EBoneIndex::Neck_01];
        FVector2D pelvispos = Player.Skeleton.ScreenBones[EBoneIndex::Pelvis];
        FVector2D headpos = Player.Skeleton.ScreenBones[EBoneIndex::Head];
        FVector2D foreheadpos = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead];

        std::vector<std::pair<ImVec2, ImVec2>> Bones;
        bool failed = false;

        // 构建所有骨骼线段
        for (const auto& a : SkeletonLists::Skeleton)
        {
            FVector2D previous = FVector2D(0, 0);
            FVector2D current, p1, c1;

            for (EBoneIndex bone : a)
            {
                current = (bone == EBoneIndex::Neck_01) ? neckpos :
                    (bone == EBoneIndex::Pelvis) ? pelvispos :
                    Player.Skeleton.ScreenBones[bone];

                if (previous.X != 0.f && previous.Y != 0.f)
                {
                    p1 = previous;
                    c1 = current;

                    Bones.emplace_back(std::make_pair(ImVec2(p1.X, p1.Y), ImVec2(c1.X, c1.Y)));

                    if (p1.X == INFINITY || p1.Y == INFINITY || c1.X == INFINITY || c1.Y == INFINITY) {
                        failed = true;
                        Bones.clear();
                        break;
                    }
                }
                previous = current;
            }

            if (failed)
                break;
        }

        // 批量绘制所有骨骼线条
        if (!Bones.empty()) {
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            for (const auto& line : Bones) {
                drawList->AddLine(line.first, line.second, SkeletonUseColor, Thickness);
            }
        }

    }

    // 绘制骨骼（带物理可见性检测）- 批量绘制版本
    static void DrawSkeleton_Physx(Player Player, const float Thickness)
    {
        // 骨骼可见性检测
        for (const auto& Bone : Player.Skeleton.LocationBones) {
            int idx = Bone.first;
            Player.Skeleton.BonesVisiablity[idx] = LineTrace::LineTraceSingle(GameData.Camera.Location, Bone.second);
        }

        // 获取关键骨骼位置和可见性
        FVector2D neckpos = Player.Skeleton.ScreenBones[EBoneIndex::Neck_01];
        bool bneckpos = Player.Skeleton.BonesVisiablity[EBoneIndex::Neck_01];
        FVector2D pelvispos = Player.Skeleton.ScreenBones[EBoneIndex::Pelvis];
        bool bpelvispos = Player.Skeleton.BonesVisiablity[EBoneIndex::Pelvis];
        FVector2D headpos = Player.Skeleton.ScreenBones[EBoneIndex::Head];
        bool bheadpos = Player.Skeleton.BonesVisiablity[EBoneIndex::Head];
        FVector2D foreheadpos = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead];
        FVector2D footpos = Player.Skeleton.ScreenBones[EBoneIndex::Foot_L];
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImU32 visibleColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Visible.Skeleton));
        ImU32 invisibleColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Default.Skeleton));

        if (Player.GroggyHealth < 99)
        {
            ImU32 groggy = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Groggy.Skeleton));
            ImU32 red = IM_COL32(255, 0, 0, 255);
            visibleColor = red;
            invisibleColor = groggy;
        }


        // 构建骨骼线段及其可见性
        std::vector<std::pair<ImVec2, ImVec2>> VisibleBones;
        std::vector<std::pair<ImVec2, ImVec2>> InvisibleBones;
        bool failed = false;

        // 构建骨骼线段
        for (const auto& a : SkeletonLists::Skeleton)
        {
            FVector2D previous = FVector2D(0, 0);
            FVector2D current, p1, c1;
            bool prevVisibility = false, currVisibility = false;
            EBoneIndex previousBone = (EBoneIndex)0;

            for (EBoneIndex bone : a)
            {
                current = (bone == EBoneIndex::Neck_01) ? neckpos :
                    (bone == EBoneIndex::Pelvis) ? pelvispos :
                    Player.Skeleton.ScreenBones[bone];

                currVisibility = (bone == EBoneIndex::Neck_01) ? bneckpos :
                    (bone == EBoneIndex::Pelvis) ? bpelvispos :
                    Player.Skeleton.BonesVisiablity[bone];

                if (previous.X != 0.f && previous.Y != 0.f)
                {
                    p1 = previous;
                    c1 = current;

                    if (p1.X == INFINITY || p1.Y == INFINITY || c1.X == INFINITY || c1.Y == INFINITY) {
                        break;
                    }

                    // 绘制逻辑：根据两端点的可见性决定绘制方式
                    if (prevVisibility && currVisibility) {
                        // 两端都可见 -> 纯可见色
                        drawList->AddLine(ImVec2(p1.X, p1.Y), ImVec2(c1.X, c1.Y), visibleColor, Thickness);
                    }
                    else if (!prevVisibility && !currVisibility) {
                        // 两端都不可见 -> 纯不可见色
                        drawList->AddLine(ImVec2(p1.X, p1.Y), ImVec2(c1.X, c1.Y), invisibleColor, Thickness);
                    }
                    else {
                        FVector pw = Player.Skeleton.LocationBones[(int)previousBone];
                        FVector cw = Player.Skeleton.LocationBones[(int)bone];
                        const int steps = 50;
                        for (int i = 0; i < steps; ++i) {
                            float t0 = (float)i / (float)steps;
                            float t1 = (float)(i + 1) / (float)steps;
                            ImVec2 s = ImVec2(p1.X + (c1.X - p1.X) * t0, p1.Y + (c1.Y - p1.Y) * t0);
                            ImVec2 e = ImVec2(p1.X + (c1.X - p1.X) * t1, p1.Y + (c1.Y - p1.Y) * t1);
                            float tm = (t0 + t1) * 0.5f;
                            FVector wm = FVector(
                                pw.X + (cw.X - pw.X) * tm,
                                pw.Y + (cw.Y - pw.Y) * tm,
                                pw.Z + (cw.Z - pw.Z) * tm
                            );
                            bool segVisible = LineTrace::LineTraceSingle(GameData.Camera.Location, wm);
                            ImU32 col = segVisible ? visibleColor : invisibleColor;
                            drawList->AddLine(s, e, col, Thickness);
                        }
                    }
                }
                previous = current;
                prevVisibility = currVisibility;
                previousBone = bone;
            }

            if (failed)
                break;
        }

        //// 批量绘制所有骨骼线段
        //ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        //ImU32 visibleColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Visible.Skeleton));
        //ImU32 invisibleColor = ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Default.Skeleton));

        //// 绘制可见骨骼
        //for (const auto& line : VisibleBones) {
        //    drawList->AddLine(line.first, line.second, visibleColor, Thickness);
        //}

        //// 绘制不可见骨骼
        //for (const auto& line : InvisibleBones) {
        //    drawList->AddLine(line.first, line.second, invisibleColor, Thickness);
        //}

        // 绘制头部圆圈（基于角色高度自适应大小）
        if (GameData.Config.ESP.HeadDrawing && headpos.X != INFINITY && headpos.Y != INFINITY)
        {
            // 计算角色高度（从脚到头）
            const auto h = (float)(int)footpos.Y - (float)(int)foreheadpos.Y;
            const float headRadius = h / 10.0f; // 自适应头部半径

            // 根据可见性选择颜色
            ImU32 headColor = bheadpos ? visibleColor : invisibleColor;

            ImVec2 center = ImVec2(headpos.X, headpos.Y);
            if (neckpos.X != INFINITY && neckpos.Y != INFINITY) {
                center.y = neckpos.Y - headRadius;
            }
            RenderHelper::Circle(center, headRadius, headColor, Thickness, 32);
        }
    }

    // 绘制FOV
    static void DrawFOV()
    {
        AimBotConfig Config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];

        if (Config.DynamicFov) {
            Config.FOV = Config.FOV * (90.0f / GameData.Camera.FOV);
            Config.WheelFOV = Config.WheelFOV * (90.0f / GameData.Camera.FOV);
        }

        ImVec2 CenterPoint = { (float)GameData.Config.Overlay.ScreenWidth / 2, (float)GameData.Config.Overlay.ScreenHeight / 2 };

        if (Config.ShowFOV && !GameData.AimBot.Lock && GameData.AimBot.Type == EntityType::Player)
        {
            if (Config.FOV <= 0) return;
            RenderHelper::Circle(CenterPoint, Config.FOV, Utils::FloatToImColor(GameData.Config.AimBot.FOVColor), 2, 360);
        }

        if (GameData.Config.AimBot.ShowWheelFOV && !GameData.AimBot.Lock)
        {
            if (Config.WheelFOV <= 0) return;
            RenderHelper::Circle(CenterPoint, Config.WheelFOV, Utils::FloatToImColor(GameData.Config.AimBot.WheelFOVColor), 2, 360);
        }
    }

    // 绘制自瞄点
    static void DrawAimBotPoint()
    {
        if (GameData.Config.AimBot.ShowPoint && GameData.LocalPlayerInfo.IsScoping && !Utils::ValidPtr(GameData.AimBot.TargetPlayerInfo.Entity))
        {
            ImVec2 CenterPoint = { (float)GameData.Config.Overlay.ScreenWidth / 2, (float)GameData.Config.Overlay.ScreenHeight / 2 };
            if (GameData.AimBot.PredictedPos.X == 0 && GameData.AimBot.PredictedPos.Y == 0 && GameData.AimBot.PredictedPos.Z == 0) return;

            FVector2D PointPos = VectorHelper::WorldToScreen(GameData.AimBot.PredictedPos);

            ImVec2 ImPointPos = { PointPos.X, PointPos.Y };

            RenderHelper::CircleFilled(ImPointPos, GameData.Config.AimBot.PointSize, Utils::FloatToImColor(GameData.Config.AimBot.PointColor), GameData.Config.AimBot.PointSize * 2);
        }
    }

    // 绘制血条
    static void DrawHealthBar(DWORD Sign, float MaxHealth, float CurrentHealth, ImVec2 Pos, ImVec2 Size, ImColor Color, bool Horizontal, bool ShowBackupHealth = true)
    {
        static std::map<DWORD, HealthBar> HealthBarMap;
        if (!HealthBarMap.count(Sign))
        {
            HealthBarMap.insert({ Sign, HealthBar() });
        }
        if (HealthBarMap.count(Sign))
        {
            if (Horizontal)
                HealthBarMap[Sign].DrawHealthBar_Horizontal(MaxHealth, CurrentHealth, Pos, Size, Color, ShowBackupHealth);
            else
                HealthBarMap[Sign].DrawHealthBar_Vertical(MaxHealth, CurrentHealth, Pos, Size, Color, ShowBackupHealth);
        }
    }

    // 获取2D包围盒
    static ImVec4 Get2DBox(const Player& Player)
    {
        FVector2D Head = Player.Skeleton.ScreenBones.at(EBoneIndex::Head);
        FVector2D Root = Player.Skeleton.ScreenBones.at(EBoneIndex::Root);

        ImVec2 Size, Pos;
        Size.y = (Root.Y - Head.Y) * 1.09;
        Size.x = Size.y * 0.6;

        Pos = ImVec2(Root.X - Size.x / 2, Head.Y - Size.y * 0.08);

        return ImVec4{ Pos.x, Pos.y, Size.x, Size.y };
    }

    // 物品分组功能
    static std::unordered_map<int, std::list<ItemInfo>> GroupItems(std::vector<std::pair<uint64_t, ItemInfo>> Items, float ThresholdX, float ThresholdY) {
        static const auto IsNear = [&](const FVector2D& A, const FVector2D& B) {
            return std::abs(A.X - B.X) < ThresholdX && std::abs(A.Y - B.Y) < ThresholdY;
            };

        std::unordered_map<int, std::list<ItemInfo>> Groups;
        int GroupId = 0;

        for (auto& Key : Items) {
            auto& Item = Key.second;

            bool FoundGroup = false;
            Item.ScreenLocation = VectorHelper::WorldToScreen(Item.Location);

            for (auto& Group : Groups) {
                for (const auto& GroupItem : Group.second) {
                    if (IsNear(Item.ScreenLocation, GroupItem.ScreenLocation)) {
                        Group.second.push_back(Item);
                        FoundGroup = true;
                        break;
                    }
                }
                if (FoundGroup) break;
            }

            if (!FoundGroup) {
                Groups[GroupId++] = std::list<ItemInfo>{ Item };
            }
        }
        return Groups;
    }

    static void DrawItems(const std::unordered_map<uint64_t, ItemInfo>& Items)
    {
        if (!GameData.Config.Item.Enable) return;

        // 射线配置
        ImColor rayColor = GetColorFromConfig(GameData.Config.Item.RayColor);

        // 转换并排序物品
        std::vector<std::pair<uint64_t, ItemInfo>> Vectors(Items.begin(), Items.end());
        std::sort(Vectors.begin(), Vectors.end(), [](const auto& a, const auto& b) {
            return a.second.Distance > b.second.Distance;
            });

        // 公共样式设置
        float FontSize = GameData.Config.Item.FontSize;
        ImColor TextColor = ImColor(255, 255, 255);

        std::unordered_set<int> active = GameData.Config.Item.ShowGroups;
        int sg = GameData.Config.Item.ShowGroup;

        if (GameData.Config.Item.Combination)
        {
            // 组合模式下的绘制逻辑
            auto CombinationItems = GroupItems(Vectors, GameData.Config.Item.ThresholdX, GameData.Config.Item.ThresholdY);
            for (auto& Group : CombinationItems)
            {
                int Index = 0;
                bool bRayDrawn = false;  // 追踪是否已经绘制过射线
                FVector2D FirstPos;

                Group.second.sort([](const ItemInfo& A, const ItemInfo& B) {
                    return A.ItemType > B.ItemType;
                    });

                for (ItemInfo& ItemInfo : Group.second)
                {
                    // 基础过滤条件
                    if (ItemInfo.bHidden || ItemInfo.Distance > GameData.Config.Item.DistanceMax) continue;

                    // 从配置获取物品详情并同步射线状态
                    auto itemConfigIt = GameData.Config.Item.Lists.find(ItemInfo.Name);
                    if (itemConfigIt == GameData.Config.Item.Lists.end()) continue;
                    ItemInfo.ShowRay = itemConfigIt->second.ShowRay;

                    int itemGroup = itemConfigIt->second.Group;
                    bool pass = true;
                    if (!active.empty()) {
                        pass = active.count(itemGroup) > 0;
                    } else {
                        if (sg == 1) {
                            pass = true;
                        } else if (sg == 2) {
                            pass = itemGroup == 1;
                        } else if (sg == 3) {
                            pass = itemGroup == 2;
                        } else if (sg == 4) {
                            pass = itemGroup == 3;
                        } else if (sg == 5) {
                            pass = itemGroup == 4;
                        } else {
                            pass = true;
                        }
                    }
                    if (!pass) continue;

                    // 设置颜色
                    switch (itemConfigIt->second.Group) {
                    case 1: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupAColor); break;
                    case 2: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupBColor); break;
                    case 3: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupCColor); break;
                    case 4: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupDColor); break;
                    default: continue;
                    }

                    // 计算屏幕位置
                    ItemInfo.ScreenLocation = VectorHelper::WorldToScreen(ItemInfo.Location);
                    if (!VectorHelper::IsInScreen(ItemInfo.ScreenLocation)) continue;
                    ItemInfo.Distance = GameData.Camera.Location.Distance(ItemInfo.Location) / 100.0f;

                    // 射线绘制（只绘制第一个有效物品的射线）
                    if (GameData.Config.Item.ShowRay && ItemInfo.ShowRay && !bRayDrawn)
                    {
                        ImVec2 IconLocation = ImVec2(ItemInfo.ScreenLocation.X, ItemInfo.ScreenLocation.Y);

                        if (GameData.Config.Item.ShowIcon)
                        {
                            // 如果物品有图标，调整ScreenLocation为图标位置
                            float Scale = (FontSize / 14.f);
                            std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";

                            if (GImGuiTextureMap.count(IconUrl) && GImGuiTextureMap[IconUrl].Width > 0)
                            {
                                // 计算图标的大小和位置
                                float TargetHeight = 14.f * Scale;
                                float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                                float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom + 5.0f;
                                float IconHeight = TargetHeight + 10.0f;

                                // 将图标的位置调整到物品的屏幕位置
                                IconLocation.x -= (FontSize + 4); // 根据字体大小调整图标位置
                                IconLocation.y -= (IconHeight + FontSize + 2 + 5.0f); // 根据图标和字体调整垂直位置，上移5像素

                                // 更新射线的终点为图标的位置
                                RenderHelper::DrawLine(
                                    screenTopCenter,
                                    { IconLocation.x, IconLocation.y },
                                    rayColor,
                                    GameData.Config.Item.RayWidth
                                );
                            }
                        }
                        else
                        {
                            // 如果没有图标，使用默认的ScreenLocation
                            RenderHelper::DrawLine(
                                screenTopCenter,
                                { IconLocation.x, IconLocation.y },
                                rayColor,
                                GameData.Config.Item.RayWidth
                            );
                        }

                        // 标记射线已绘制
                        bRayDrawn = true;
                    }

                    // 记录第一个有效物品位置
                    if (FirstPos.X == 0 && FirstPos.Y == 0)
                    {
                        FirstPos = ItemInfo.ScreenLocation;
                    }

                    // 文本内容构建 - 修改：距离始终显示在右边
                    std::string Text = Utils::StringToUTF8(std::format("{}", ItemInfo.DisplayName));
                    if (GameData.Config.Item.ShowDistance && Index == 0)
                    {
                        Text += " [" + std::to_string((int)ItemInfo.Distance) + "M]";
                    }

                    // 计算绘制位置
                    ImVec2 ScreenLocation = { FirstPos.X, FirstPos.Y - ((FontSize + 1) * Index) };

                    // 图标绘制
                    if (GameData.Config.Item.ShowIcon)
                    {
                        float Scale = (FontSize / 14.f);
                        std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";
                        if (GImGuiTextureMap.count(IconUrl) && GImGuiTextureMap[IconUrl].Width > 0)
                        {
                            float TargetHeight = 14.f * Scale;
                            float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                            float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom + 5.0f;
                            float IconHeight = TargetHeight + 10.0f;
                            ScreenLocation.x -= (FontSize + 4);
                            ScreenLocation.y -= 5.0f; // 上移5像素
                            RenderHelper::Image(GImGuiTextureMap[IconUrl].Texture, ScreenLocation, ImVec2(IconWidth, IconHeight), true, TextColor);

                            // 如果启用了同时显示图标和文字
                            if (GameData.Config.Item.ShowIconAndText)
                            {
                                // 调整文字位置在图标右侧
                                ImVec2 TextLocation = { ScreenLocation.x + IconWidth + 5.0f, ScreenLocation.y + (IconHeight / 2) - (FontSize / 2) };
                                RenderHelper::StrokeText(Text.c_str(), TextLocation, TextColor, FontSize, false, false);
                            }
                        }
                    }
                    else
                    {
                        // 文本绘制 (仅在不显示图标时)
                        RenderHelper::StrokeText(Text.c_str(), { ScreenLocation.x, ScreenLocation.y }, TextColor, FontSize, false, false);
                    }

                    Index++;
                }
            }
        }
        else
        {
            // 非组合模式下的绘制逻辑
            for (auto& Item : Vectors)
            {
                ItemInfo& ItemInfo = Item.second;

                // 基础过滤条件
                if (ItemInfo.bHidden || ItemInfo.Distance > GameData.Config.Item.DistanceMax) continue;

                // 从配置获取物品详情并同步射线状态
                auto itemConfigIt = GameData.Config.Item.Lists.find(ItemInfo.Name);
                if (itemConfigIt == GameData.Config.Item.Lists.end()) continue;
                ItemInfo.ShowRay = itemConfigIt->second.ShowRay;

                int itemGroup = itemConfigIt->second.Group;
                bool pass = true;
                if (!active.empty()) {
                    pass = active.count(itemGroup) > 0;
                } else {
                    if (sg == 1) {
                        pass = true;
                    } else if (sg == 2) {
                        pass = itemGroup == 1;
                    } else if (sg == 3) {
                        pass = itemGroup == 2;
                    } else if (sg == 4) {
                        pass = itemGroup == 3;
                    } else if (sg == 5) {
                        pass = itemGroup == 4;
                    } else {
                        pass = true;
                    }
                }
                if (!pass) continue;

                // 设置颜色
                switch (itemConfigIt->second.Group) {
                case 1: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupAColor); break;
                case 2: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupBColor); break;
                case 3: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupCColor); break;
                case 4: TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupDColor); break;
                default: continue;
                }

                // 计算屏幕位置
                ItemInfo.ScreenLocation = VectorHelper::WorldToScreen(ItemInfo.Location);
                if (!VectorHelper::IsInScreen(ItemInfo.ScreenLocation)) continue;
                ItemInfo.Distance = GameData.Camera.Location.Distance(ItemInfo.Location) / 100.0f;

                // 射线绘制（检查全局和单独设置）
                if (GameData.Config.Item.ShowRay && ItemInfo.ShowRay)
                {
                    ImVec2 IconLocation = ImVec2(ItemInfo.ScreenLocation.X, ItemInfo.ScreenLocation.Y);

                    if (GameData.Config.Item.ShowIcon)
                    {
                        // 如果物品有图标，调整ScreenLocation为图标位置
                        float Scale = 1.f;
                        std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";

                        if (GImGuiTextureMap.count(IconUrl) && GImGuiTextureMap[IconUrl].Width > 0)
                        {
                            // 计算图标的大小和位置
                            float TargetHeight = 24.f * (FontSize / 14.f) * Scale;
                            float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                            float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom + 15.0f;
                            float IconHeight = TargetHeight + 20.0f;

                            // 调整图标位置
                            IconLocation.x -= IconWidth / 2;
                            IconLocation.y -= (IconHeight + FontSize + 2 + 5.0f); // 上移5像素

                            // 绘制射线到图标位置
                            RenderHelper::DrawLine(
                                screenTopCenter,
                                { IconLocation.x, IconLocation.y },
                                rayColor,
                                GameData.Config.Item.RayWidth
                            );
                        }
                    }
                    else
                    {
                        // 如果没有图标，使用默认的ScreenLocation
                        RenderHelper::DrawLine(
                            screenTopCenter,
                            { IconLocation.x, IconLocation.y },
                            rayColor,
                            GameData.Config.Item.RayWidth
                        );
                    }
                }

                // 文本内容构建 - 修改：距离显示在物品名字右边
                std::string Text = Utils::StringToUTF8(std::format("{}", ItemInfo.DisplayName));
                if (GameData.Config.Item.ShowDistance)
                {
                    Text += " [" + std::to_string((int)ItemInfo.Distance) + "M]";
                }

                // 图标绘制
                if (GameData.Config.Item.ShowIcon)
                {
                    float Scale = 1.f;
                    if (ItemInfo.ItemType == WeaponType::AR || ItemInfo.ItemType == WeaponType::DMR ||
                        ItemInfo.ItemType == WeaponType::SG || ItemInfo.ItemType == WeaponType::SR ||
                        ItemInfo.ItemType == WeaponType::LMG)
                    {
                        Scale = 1.4f;
                    }

                    std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";
                    if (GImGuiTextureMap.count(IconUrl) && GImGuiTextureMap[IconUrl].Width > 0)
                    {
                        float TargetHeight = 24.f * (FontSize / 14.f) * Scale;
                        float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                        float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom + 15.0f;
                        float IconHeight = TargetHeight + 20.0f;

                        ItemInfo.ScreenLocation.X -= IconWidth / 2;
                        ItemInfo.ScreenLocation.Y -= (IconHeight + FontSize + 2 + 5.0f); // 上移5像素

                        RenderHelper::Image(
                            GImGuiTextureMap[IconUrl].Texture,
                            { ItemInfo.ScreenLocation.X, ItemInfo.ScreenLocation.Y },
                            ImVec2(IconWidth, IconHeight),
                            true,
                            TextColor
                        );

                        // 如果启用了同时显示图标和文字
                        if (GameData.Config.Item.ShowIconAndText)
                        {
                            // 调整文字位置在图标下方，包含距离信息
                            ImVec2 TextLocation = {
                                ItemInfo.ScreenLocation.X + (IconWidth / 2),
                                ItemInfo.ScreenLocation.Y + IconHeight + 2.0f
                            };
                            RenderHelper::StrokeText(Text.c_str(), TextLocation, TextColor, FontSize, true, true);
                        }
                    }
                }
                else
                {
                    // 文本绘制，包含距离信息
                    RenderHelper::StrokeText(Text.c_str(), { ItemInfo.ScreenLocation.X, ItemInfo.ScreenLocation.Y }, TextColor, FontSize, true, true);
                }
            }
        }
    }
    // 获取或创建唯一ID
    static uint64_t GetOrCreateUniqueId(uint64_t grenadeId)
    {
        // 检查 grenadeId 是否已经有分配的 uniqueId
        auto it = GrenadeUniqueIds.find(grenadeId);
        if (it == GrenadeUniqueIds.end())
        {
            // 分配一个新的 uniqueId，并确保线程安全
            uint64_t newId = NextUniqueId.fetch_add(1, std::memory_order_relaxed);
            GrenadeUniqueIds[grenadeId] = newId;
            return newId;
        }
        return it->second;
    }

    // 处理手雷爆炸
    static void HandleGrenadeExplosion(uint64_t grenadeId)
    {
        uint64_t uniqueId = GetOrCreateUniqueId(grenadeId);

        if (ExplodedGrenades.find(uniqueId) == ExplodedGrenades.end())
        {
            ExplodedGrenades.insert(uniqueId);

            // 确保清除对应的手雷轨迹
            if (GrenadeTrajectories.find(uniqueId) != GrenadeTrajectories.end())
            {
                GrenadeTrajectories.erase(uniqueId);
            }
        }
    }

    // 绘制投掷物
    static void DrawProjects(const std::unordered_map<uint64_t, ProjectInfo>& Items)
    {
        if (!GameData.Config.Project.Enable) return;

        std::vector<std::pair<uint64_t, ProjectInfo>> Vectors(Items.begin(), Items.end());
        float bestC4Remaining = -1.0f;
        const float c4TotalTime = 16.0f;

        std::sort(Vectors.begin(), Vectors.end(), [](const std::pair<uint64_t, ProjectInfo>& a, const std::pair<uint64_t, ProjectInfo>& b) {
        return a.second.Distance > b.second.Distance;
        });

        // 用于存储当前帧中所有活跃的手雷ID
        std::unordered_set<uint64_t> ActiveGrenades;

        for (auto& Item : Vectors)
        {
            ProjectInfo& Project = Item.second;

            if (Project.Distance > GameData.Config.Project.DistanceMax)
                continue;
            bool isC4 = (Project.EntityName == "ProjC4_C");
            if (!isC4 && (Project.bVisible == 1 || Project.TimeTillExplosion <= 0.0f))
                continue;

            Project.ScreenLocation = VectorHelper::WorldToScreen(Project.Location);
            Project.Distance = GameData.Camera.Location.Distance(Project.Location) / 100.0f;

            const int FontSize = GameData.Config.Project.FontSize;
            const float Scale = (float)FontSize / 14.f;

            std::string Text = Utils::StringToUTF8(std::format(
                "{} [{}{}]",
                (Languages == 1 ? Project.Name : "Project Name"),
                (int)Project.Distance,
                (Languages == 1 ? "M" : "M")
            ));
            ImColor InfoColor = Utils::FloatToImColor(GameData.Config.Project.Color);
            Project.ScreenLocation.Y += 5 * Scale;
            ImVec2 InfoSize = RenderHelper::StrokeText(Text.c_str(), { Project.ScreenLocation.X, Project.ScreenLocation.Y }, InfoColor, FontSize, true, false);

            uint64_t grenadeId = Project.Entity;
            uint64_t uniqueId = GetOrCreateUniqueId(grenadeId);

            // 检查手雷是否已爆炸
            if (Project.TimeTillExplosion <= 0)
            {
                HandleGrenadeExplosion(grenadeId);
                continue;
            }

            // 跳过已爆炸手雷
            if (ExplodedGrenades.find(uniqueId) != ExplodedGrenades.end())
            {
                GrenadeTrajectories.erase(uniqueId);
                continue;
            }

            // 将当前手雷ID添加到活跃手雷集合中
            ActiveGrenades.insert(uniqueId);

            // 初始化或清空轨迹
            if (!GameData.Config.Project.GrenadeTrajectory)
            {
                GrenadeTrajectories.erase(uniqueId);
            }
            else
            {
                if (GrenadeTrajectories.find(uniqueId) == GrenadeTrajectories.end())
                {
                    GrenadeTrajectories[uniqueId] = {}; // 初始化为空
                }

                // 更新轨迹
                if (GrenadeTrajectories[uniqueId].empty() ||
                    (GrenadeTrajectories[uniqueId].back() != Project.Location &&
                        GrenadeTrajectories[uniqueId].back().Distance(Project.Location) > 5.0f))
                {
                    GrenadeTrajectories[uniqueId].push_back(Project.Location);
                }
            }

            // 如果是手雷类型
            if (Project.EntityName == "ProjGrenade_C")
            {
                ImVec2 HealthBarPos, HealthBarSize;
                HealthBarSize = { 26 * Scale, 5 * Scale };
                HealthBarPos = { Project.ScreenLocation.X - (HealthBarSize.x / 2), Project.ScreenLocation.Y + (InfoSize.y + 3 * Scale) };
                DrawHealthBar(Project.Entity, 100.f, Project.TimeTillExplosion / 4.0f * 100.0f, HealthBarPos, HealthBarSize, InfoColor, true, false);

                // 绘制轨迹
                if (GameData.Config.Project.GrenadeTrajectory)
                {
                    for (size_t i = 1; i < GrenadeTrajectories[uniqueId].size(); ++i)
                    {
                        FVector prevPos = GrenadeTrajectories[uniqueId][i - 1];
                        FVector currPos = GrenadeTrajectories[uniqueId][i];
                        FVector2D prevScreen = VectorHelper::WorldToScreen(prevPos);
                        FVector2D currScreen = VectorHelper::WorldToScreen(currPos);

                        if (prevScreen.X > 0 && prevScreen.X < GameData.Config.Overlay.ScreenWidth &&
                            prevScreen.Y > 0 && prevScreen.Y < GameData.Config.Overlay.ScreenHeight &&
                            currScreen.X > 0 && currScreen.X < GameData.Config.Overlay.ScreenWidth &&
                            currScreen.Y > 0 && currScreen.Y < GameData.Config.Overlay.ScreenHeight)
                        {
                            ImColor TrajectoryColor = Utils::FloatToImColor(GameData.Config.Project.TrajectoryColor);
                            ImGui::GetBackgroundDrawList()->AddLine(
                                { prevScreen.X, prevScreen.Y },
                                { currScreen.X, currScreen.Y },
                                TrajectoryColor,
                                GameData.Config.Project.TrajectorySize);
                        }
                    }
                }

                // 绘制爆炸范围
                ImColor ExplosionRangeColor = Utils::FloatToImColor(GameData.Config.Project.explosionrangeColor);
                DrawExplosiveRange(Project.Location, 380, ExplosionRangeColor, 3);
            }
            else if (Project.EntityName == "ProjC4_C")
            {
                if ((int)Project.Distance > 0)
                {
                    ImColor RangeColor = Utils::FloatToImColor(GameData.Config.Project.explosionrangeColor);
                    DrawExplosiveRange(Project.Location, 1600.0f, RangeColor, 3);
                    ImColor TimerColor = Utils::FloatToImColor(GameData.Config.Project.ChareColor);
                    float totalTime = 16.0f;
                    float remaining = Project.TimeTillExplosion;
                    if (remaining > 0.0f && remaining <= totalTime)
                    {
                        std::string countdownText = std::format("{:.1f}秒", remaining);
                        RenderHelper::StrokeText(countdownText.c_str(), { Project.ScreenLocation.X, Project.ScreenLocation.Y + (InfoSize.y + 3 * Scale) }, TimerColor, FontSize, true, false);
                        bestC4Remaining = (bestC4Remaining < 0.0f) ? remaining : (bestC4Remaining < remaining ? bestC4Remaining : remaining);
                    }
                }
            }
        }

        // 清除不再活跃的手雷轨迹
        for (auto it = GrenadeTrajectories.begin(); it != GrenadeTrajectories.end(); )
        {
            if (ActiveGrenades.find(it->first) == ActiveGrenades.end())
            {
                it = GrenadeTrajectories.erase(it);
            }
            else
            {
                ++it;
            }
        }

        if (bestC4Remaining > 0.0f && bestC4Remaining <= c4TotalTime)
        {
            ImColor centerColor = Utils::FloatToImColor(GameData.Config.Project.ChareColor);
            int fontSize = GameData.Config.Project.ChareFontSize > 0 ? (GameData.Config.Project.ChareFontSize * 3) : 72;
            std::string centerText = std::format("{:.1f}秒", bestC4Remaining);
            RenderHelper::StrokeText(centerText.c_str(),
                { GameData.Config.Overlay.ScreenWidth / 2.0f, GameData.Config.Overlay.ScreenHeight / 2.0f },
                centerColor,
                fontSize,
                true,
                false);
        }
    }

    // 绘制玩家的投掷物计时器
    static void DrawLocalPlayerProject()
    {
        if (GameData.Config.Project.ShowChareTime)
        {
            if (!GameData.Config.Project.BarShowChareTime && !GameData.Config.Project.TextShowChareTime) return;

            if (GameData.LocalPlayerInfo.ElapsedCookingTime > 0 && (int)GameData.LocalPlayerInfo.WeaponClassByte == 10)
            {
                float totalTime = 5.05f;
                if (GameData.LocalPlayerInfo.WeaponName == "手雷")
                    totalTime = 5.0f;
                else if (GameData.LocalPlayerInfo.WeaponName == "闪光弹")
                    totalTime = 2.5f;
                float progressValue = GameData.LocalPlayerInfo.ElapsedCookingTime / totalTime;

                ImColor Color = Utils::FloatToImColor(GameData.Config.Project.ChareColor);

                RenderHelper::DrawDashboardProgress(
                    ImVec2(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2),
                    120.f * (GameData.Config.Project.ChareFontSize / 14.f),
                    2,
                    20,
                    8,
                    progressValue,
                    Color
                );

                float remaining = totalTime - GameData.LocalPlayerInfo.ElapsedCookingTime;
                if (remaining > 0.0f)
                {
                    int fontSize = GameData.Config.Project.ChareFontSize > 0 ? (GameData.Config.Project.ChareFontSize * 2) : 56;
                    std::string centerText = std::format("{:.1f}秒", remaining);
                    RenderHelper::StrokeText(centerText.c_str(),
                        { GameData.Config.Overlay.ScreenWidth / 2.0f, GameData.Config.Overlay.ScreenHeight / 2.0f },
                        Color,
                        fontSize,
                        true,
                        false);
                }
            }
        }
    }

    // 绘制爆炸范围
    static void DrawExplosiveRange(FVector center, float radius, ImColor color, float thickness)
    {
        if (GameData.Config.Project.explosionrange)
        {
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
                        ImGui::GetBackgroundDrawList()->AddLine({ Vertices[i].X, Vertices[i].Y }, { Vertices[i + 1].X,  Vertices[i + 1].Y }, color, thickness);
            }
        }
    }

    // 绘制车辆
    static void DrawVehicles(const std::unordered_map<uint64_t, VehicleInfo>& Vehicles)
    {
        if (!GameData.Config.Vehicle.Enable) return;

        std::vector<std::pair<uint64_t, VehicleInfo>> Vectors(Vehicles.begin(), Vehicles.end());

        std::sort(Vectors.begin(), Vectors.end(), [](const std::pair<uint64_t, VehicleInfo>& a, const std::pair<uint64_t, VehicleInfo>& b) {
            return a.second.Distance > b.second.Distance;
            });

        for (auto& Item : Vectors)
        {
            VehicleInfo& Vehicle = Item.second;

            if (Vehicle.Distance > GameData.Config.Vehicle.DistanceMax) continue;

            Vehicle.ScreenLocation = VectorHelper::WorldToScreen(Vehicle.Location);
            Vehicle.Distance = GameData.Camera.Location.Distance(Vehicle.Location) / 100.0f;
            std::string Text = Utils::StringToUTF8(std::format("{}\n{}M", Vehicle.Name, (int)Vehicle.Distance));
            ImColor InfoColor = Utils::FloatToImColor(GameData.Config.Vehicle.Color);

            ImVec2 HeadInfoSize = RenderHelper::StrokeText(Text.c_str(), { Vehicle.ScreenLocation.X, Vehicle.ScreenLocation.Y },
                InfoColor, GameData.Config.Vehicle.FontSize, true, true);

            // 计算血量和油量的百分比
            float HealthPercent = Vehicle.VehicleHealth / Vehicle.VehicleHealthMax;
            float FuelPercent = Vehicle.VehicleFuel / Vehicle.VehicleFuelMax;

            // 耐久血条绘制
            if (GameData.Config.Vehicle.Health)
            {
                float HealthMaxWidth = 60.0f; // 设置血量条最大宽度
                float HealthWidth = HealthMaxWidth * HealthPercent;
                ImColor Fuelbarcolor = Utils::FloatToImColor(GameData.Config.Vehicle.Fuelbarcolor);
                ImVec2 HealthBarPos = { Vehicle.ScreenLocation.X - HealthMaxWidth / 2, Vehicle.ScreenLocation.Y + HeadInfoSize.y - 20 };
                DrawBar(HealthBarPos, HealthMaxWidth, HealthPercent, Fuelbarcolor);
            }

            // 油量条绘制
            if (GameData.Config.Vehicle.Durability)
            {
                float FuelMaxWidth = 60.0f; // 设置油量条最大宽度
                float FuelWidth = FuelMaxWidth * FuelPercent;
                ImColor Healthbarcolor = Utils::FloatToImColor(GameData.Config.Vehicle.Healthbarcolor);
                ImVec2 FuelBarPos = { Vehicle.ScreenLocation.X - FuelMaxWidth / 2, Vehicle.ScreenLocation.Y + HeadInfoSize.y - 26 };
                DrawBar(FuelBarPos, FuelMaxWidth, FuelPercent, Healthbarcolor);
            }
        }
    }

    // 绘制车轮信息
    static void DrawVehicleWheels(const std::unordered_map<uint64_t, VehicleWheelInfo>& VehicleWheels)
    {
        std::vector<std::pair<uint64_t, VehicleWheelInfo>> Vectors(VehicleWheels.begin(), VehicleWheels.end());

        std::sort(Vectors.begin(), Vectors.end(), [](const std::pair<uint64_t, VehicleWheelInfo>& a, const std::pair<uint64_t, VehicleWheelInfo>& b) {
            return a.second.Distance > b.second.Distance;
            });

        for (auto& Item : Vectors)
        {
            VehicleWheelInfo& Wheel = Item.second;

            Wheel.ScreenLocation = VectorHelper::WorldToScreen(Wheel.Location);
            float Distance = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, Wheel.ScreenLocation.X, Wheel.ScreenLocation.Y);

            std::string Text = std::format("{}M {}", (int)Distance, Wheel.DampingRate);
            ImColor InfoColor = ImColor(255, 255, 255);

            ImVec2 HeadInfoSize = RenderHelper::StrokeText(Text.c_str(), { Wheel.ScreenLocation.X, Wheel.ScreenLocation.Y }, InfoColor, 14, true, true);
        }
    }

    // 获取玩家排名信息
    static PlayerRankInfo GetPlayerRankInfo(const std::unordered_map<std::string, PlayerRankList> PlayerRankLists, const Player Player)
    {
        PlayerRankInfo PlayerRankData;
        if (PlayerRankLists.count(Player.Name) > 0)
        {
            PlayerRankList PlayerRank = PlayerRankLists.at(Player.Name);

            switch (GameData.Config.PlayerList.RankMode) {
            case 1:
                PlayerRankData = PlayerRank.TPP;
                break;
            case 2:
                PlayerRankData = PlayerRank.SquadTPP;
                break;
            case 3:
                PlayerRankData = PlayerRank.FPP;
                break;
            case 4:
                PlayerRankData = PlayerRank.SquadFPP;
                break;
            default:
                break;
            }
        }
        return PlayerRankData;
    }

    // 雷达绘制（地图载具和物体）
    static void DrawRadars(const std::unordered_map<uint64_t, Player>& Players, const std::unordered_map<uint64_t, VehicleInfo>& Vehicles, const std::unordered_map<uint64_t, PackageInfo>& Packages)
    {
        if (GameData.Config.Radar.Main.MapRoom || GameData.Config.Radar.Mini.MapRoom)
        {
            std::vector<FVector> KeyFVector = MapKey[GameData.MapName];
            std::string IconUrl = "Assets/image/Map/back_room.png";

            for (auto& Location : KeyFVector)
            {
                FVector LocMap = Location - GameData.Radar.WorldOriginLocation;
                float Distance = GameData.Camera.Location.Distance(LocMap) / 100.0f;

                // 主雷达显示
                if (GameData.Radar.Visibility && GameData.Config.Radar.Main.MapRoom)
                {
                    FVector2D RadarLocation = Radar::WorldToRadarLocation(LocMap);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = GImGuiTextureMap[IconUrl].Width * (0.25f * (GameData.Config.Radar.Main.FontSize / 20.f));
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { RadarLocation.X, RadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                    }
                }

                // 小雷达显示
                if (GameData.Radar.MiniRadarVisibility && Distance < GameData.Radar.MiniRadarDistance && GameData.Config.Radar.Mini.MapRoom)
                {
                    FVector2D MiniRadarLocation = Radar::WorldToMiniRadarLocation(LocMap);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = GImGuiTextureMap[IconUrl].Width * (0.20f * (GameData.Config.Radar.Mini.FontSize / 20.f));
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { MiniRadarLocation.X, MiniRadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                    }
                }
            }
        }

        // 绘制车辆图标
        for (const auto& Item : Vehicles)
        {
            const VehicleInfo& Vehicle = Item.second;

            std::string IconUrl = "Assets/image/Map/car_land.png";

            if (GameData.Radar.Visibility && GameData.Config.Radar.Main.ShowVehicle)
            {
                FVector2D RadarLocation = Radar::WorldToRadarLocation(Vehicle.Location);

                if (GImGuiTextureMap[IconUrl].Width > 0) {
                    float IconSize = GImGuiTextureMap[IconUrl].Width * (0.25f * (GameData.Config.Radar.Main.FontSize / 14.f));
                    RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { RadarLocation.X, RadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                }
            }

            if (GameData.Radar.MiniRadarVisibility && Vehicle.Distance < GameData.Radar.MiniRadarDistance && GameData.Config.Radar.Mini.ShowVehicle)
            {
                FVector2D MiniRadarLocation = Radar::WorldToMiniRadarLocation(Vehicle.Location);

                if (GImGuiTextureMap[IconUrl].Width > 0) {
                    float IconSize = GImGuiTextureMap[IconUrl].Width * (0.20f * (GameData.Config.Radar.Mini.FontSize / 14.f));
                    RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { MiniRadarLocation.X, MiniRadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                }
            }
        }

        // 绘制包裹（空投和死亡箱）
        for (const auto& Item : Packages)
        {
            const PackageInfo& Package = Item.second;

            if (Package.Type == EntityType::AirDrop)
            {
                // 绘制空投
                std::string IconUrl = "Assets/image/Map/Carapackage_RedBox_C.png";

                if (GameData.Radar.Visibility && GameData.Config.Radar.Main.ShowAirDrop)
                {
                    FVector2D RadarLocation = Radar::WorldToRadarLocation(Package.Location);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = (0.2f * (GameData.Config.Radar.Main.FontSize / 14.f));//空投大小
                        float IconWidth = GImGuiTextureMap[IconUrl].Width * IconSize;
                        float IconHeight = GImGuiTextureMap[IconUrl].Height * IconSize;
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { RadarLocation.X, RadarLocation.Y }, ImVec2(IconWidth, IconHeight), 0);
                    }
                }

                if (GameData.Radar.MiniRadarVisibility && Package.Distance < GameData.Radar.MiniRadarDistance && GameData.Config.Radar.Mini.ShowAirDrop)
                {
                    FVector2D MiniRadarLocation = Radar::WorldToMiniRadarLocation(Package.Location);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = (0.1f * (GameData.Config.Radar.Main.FontSize / 14.f));//空投大小
                        float IconWidth = GImGuiTextureMap[IconUrl].Width * IconSize;
                        float IconHeight = GImGuiTextureMap[IconUrl].Height * IconSize;
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { MiniRadarLocation.X, MiniRadarLocation.Y }, ImVec2(IconWidth, IconHeight), 0);
                    }
                }
            }
            else {
                // 绘制死亡箱
                std::string IconUrl = "Assets/image/Map/dead.png";

                if (GameData.Radar.Visibility && GameData.Config.Radar.Main.ShowDeadBox)
                {
                    FVector2D RadarLocation = Radar::WorldToRadarLocation(Package.Location);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = GImGuiTextureMap[IconUrl].Width * (0.7f * (GameData.Config.Radar.Main.FontSize / 14.f));
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { RadarLocation.X, RadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                    }
                }

                if (GameData.Radar.MiniRadarVisibility && Package.Distance < GameData.Radar.MiniRadarDistance && GameData.Config.Radar.Mini.ShowDeadBox)
                {
                    FVector2D MiniRadarLocation = Radar::WorldToMiniRadarLocation(Package.Location);

                    if (GImGuiTextureMap[IconUrl].Width > 0) {
                        float IconSize = GImGuiTextureMap[IconUrl].Width * (0.6f * (GameData.Config.Radar.Mini.FontSize / 14.f));
                        RenderHelper::AddImageRotated(GImGuiTextureMap[IconUrl].Texture, { MiniRadarLocation.X, MiniRadarLocation.Y }, ImVec2(IconSize, IconSize), 0);
                    }
                }
            }
        }

        // 绘制玩家标记
        for (const auto& Item : Players)
        {
            const Player& Player = Item.second;

            if (Player.InFog || Player.IsMe || Player.IsMyTeam || (Player.State == CharacterState::Dead))
            {
                continue;
            }

            // 高抛雷处理
            if (GameData.Config.Project.GrenadePrediction)
            {
                // 获取玩家的血量
                float health = Player.Health; // 假设 Player.Health 返回一个 0.0f 到 100.0f 的血量值

                // 根据血量设置颜色
                ImColor HealthColor;
                if (health > 75.0f) {
                    // 血量高于 75% 时为绿色
                    HealthColor = ImColor(0, 255, 0, 255); // 绿色
                }
                else if (health > 50.0f) {
                    // 血量在 50% 到 75% 之间时为黄色
                    HealthColor = ImColor(255, 255, 0, 255); // 黄色
                }
                else if (health > 25.0f) {
                    // 血量在 25% 到 50% 之间时为橙色
                    HealthColor = ImColor(255, 165, 0, 255); // 橙色
                }
                else {
                    // 血量低于 25% 时为红色
                    HealthColor = ImColor(255, 0, 0, 255); // 红色
                }

                // 现有的投掷物预测逻辑
                float ProjectHeight;
                float Distance = GameData.Camera.Location.Distance(Player.Location) / 100.0f;
                if (Distance > 16.0f && Distance < 18.0f)
                {
                    ProjectHeight = 3970;
                }
                else if (Distance > 18.0f && Distance < 22.0f) {
                    ProjectHeight = 4103;
                }
                else if (Distance > 22.0f && Distance < 28.0f) {
                    ProjectHeight = 4328;
                }
                else if (Distance > 28.0f && Distance < 50.0f) {
                    ProjectHeight = 4397;
                }
                else if (Distance > 50.0f && Distance < 55.0f) {
                    ProjectHeight = 3809;
                }

                // 根据玩家的距离和投掷物预测高度绘制圆圈
                if (Distance > 16.0f && Distance < 55.0f)
                {
                    DrawExplosiveRange(
                        { Player.Location.X, Player.Location.Y, Player.Location.Z + ProjectHeight },
                        200, HealthColor, 3 // 使用根据血量动态计算的颜色
                    );
                }
            }

            // 雷达玩家颜色和方向处理
            RenderHelper::PlayerColor PlayerColors = RenderHelper::GetPlayerColor(Player);

            FVector AimDirection = FRotator(0.0f, Player.AimOffsets.Yaw, 0.0f).GetUnitVector();
            FVector2D Direction = FVector2D{ AimDirection.X, AimDirection.Y };
            float AngleRadians = atan2(Direction.Y, Direction.X);
            float AngleDegrees = AngleRadians;

            // 主雷达上绘制玩家
            if (GameData.Radar.Visibility && GameData.Config.Radar.Main.ShowPlayer)
            {
                FVector2D RadarLocation = Radar::WorldToRadarLocation(Player.Location);

                RenderHelper::DrawRadarPlayerCircleWithText(
                    Player.Type == EntityType::AI ? "AI" : std::to_string(Player.TeamID).c_str(),
                    PlayerColors.teamNumberColor,
                    10 * (GameData.Config.Radar.Main.FontSize / 14.f),
                    { RadarLocation.X, RadarLocation.Y },
                    AngleDegrees,
                    Player.RadarState
                );
            }

            // 小雷达上绘制玩家
            if (GameData.Radar.MiniRadarVisibility && Player.Distance < GameData.Radar.MiniRadarDistance && GameData.Config.Radar.Mini.ShowPlayer)
            {
                FVector2D MiniRadarLocation = Radar::WorldToMiniRadarLocation(Player.Location);

                RenderHelper::DrawRadarPlayerCircleWithText(
                    Player.Type == EntityType::AI ? "AI" : std::to_string(Player.TeamID).c_str(),
                    PlayerColors.teamNumberColor,
                    10 * (GameData.Config.Radar.Mini.FontSize / 14.f),
                    { MiniRadarLocation.X, MiniRadarLocation.Y },
                    AngleDegrees,
                    Player.RadarState
                );
            }
        }
    }

    // 绘制玩家信息
    static void DrawPlayers(const std::unordered_map<uint64_t, Player>& Players)
    {
        if (!GameData.Config.ESP.Enable) return;

        std::vector<std::pair<uint64_t, Player>> PlayersVector(Players.begin(), Players.end());

        std::sort(PlayersVector.begin(), PlayersVector.end(), [](const std::pair<uint64_t, Player>& a, const std::pair<uint64_t, Player>& b) {
            return a.second.Distance > b.second.Distance;
            });

        static uint64_t lastPlayerRankListsSnapshot = 0;
        static std::unordered_map<std::string, PlayerRankList> PlayerRankLists{};
        const uint64_t playerRankListsNow = GetTickCount64();
        if (PlayerRankLists.empty() || playerRankListsNow - lastPlayerRankListsSnapshot >= 250) {
            PlayerRankLists = Data::GetPlayerRankLists();
            lastPlayerRankListsSnapshot = playerRankListsNow;
        }

        int M200PlayerCount = 0;  // 200米内敌人计数
        int PalyerBB = 500;       // 最近敌人距离初始值
        std::string DisNane = ""; // 最近敌人名称

        for (auto& Item : PlayersVector)
        {
            Player& Player = Item.second;

            // 基础过滤
            if (Player.IsMe || Player.IsMyTeam || (Player.State == CharacterState::Dead))
            {
                continue;
            }

            Player.Distance = GameData.Camera.Location.Distance(Player.Location) / 100.0f;

            // 距离过滤
            if (Player.Distance > GameData.Config.ESP.DistanceMax)
            {
                continue;
            }

            // 收集200米内敌人信息
            if (Player.Distance <= 200)
            {
                M200PlayerCount++;
                if (PalyerBB > Player.Distance)
                {
                    PalyerBB = Player.Distance;
                    DisNane = Player.Name;
                }
            }

            // 屏幕外跳过
            if (!Player.InScreen)
                continue;

            // 控制是否显示信息
            bool bShowInfo = (Player.Distance <= GameData.Config.ESP.InfoDistanceMax ||
                (GameData.Config.ESP.AimExpandInfo && GameData.AimBot.TargetPlayerInfo.Entity == Player.Entity));
            bool bShowWeapon = ((Player.Distance <= GameData.Config.ESP.WeaponDistanceMax && GameData.Config.ESP.Weapon) || bShowInfo) &&
                Player.WeaponID > 0;

            // 计算骨骼屏幕坐标
            FTransform PredictedCTW = Player.ComponentToWorld;
            float dt = GameData.WorldTimeSeconds - Player.LastUpdateTime;
            if (dt > 0.0f && dt < 0.5f) {
                PredictedCTW.Translation = Player.ComponentToWorld.Translation + (Player.Velocity * dt);
            }
            for (EBoneIndex Bone : SkeletonLists::Bones)
            {
                Player.Skeleton.LocationBones[Bone] = VectorHelper::GetBoneWithRotation(Player.Skeleton.Bones[Bone], PredictedCTW);
                Player.Skeleton.ScreenBones[Bone] = VectorHelper::WorldToScreen(Player.Skeleton.LocationBones[Bone]);
            }

            // 获取玩家颜色
            RenderHelper::PlayerColor PlayerColors = RenderHelper::GetPlayerColor(Player);
            ImColor UseColor = PlayerColors.infoUseColor;

            // 绘制骨骼
            if (GameData.Config.ESP.Skeleton &&
                (!GameData.Config.ESP.LockedHiddenBones ||
                    (GameData.AimBot.Target != Player.Entity || !GameData.AimBot.Lock)))
            {
                if (GameData.Config.ESP.VisibleCheck)
                {
                    if (Player.GroggyHealth < 99)
                    {
                        ImColor groggy = Utils::FloatToImColor(GameData.Config.ESP.Color.Groggy.Skeleton);
                        ImColor red = ImColor(255, 0, 0, 255);
                        PlayerColors.skeletonUseColor = Player.IsVisible ? red : groggy;
                    }
                    else if (!Player.IsVisible)
                    {
                        PlayerColors.skeletonUseColor = Utils::FloatToImColor(GameData.Config.ESP.Color.Default.Skeleton);
                    }
                }

                // 绘制骨骼20260311注释
                //DrawSkeleton(Player, PlayerColors.skeletonUseColor, GameData.Config.ESP.SkeletonWidth);

                // 获取头部骨骼的屏幕坐标
                FVector2D headBonePos = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead];
                ImVec2 boneScreenPos(headBonePos.X, headBonePos.Y);

                // 将 GameData.Config.ESP.Color.Ray.Line 转换为 ImColor
                ImColor rayColor = GetColorFromConfig(GameData.Config.ESP.Color.Ray.Line);

                // 绘制射线
                if (GameData.Config.ESP.PlayerLine)
                {
                    // 从主屏幕顶部中间到玩家头部骨骼位置绘制射线
                    RenderHelper::DrawLine(screenTopCenter, boneScreenPos, rayColor, GameData.Config.ESP.RayWidth);
                }

                // 锁定变色
                if (GameData.Config.ESP.suodingbianse)
                {
                    if ((GameData.AimBot.Target == Player.Entity && GameData.AimBot.Lock))
                    {
                        PlayerColors.skeletonUseColor = Utils::FloatToImColor(GameData.Config.ESP.Color.aim.Skeleton);
                    }
                    else if (!Player.IsVisible && !(Player.GroggyHealth < 99))
                    {
                        PlayerColors.skeletonUseColor = Utils::FloatToImColor(GameData.Config.ESP.Color.Default.Skeleton);
                    }
                }

                // 根据距离和设置选择骨骼绘制方式
                if (GameData.Config.ESP.AdjustableDistance)
                {
                    DrawSkeleton_Physx(Player, GameData.Config.ESP.SkeletonWidth); // 如果在可调距离范围内，则绘制骨骼
                }
                else if (GameData.Config.ESP.Skeleton && (!GameData.Config.ESP.LockedHiddenBones || (GameData.AimBot.Target != Player.Entity || !GameData.AimBot.Lock)))
                {
                    DrawSkeleton(Player, PlayerColors.skeletonUseColor, GameData.Config.ESP.SkeletonWidth);
                }
            }

            if (Player.WeaponName == "手雷" && (int)Player.WeaponClassByte == 10 && Player.ElapsedCookingTime > 0.0f)
            {
                float remaining = 5.0f - Player.ElapsedCookingTime;
                if (remaining > 0.0f && remaining <= 5.0f)
                {
                    FVector2D handPos = Player.Skeleton.ScreenBones[EBoneIndex::Hand_R];
                    ImColor timerColor = Utils::FloatToImColor(GameData.Config.Project.ChareColor);
                    int timerFontSize = GameData.Config.Project.ChareFontSize > 0 ? GameData.Config.Project.ChareFontSize : GameData.Config.ESP.FontSize;
                    std::string countdownText = std::format("{:.1f}秒", remaining);
                    RenderHelper::StrokeText(countdownText.c_str(), { handPos.X, handPos.Y }, timerColor, timerFontSize, true, false);
                }
            }

            // 计算绘制位置
            ImVec2 Pos = {
    (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::Root].X,
    (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::ForeHead].Y - 19.0f  // 与队标水平对齐
            };

            int FontSize = GameData.Config.ESP.FontSize;
            int RankIcon = (int)(FontSize * 1.64f);
            auto Rect = Get2DBox(Player);

            // 获取头部位置
            FVector2D Headpos = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead];

            // 计算玩家高度和宽度比例
            const auto h = (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::Foot_L].Y - (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::ForeHead].Y;
            const auto w = (h / 4.0f) * 2.5f;
            const auto x = (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::Root].X - (w / 2.0f);
            const auto y = (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::ForeHead].Y;

            const float Scale = (float)FontSize / 14.f;
            ImVec2 HealthBarPos, HealthBarSize;
            auto health_xPos = NULL;

            // 血条绘制
            float Health = Player.State == CharacterState::Groggy ? Player.GroggyHealth : Player.Health;
            if (Player.State == CharacterState::Groggy) UseColor = Utils::FloatToImColor(GameData.Config.ESP.Color.Groggy.Info);

            constexpr auto health_bar_width = 20.0f; // 血条的固定宽度
            constexpr auto health_bar_height = 4.0f;  // 血条的固定高度
            constexpr auto example_health_max_value = 100.0f;

            // 只有在配置中启用血条时才绘制血条
            if (GameData.Config.ESP.health_bar && bShowInfo)
            {
                // 确保血量在 0 到 100 范围内
                const auto clamped_health = std::clamp(Health, 0.0f, 100.0f);
                const auto health_fill_width = (health_bar_width * clamped_health) / example_health_max_value;
                const auto health_x = x + (w - health_bar_width) / 2.0f;
                const auto health_y = y - 5.0f;

                // 检查是否绘制顶部血条
                if (GameData.Config.ESP.血条位置 == 0) // 血条显示在顶部
                {
                    if (GameData.Config.ESP.血条样式 == 0) // 彩虹血条
                    {
                        constexpr auto health_bar_width = 20.0f; // 血条的固定宽度
                        constexpr auto health_bar_height = 4.0f;  // 血条的固定高度

                        const auto color_1 = ImColor{ 255, 0, 0 };
                        const auto color_2 = ImColor{ 255, 128, 0 };
                        const auto color_3 = ImColor{ 0, 255, 0 };
                        const auto health_x = x + (w - health_bar_width) / 2.0f;
                        health_xPos = health_x;

                        // 绘制血条背景
                        RenderHelper::window_filled_rect({ health_x, y - 5.0f }, { health_bar_width, health_bar_height }, { 30, 30, 30 });

                        const auto health_fill_width = (health_bar_width * Health) / example_health_max_value;

                        // 根据健康值设置彩虹颜色
                        const auto rainbow_bar = [](const ImVec2& position, const ImVec2& size, const ImColor& color1, const ImColor& color2, const ImColor& color3, const ImColor& color4) {
                            RenderHelper::window_filled_rect_multicolor_horizontal(position, { size.x / 2.0f, size.y }, color1, color2);
                            RenderHelper::window_filled_rect_multicolor_horizontal({ position.x + size.x / 2.0f, position.y }, { size.x / 2.0f, size.y }, color3, color4);
                            };

                        rainbow_bar(
                            { health_x + 1.0f, y - 4.0f },
                            { health_fill_width - 2.0f, health_bar_height - 1.0f },
                            color_1,
                            Health < 30.0f ? color_1 : color_2,
                            Health < 30.0f ? color_1 : color_2,
                            Health < 60.0f ? color_2 : color_3
                        );
                    }
                    else if (GameData.Config.ESP.血条样式 == 1) // 单色血条
                    {
                        constexpr auto health_bar_width = 20.0f;
                        constexpr auto health_bar_height = 4.0f;

                        const auto color_3 = GetColorFromConfig(GameData.Config.ESP.Color.xuetiaoyanse.Skeleton);
                        const auto health_x = x + (w - health_bar_width) / 2.0f;
                        health_xPos = health_x;

                        // 绘制血条背景
                        RenderHelper::window_filled_rect({ health_x, y - 5.0f }, { health_bar_width, health_bar_height }, { 30, 30, 30 });

                        const auto health_fill_width = (health_bar_width * Health) / example_health_max_value;

                        // 绘制单色血条
                        const auto rainbow_bar = [](const ImVec2& position, const ImVec2& size, const ImColor& color1, const ImColor& color2, const ImColor& color3, const ImColor& color4) {
                            RenderHelper::window_filled_rect_multicolor_horizontal(position, { size.x / 2.0f, size.y }, color1, color2);
                            RenderHelper::window_filled_rect_multicolor_horizontal({ position.x + size.x / 2.0f, position.y }, { size.x / 2.0f, size.y }, color3, color4);
                            };

                        rainbow_bar(
                            { health_x + 1.0f, y - 4.0f },
                            { health_fill_width - 2.0f, health_bar_height - 1.0f },
                            color_3,
                            color_3,
                            color_3,
                            color_3
                        );
                    }
                    else if (GameData.Config.ESP.血条样式 == 2) // 宽单色血条
                    {
                        auto example_health_value = Health;
                        const auto green_color = GetColorFromConfig(GameData.Config.ESP.Color.xuetiaoyanse.Skeleton);

                        auto nickname_label_size = ImVec2{ w, 6.0f * 3.0f };

                        const auto health_w = (nickname_label_size.x < w) ? w : nickname_label_size.x;
                        const auto health_x = x + (w - health_w) / 2.0f;
                        health_xPos = health_x;

                        // 绘制血条背景
                        RenderHelper::window_filled_rect({ health_x, y - 5.0f }, { health_w, 3.0f }, { 30, 30, 30 });

                        // 绘制绿色单色血条
                        RenderHelper::window_filled_rect_multicolor_horizontal(
                            { health_x + 1.0f, y - 4.0f },
                            { ((health_w - 2.0f) * example_health_value) / example_health_max_value, 2.0f },
                            green_color, green_color  // 使用固定绿色，不受健康度变化影响
                        );
                    }
                    else if (GameData.Config.ESP.血条样式 == 3) // 宽彩虹血条
                    {
                        auto example_health_value = Health;
                        const auto color_1 = ImColor{ 255, 0, 0 };
                        const auto color_2 = ImColor{ 255, 128, 0 };
                        const auto color_3 = ImColor{ 0, 255, 0 };
                        auto nickname_label_size = ImVec2{ w, 6.0f * 3.0f };

                        const auto health_w = (nickname_label_size.x < w) ? w : nickname_label_size.x;
                        const auto health_x = x + (w - health_w) / 2.0f;
                        health_xPos = health_x;

                        RenderHelper::window_filled_rect({ health_x, y - 5.0f }, { health_w, 3.0f }, { 30, 30, 30 });

                        const auto rainbow_bar = [](const ImVec2& position, const ImVec2& size, const ImColor& color1, const ImColor& color2, const ImColor& color3, const ImColor& color4)
                            {
                                RenderHelper::window_filled_rect_multicolor_horizontal(position, { size.x / 2.0f, size.y }, color1, color2);
                                RenderHelper::window_filled_rect_multicolor_horizontal({ position.x + size.x / 2.0f, position.y }, { size.x / 2.0f, size.y }, color3, color4);
                            };

                        rainbow_bar(
                            { health_x + 1.0f, y - 4.0f }, { ((health_w - 2.0f) * example_health_value) / example_health_max_value, 2.0f },
                            color_1,
                            example_health_value < 30.0f ? color_1 : color_2,
                            example_health_value < 30.0f ? color_1 : color_2,
                            example_health_value < 60.0f ? color_2 : color_3
                        );
                    }
                }
                else if (GameData.Config.ESP.血条位置 == 1) // 血条显示在左侧
                {
                    // 绘制左侧血条
                    HealthBarPos = { Rect.x - 6, Rect.y };
                    HealthBarSize = { std::clamp(Rect.w / 4.f, 0.f, 4.f), Rect.w - 1 };
                    DrawHealthBar(Player.Entity, 100.f, Health, HealthBarPos, HealthBarSize, UseColor, false);
                    health_xPos = HealthBarPos.x;
                }
            }

            // 玩家信息文本处理
            std::string InfoText = "";

            // 获取队伍ID
            std::string TeamID = std::to_string(Player.TeamID);
            if (Player.Type == EntityType::AI)
            {
                TeamID = "AI";
            }

            // 计算战队标签部分
            float clanPrefixWidth = 0.0f;
            if (GameData.Config.ESP.ClanName && bShowInfo && Player.ClanName != "") {
                std::string clanPrefix = std::format("[{}] ", Player.ClanName);
                clanPrefixWidth = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.0f,
                    Utils::StringToUTF8(clanPrefix).c_str()).x;
                InfoText += clanPrefix;
            }

            // 计算名字部分
            float nameWidth = 0.0f;
            if (GameData.Config.ESP.Nickname && bShowInfo) {
                nameWidth = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.0f,
                    Utils::StringToUTF8(Player.Name).c_str()).x;
                InfoText += Player.Name;
            }

            // 段位和KDA数据
            PlayerRankInfo PlayerRankData;
            PlayerRankList temp = Data::GetPlayerSegmentListsItem(Player.Name);

            // 获取排位模式数据用于显示
            switch (GameData.Config.PlayerList.RankMode) {
            case 1: PlayerRankData = temp.TPP; break;
            case 2: PlayerRankData = temp.SquadTPP; break;
            case 3: PlayerRankData = temp.FPP; break;
            case 4: PlayerRankData = temp.SquadFPP; break;
            default: break;
            }

            // 获取所有模式中最高的 KD 值
            float maxKDA = PlayerRankData.KDA;
            if (temp.TPP.KDA > maxKDA) maxKDA = temp.TPP.KDA;
            if (temp.FPP.KDA > maxKDA) maxKDA = temp.FPP.KDA;
            if (temp.SquadTPP.KDA > maxKDA) maxKDA = temp.SquadTPP.KDA;
            if (temp.SquadFPP.KDA > maxKDA) maxKDA = temp.SquadFPP.KDA;

            // 计算实力判断文本
            std::string skillText = "";
            ImColor skillColor;
            bool hasSkillData = false;

            // 使用maxKDA来判断实力
            if (maxKDA > 0) {
                hasSkillData = true;
                if (maxKDA < 1.5f) {
                    skillText = "绿玩";
                    skillColor = IM_COL32(0, 255, 0, 255);  // 绿色
                } else if (maxKDA >= 1.5f && maxKDA < 2.3f) {
                    skillText = "高手";
                    skillColor = IM_COL32(255, 165, 0, 255);  // 橙色
                } else if (maxKDA >= 2.3f && maxKDA < 3.5f) {
                    skillText = "挂逼";
                    skillColor = IM_COL32(255, 182, 193, 255);  // 浅粉红色
                } else if (maxKDA >= 3.5f) {
                    skillText = "老挂逼";
                    skillColor = IM_COL32(255, 0, 0, 255);  // 红色
                }
            } else {
                hasSkillData = true;
                skillText = "未知";
                skillColor = IM_COL32(65, 105, 225, 255);  // 皇家蓝色
            }

            // 队伍编号位置
            if (GameData.Config.ESP.TeamID && bShowInfo) {
                // 基准位置计算
                float baseX = Pos.x - nameWidth / 2.0f - 2.0f - 15.0f;
                if (clanPrefixWidth > 0) {
                    baseX -= clanPrefixWidth / 2.0f;
                }
                float teamIdX = baseX;
                float teamIdY = Pos.y +8;  // 使队标与文本在同一水平线

                RenderHelper::DrawRadarPlayerCircleWithText(
                    Player.Type == EntityType::AI ? "AI" : std::to_string(Player.TeamID).c_str(),
                    PlayerColors.teamNumberColor,
                    FontSize / 2.f,
                    { teamIdX, teamIdY },
                    0,
                    ECharacterIconType::Normal,
                    true
                );
            }

            // 删除首尾空白
            Utils::Trim(InfoText);

            // 记录名称显示前的Y位置
            float beforeNameY = Pos.y;
            ImVec2 HeadInfoSize(0, 0);

            // 显示名称信息
            if (InfoText != "") {
                HeadInfoSize = RenderHelper::StrokeText(
                    Utils::StringToUTF8(InfoText).c_str(),
                    { Pos.x, Pos.y },
                    PlayerColors.infoUseColor,
                    FontSize,
                    true,
                    false  // 不更新Y位置
                );
                Pos.y += HeadInfoSize.y;  // 手动更新位置
            }

            // 在名字后显示实力判断
            if (hasSkillData && bShowInfo && GameData.Config.ESP.Nickname) {
                ImVec2 textSize = RenderHelper::StrokeText(
                    Utils::StringToUTF8(skillText).c_str(),
                    { Pos.x + nameWidth / 2.0f + 5.0f, beforeNameY },
                    skillColor,
                    FontSize,
                    false,
                    false
                );
            }

            // 手持武器图标绘制
            if (GameData.Config.ESP.Weapon && bShowWeapon)
            {
                std::string IconUrl = "Assets/image/Weapon/" + Player.WeaponName + ".png";
                if (GImGuiTextureMap[IconUrl].Width > 0) {
                    float TargetHeight = 15.f * Scale;
                    float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                    float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom;
                    float IconHeight = TargetHeight;

                    // 将武器图标移到名字上方
                    float weaponY = Pos.y - IconHeight - 15.0f;
                    float weaponX = Pos.x - IconWidth / 2;

                    RenderHelper::Image(
                        GImGuiTextureMap[IconUrl].Texture,
                        ImVec2(weaponX, weaponY),
                        ImVec2(IconWidth, IconHeight),
                        true,
                        PlayerColors.infoUseColor
                    );

                    if (GameData.Config.ESP.Ammo) {
                        std::string ammoText;
                        ImColor ammoColor;

                        // Define special weapon sets
                        bool isThrowable = (Player.WeaponName == "手雷" || Player.WeaponName == "烟雾弹" || Player.WeaponName == "闪光弹" || Player.WeaponName == "燃烧瓶" ||
                            Player.WeaponName == "C4" || Player.WeaponName == "诱饵手雷" || Player.WeaponName == "蓝圈手雷" || Player.WeaponName == "粘性炸弹");
                        bool isSpecial = (Player.WeaponName == "迫击炮" || Player.WeaponName == "火箭筒");

                        bool reloading = Player.IsReloading || (isThrowable && Player.ElapsedCookingTime > 0.01f);
                        if (reloading) {
                            if (isThrowable) {
                                ammoText = Localize((const char*)u8"拉栓", "Pin Pulled");
                                ammoColor = IM_COL32(255, 50, 50, 255);
                            }
                            else {
                                ammoText = Localize((const char*)u8"装填中", "Reloading");
                                ammoColor = IM_COL32(255, 50, 50, 255);
                            }
                        }
                        else {
                            if (isThrowable || isSpecial) {
                                // Don't show ammo count for throwables/special weapons
                                ammoText = "";
                            }
                            else if (Player.AmmoCount < 0 || Player.AmmoCount > 999) {
                                // Filter out garbage values
                                ammoText = "";
                            }
                            else {
                                ammoText = std::to_string(Player.AmmoCount);
                            }
                            ammoColor = IM_COL32(255, 200, 0, 255);
                        }

                        if (!ammoText.empty()) {
                            RenderHelper::StrokeText(ammoText.c_str(), { weaponX + IconWidth + 5.0f, weaponY + IconHeight / 2.0f - 7.0f }, ammoColor, 14, false, false);
                        }
                    }
                }
            }

            // 段位图标绘制
            if (GameData.Config.ESP.showico && bShowInfo)
            {
                std::string IconUrl = "Assets/image/RankImage/" + PlayerRankData.Tier + "-" + PlayerRankData.SubTier + ".png";
                if (PlayerRankData.Tier == "Master")
                    IconUrl = "Assets/image/RankImage/" + PlayerRankData.Tier + ".png";
                if (PlayerRankData.Tier == "")
                    IconUrl = "Assets/image/RankImage/Unranked.png";

                if (GImGuiTextureMap[IconUrl].Width > 0)
                {
                    // 计算 RankIcon 的位置（默认在玩家头顶，可调整）
                    float rankX = Pos.x - (RankIcon / 2);  // 居中
                    float rankY = Pos.y - RankIcon - 10.0f; // 默认在玩家头顶上方

                    // 如果武器图标开启，则 RankIcon 显示在武器图标上方
                    if (GameData.Config.ESP.Weapon && bShowWeapon && GImGuiTextureMap["Assets/image/Weapon/" + Player.WeaponName + ".png"].Width > 0)
                    {
                        float weaponHeight = 15.f * Scale;
                        rankY = Pos.y - weaponHeight - RankIcon - 8.0f - 4.0f; // 武器上方 + 额外间距
                    }

                    RenderHelper::Image(
                        GImGuiTextureMap[IconUrl].Texture,
                        ImVec2(rankX, rankY),
                        ImVec2(RankIcon, RankIcon),
                        false
                    );
                }
            }

            // 重置位置到玩家根部
            Pos.x = (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::Root].X;
            Pos.y = (float)(int)Player.Skeleton.ScreenBones[EBoneIndex::Root].Y;

            // 构建距离和血量信息
            if (GameData.Config.ESP.Dis || (GameData.Config.ESP.Health && bShowInfo))
            {
                InfoTextBuilder textBuilder;

                // 添加距离信息
                if (GameData.Config.ESP.Dis) {
                    textBuilder.add(std::format("{}M", (int)Player.Distance));
                }

                // 添加血量信息
                if (GameData.Config.ESP.Health && bShowInfo) {
                    textBuilder.add(std::format("{}HP", (int)Player.Health));
                }

                // 显示组合信息
                if (!textBuilder.isEmpty()) {
                    DisplayInfoText(textBuilder.build(), Pos, PlayerColors.infoUseColor, FontSize, true, true);
                }
            }

            // 绘制方框
            if (GameData.Config.ESP.DisplayFrame)
            {
                RenderHelper::Line(ImVec2(Rect.x, Rect.y), ImVec2(Rect.x + Rect.z, Rect.y), PlayerColors.skeletonUseColor, 2.0f);// 上横
                RenderHelper::Line(ImVec2(Rect.x, Rect.y), ImVec2(Rect.x, Rect.y + Rect.z * 1.6), PlayerColors.skeletonUseColor, 2.0f);// 左竖
                RenderHelper::Line(ImVec2(Rect.x + Rect.z, Rect.y), ImVec2(Rect.x + Rect.z, Rect.y + Rect.z * 1.6), PlayerColors.skeletonUseColor, 2.0f);// 右竖
                RenderHelper::Line(ImVec2(Rect.x, Rect.y + Rect.z * 1.6), ImVec2(Rect.x + Rect.z, Rect.y + Rect.z * 1.6), PlayerColors.skeletonUseColor, 2.0f);// 下横
            }

            // 显示合作者信息
            if (GameData.Config.ESP.Partner && Player.PartnerLevel > 0 && bShowInfo) {
                std::string partnerText = Localize(
                    std::format("合作者Lv:{}", (int)Player.PartnerLevel),
                    std::format("Partner Lv:{}", (int)Player.PartnerLevel)
                );

                DisplayInfoText(partnerText, Pos, PlayerColors.infoUseColor, FontSize, true, true);
            }

            // 显示等级和伤害信息
            if ((GameData.Config.ESP.等级 && bShowInfo) || (GameData.Config.ESP.伤害 && Player.DamageDealtOnEnemy > 0 && bShowInfo))
            {
                InfoTextBuilder textBuilder;

                // 添加等级信息
                if (GameData.Config.ESP.等级 && bShowInfo) {
                    textBuilder.add(Localize(
                        std::format("等级:{} ", (int)Player.SurvivalLevel),
                        std::format("Lv:{} ", (int)Player.SurvivalLevel)
                    ));
                }

                // 添加伤害信息
                if (GameData.Config.ESP.伤害 && Player.DamageDealtOnEnemy > 0 && bShowInfo) {
                    textBuilder.add(Localize(
                        std::format("伤害:{}", (int)Player.DamageDealtOnEnemy),
                        std::format("D:{}", (int)Player.DamageDealtOnEnemy)
                    ));
                }

                // 显示组合信息
                if (!textBuilder.isEmpty()) {
                    DisplayInfoText(textBuilder.build(), Pos, PlayerColors.infoUseColor, FontSize, true, true);
                }
            }

            // 显示击杀和观战信息
            if ((GameData.Config.ESP.击杀 && Player.KillCount > 0 && bShowInfo) || (GameData.Config.ESP.观战 && Player.SpectatedCount > 0 && bShowInfo))
            {
                InfoTextBuilder textBuilder;

                // 添加击杀信息
                if (GameData.Config.ESP.击杀 && Player.KillCount > 0 && bShowInfo) {
                    textBuilder.add(Localize(
                        std::format("击杀:{} ", (int)Player.KillCount),
                        std::format("K:{} ", (int)Player.KillCount)
                    ));
                }

                // 添加观战信息
                if (GameData.Config.ESP.观战 && Player.SpectatedCount > 0 && bShowInfo) {
                    textBuilder.add(Localize(
                        std::format("观战:{}", (int)Player.SpectatedCount),
                        std::format("G:{}", (int)Player.SpectatedCount)
                    ));
                }

                // 显示组合信息
                if (!textBuilder.isEmpty()) {
                    DisplayInfoText(textBuilder.build(), Pos, PlayerColors.infoUseColor, FontSize, true, true);
                }
            }

            // 显示掉线状态和倒地状态
            if ((GameData.Config.ESP.Offline && bShowInfo) || (GameData.Config.ESP.Downed && bShowInfo))
            {
                InfoTextBuilder textBuilder;

                // 检查掉线状态
                if (GameData.Config.ESP.Offline && bShowInfo && Player.CharacterState == ECharacterState::Offline) {
                    textBuilder.add(Localize("[离线]", "[Offline]"));
                }

                if (GameData.Config.ESP.Downed && bShowInfo &&
                    (Player.State == CharacterState::Groggy ||
                     Player.CharacterState == ECharacterState::BeHit ||
                     Player.GroggyHealth < 99)) {
                    textBuilder.add(Localize("[倒地]", "[Downed]"));
                }

                // 显示组合信息
                if (!textBuilder.isEmpty()) {
                    DisplayInfoText(textBuilder.build(), Pos, PlayerColors.infoUseColor, FontSize, true, true);
                }
            }

            // 被瞄射线
            if (Player.IsAimMe && GameData.Config.ESP.TargetedRay)
            {
                auto IsAimMeX = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead].X;
                auto IsAimMeY = Player.Skeleton.ScreenBones[EBoneIndex::ForeHead].Y;

                if (!Player.InScreen)
                {
                    FVector2D LoctionToScreen = VectorHelper::WorldToScreen(Player.Location);
                    IsAimMeY = LoctionToScreen.Y;
                    IsAimMeY = LoctionToScreen.X;
                }

                if (!GameData.Config.ESP.VisibleCheckRay || Player.IsVisible)
                {
                    RenderHelper::Line(ImVec2(IsAimMeX, IsAimMeY),
                        ImVec2(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2),
                        ImGui::ColorConvertFloat4ToU32(Utils::FloatToImColor(GameData.Config.ESP.Color.Ray.Line)),
                        GameData.Config.ESP.RayWidth);
                }

            }

            //手持武器文字
            if (GameData.Config.ESP.WeaponText && bShowWeapon)
            {
                InfoText = "";

                InfoText += std::format("{}", Player.WeaponName);

                if (InfoText != "") {
                    ImVec2 HeadInfoSize = RenderHelper::StrokeText(Utils::StringToUTF8(InfoText).c_str(), { Pos.x, Pos.y }, PlayerColors.infoUseColor, FontSize, true, false);
                    Pos.y += HeadInfoSize.y - 2;
                }
            }

            // 段位查询
            if (bShowInfo)
            {
                // 只在段位不为空的情况下处理
                if (!PlayerRankData.TierToString.empty())
                {
                    // 获取本地化段位文本（PUBG）
                    std::string localizedTier;
                    if (Languages == 1) { // 中文
                        if (PlayerRankData.TierToString == U8("青铜1")) localizedTier = U8("青铜 1");
                        else if (PlayerRankData.TierToString == U8("青铜2")) localizedTier = U8("青铜 2");
                        else if (PlayerRankData.TierToString == U8("青铜3")) localizedTier = U8("青铜 3");
                        else if (PlayerRankData.TierToString == U8("青铜4")) localizedTier = U8("青铜 4");
                        else if (PlayerRankData.TierToString == U8("青铜5")) localizedTier = U8("青铜 5");
                        else if (PlayerRankData.TierToString == U8("白银1")) localizedTier = U8("白银 1");
                        else if (PlayerRankData.TierToString == U8("白银2")) localizedTier = U8("白银 2");
                        else if (PlayerRankData.TierToString == U8("白银3")) localizedTier = U8("白银 3");
                        else if (PlayerRankData.TierToString == U8("白银4")) localizedTier = U8("白银 4");
                        else if (PlayerRankData.TierToString == U8("白银5")) localizedTier = U8("白银 5");
                        else if (PlayerRankData.TierToString == U8("黄金1")) localizedTier = U8("黄金 1");
                        else if (PlayerRankData.TierToString == U8("黄金2")) localizedTier = U8("黄金 2");
                        else if (PlayerRankData.TierToString == U8("黄金3")) localizedTier = U8("黄金 3");
                        else if (PlayerRankData.TierToString == U8("黄金4")) localizedTier = U8("黄金 4");
                        else if (PlayerRankData.TierToString == U8("黄金5")) localizedTier = U8("黄金 5");
                        else if (PlayerRankData.TierToString == U8("白金1")) localizedTier = U8("白金 1");
                        else if (PlayerRankData.TierToString == U8("白金2")) localizedTier = U8("白金 2");
                        else if (PlayerRankData.TierToString == U8("白金3")) localizedTier = U8("白金 3");
                        else if (PlayerRankData.TierToString == U8("白金4")) localizedTier = U8("白金 4");
                        else if (PlayerRankData.TierToString == U8("白金5")) localizedTier = U8("白金 5");
                        else if (PlayerRankData.TierToString == U8("水晶1")) localizedTier = U8("水晶 1");
                        else if (PlayerRankData.TierToString == U8("水晶2")) localizedTier = U8("水晶 2");
                        else if (PlayerRankData.TierToString == U8("水晶3")) localizedTier = U8("水晶 3");
                        else if (PlayerRankData.TierToString == U8("水晶4")) localizedTier = U8("水晶 4");
                        else if (PlayerRankData.TierToString == U8("钻石1")) localizedTier = U8("钻石 1");
                        else if (PlayerRankData.TierToString == U8("钻石2")) localizedTier = U8("钻石 2");
                        else if (PlayerRankData.TierToString == U8("钻石3")) localizedTier = U8("钻石 3");
                        else if (PlayerRankData.TierToString == U8("钻石4")) localizedTier = U8("钻石 4");
                        else if (PlayerRankData.TierToString == U8("钻石5")) localizedTier = U8("钻石 5");
                        else if (PlayerRankData.TierToString == U8("大师1")) localizedTier = U8("大师");
                        else if (PlayerRankData.TierToString == U8("生存者1")) localizedTier = U8("生存者");
                        else if (PlayerRankData.TierToString == U8("未定级")) localizedTier = U8("没有KD");
                        else if (PlayerRankData.TierToString == U8("没有KD")) localizedTier = U8("实力不详");
                    }
                    else { // 英文
                        if (PlayerRankData.TierToString == U8("青铜1")) localizedTier = U8("Bronze 1");
                        else if (PlayerRankData.TierToString == U8("青铜2")) localizedTier = U8("Bronze 2");
                        else if (PlayerRankData.TierToString == U8("青铜3")) localizedTier = U8("Bronze 3");
                        else if (PlayerRankData.TierToString == U8("青铜4")) localizedTier = U8("Bronze 4");
                        else if (PlayerRankData.TierToString == U8("青铜5")) localizedTier = U8("Bronze 5");
                        else if (PlayerRankData.TierToString == U8("白银1")) localizedTier = U8("Silver 1");
                        else if (PlayerRankData.TierToString == U8("白银2")) localizedTier = U8("Silver 2");
                        else if (PlayerRankData.TierToString == U8("白银3")) localizedTier = U8("Silver 3");
                        else if (PlayerRankData.TierToString == U8("白银4")) localizedTier = U8("Silver 4");
                        else if (PlayerRankData.TierToString == U8("白银5")) localizedTier = U8("Silver 5");
                        else if (PlayerRankData.TierToString == U8("黄金1")) localizedTier = U8("Gold 1");
                        else if (PlayerRankData.TierToString == U8("黄金2")) localizedTier = U8("Gold 2");
                        else if (PlayerRankData.TierToString == U8("黄金3")) localizedTier = U8("Gold 3");
                        else if (PlayerRankData.TierToString == U8("黄金4")) localizedTier = U8("Gold 4");
                        else if (PlayerRankData.TierToString == U8("黄金5")) localizedTier = U8("Gold 5");
                        else if (PlayerRankData.TierToString == U8("白金1")) localizedTier = U8("Platinum 1");
                        else if (PlayerRankData.TierToString == U8("白金2")) localizedTier = U8("Platinum 2");
                        else if (PlayerRankData.TierToString == U8("白金3")) localizedTier = U8("Platinum 3");
                        else if (PlayerRankData.TierToString == U8("白金4")) localizedTier = U8("Platinum 4");
                        else if (PlayerRankData.TierToString == U8("白金5")) localizedTier = U8("Platinum 5");
                        else if (PlayerRankData.TierToString == U8("水晶1")) localizedTier = U8("Crystal 1");
                        else if (PlayerRankData.TierToString == U8("水晶2")) localizedTier = U8("Crystal 2");
                        else if (PlayerRankData.TierToString == U8("水晶3")) localizedTier = U8("Crystal 3");
                        else if (PlayerRankData.TierToString == U8("水晶4")) localizedTier = U8("Crystal 4");
                        else if (PlayerRankData.TierToString == U8("钻石1")) localizedTier = U8("Diamond 1");
                        else if (PlayerRankData.TierToString == U8("钻石2")) localizedTier = U8("Diamond 2");
                        else if (PlayerRankData.TierToString == U8("钻石3")) localizedTier = U8("Diamond 3");
                        else if (PlayerRankData.TierToString == U8("钻石4")) localizedTier = U8("Diamond 4");
                        else if (PlayerRankData.TierToString == U8("钻石5")) localizedTier = U8("Diamond 5");
                        else if (PlayerRankData.TierToString == U8("大师1")) localizedTier = U8("Master");
                        else if (PlayerRankData.TierToString == U8("生存者1")) localizedTier = U8("Survivor");
                        else if (PlayerRankData.TierToString == U8("未定级")) localizedTier = U8("Unranked");
                        else if (PlayerRankData.TierToString == U8("没有KD")) localizedTier = U8("Unranked");
                    }

                    // 准备文本内容
                    std::string tierText = localizedTier; // 段位文字
                    std::string kdaText = "";

                    // 处理KDA显示逻辑 - 使用所有模式中最高的 KD 值
                    if (PlayerRankData.Tier == "") {
                        // 不显示KD值，避免显示0.00
                    }
                    else if (maxKDA > 0.0f) {
                        std::ostringstream kdaStream;
                        kdaStream << std::fixed << std::setprecision(2) << maxKDA;
                        kdaText = std::format("  [{}]", kdaStream.str());
                    }

                    // 合并文本
                    std::string combinedText = tierText + kdaText;

                    // 检查是否显示没有KD，如果是则使用皇家蓝色
                    ImColor tierColor = PlayerColors.infoUseColor;
                    if (PlayerRankData.Tier == "" || PlayerRankData.TierToString == U8("没有KD") || PlayerRankData.TierToString == U8("未定级")) {
                        tierColor = IM_COL32(65, 105, 225, 255);  // 皇家蓝色
                    }

                    // 绘制文本（确保使用支持中文的字体）
                    ImVec2 textSize = RenderHelper::StrokeText(
                        combinedText.c_str(),  // 直接使用原始字符串，不使用Utils::StringToUTF8转换
                        { Pos.x, Pos.y },
                        tierColor,
                        FontSize,
                        true,  // 居中
                        false
                    );

                    // 调整下一项位置
                    Pos.y += textSize.y - 3;
                }
            }
        }

        // 危险预警
        //if (GameData.Config.ESP.DangerWarning)
        //{
        //    // 计算相对位置（例如，固定在窗口的右下角）
        //    float posX = GameData.Config.Overlay.ScreenWidth / 2; // 相对于窗口宽度的中心
        //    float posY = GameData.Config.Overlay.ScreenHeight - 150; // 距底部150像素

        //    // 有敌人时显示预警信息
        //    if (M200PlayerCount > 0) {
        //        std::string warningText = std::format(
        //            "{}: {}  {}: {}M  {}: {}",
        //            Localize("200米范围内敌人数量", "Enemies within 200 meters"), M200PlayerCount,
        //            Localize("离你最近的敌人", "Nearest enemy"), PalyerBB,
        //            Localize("最近的玩家名字", "Nearest player name"), DisNane
        //        );

        //        RenderHelper::StrokeText(
        //            Utils::StringToUTF8(warningText).c_str(),
        //            ImVec2(posX, posY),
        //            IM_COL32(255, 0, 0, 255),
        //            17.0,
        //            true,
        //            true
        //        );
        //    }
        //}
    }

    // 危险预警
    static void DrawPlayersEarly(const std::unordered_map<uint64_t, Player>& Players)
    {
        if (!GameData.Config.ESP.DangerWarning) return;

        const float Scale = (float)GameData.Config.Early.FontSize / 7.f;
        float Size = 200.f * Scale;
        float AngleXSize = 2.f * Scale;
        float AngleYSize = 5.f * Scale;
        int EnemyCountInRange = 0;
        bool IsanyAimMe = false;
        int EnemyCountInCloseRange = 0; // 50米内的敌人数量
        float ClosestEnemyDistance = FLT_MAX;
        uint64_t ClosestEnemyTeamID = 0; // 最近敌人的队伍ID
        float detectionRange = GameData.Config.Early.DistanceMax;
        for (const auto& Item : Players)
        {
            const Player& Player = Item.second;

            if (Player.IsMe || Player.IsMyTeam || (Player.State == CharacterState::Dead))
            {
                continue;
            }

            // 计算距离
            float Distance = (GameData.Camera.Location).Distance(Player.Location) / 100;

            // 检查200米内是否有敌人
            if (Distance <= detectionRange && !Player.InFog)
            {
                EnemyCountInRange++;
                if (Distance < ClosestEnemyDistance)
                {
                    ClosestEnemyDistance = Distance;
                    ClosestEnemyTeamID = Player.TeamID; // 记录最近敌人的队伍ID
                }
            }

            // 检查50米内是否有敌人
            if (Distance <= 50.0f && !Player.InFog)
            {
                EnemyCountInCloseRange++;
            }

            // 判断是否瞄准自己
            FVector Direction = GameData.LocalPlayerInfo.Location - Player.Location;
            float Length = Direction.Length();
            FVector DirectionToMe = (Length > 0) ? Direction * (1.0f / Length) : FVector(0, 0, 0);

            FVector EnemyAimDirection = Player.AimOffsets.GetUnitVector();
            float DotProduct = EnemyAimDirection.DotProduct(DirectionToMe);
            float Angle = std::acos(DotProduct) * (180.0f / 3.14159265358979323846f);

            if (Angle <= 2.0f && Distance <= detectionRange && !Player.InFog && !Player.InScreen) // 5° 是一个合理的阈值
            {
                IsanyAimMe = true;
            }
        }

        if (EnemyCountInRange > 0)
        {
            std::string infoText = std::format("| {}米内有{}个敌人 | 最近{}米 | {}队 |",
                (int)detectionRange,
                EnemyCountInRange,
                (int)ClosestEnemyDistance,
                static_cast<uint32_t>(ClosestEnemyTeamID));

            std::string utf8Text = Utils::StringToUTF8(infoText);
            ImVec2 textSize = ImGui::CalcTextSize(utf8Text.c_str());
            ImVec2 textPos(
                GameData.Config.Overlay.ScreenWidth / 2 - textSize.x / 2 - 80.f,
                GameData.Config.Overlay.ScreenHeight - textSize.y + 10.0f
            );

            ImU32 textColor = (EnemyCountInCloseRange > 0)
                ? IM_COL32(255, 96, 96, 255)
                : IM_COL32(0, 255, 0, 255);

            RenderHelper::Text(
                utf8Text.c_str(),
                textPos,
                textColor,
                30,
                false,
                true
            );
        }
        // -----------------------------------------------
        // 2. 被瞄准警告（闪烁效果）
        // -----------------------------------------------

        if (IsanyAimMe) {
            // 生成文本
            std::string warningText = "！！注意！！小心背后有人对准了你！！";

            // 转换为 UTF-8 编码
            std::string utf8Text = Utils::StringToUTF8(warningText);

            // 计算文本尺寸
            ImVec2 textSize = ImGui::CalcTextSize(utf8Text.c_str());

            // 计算位置（准星正下方）
            ImVec2 warningPos(
                GameData.Config.Overlay.ScreenWidth / 2 - textSize.x / 2, // 水平居中
                GameData.Config.Overlay.ScreenHeight / 2 + 55.0f // 准星下方35像素
            );

            // 渲染文本（红色字体）
            RenderHelper::Text(
                utf8Text.c_str(),
                warningPos,
                IM_COL32(255, 0, 0, 255), // 红色
                30.0, // 稍大字体
                false, // 是否居中（已手动计算居中位置）
                true  // 是否启用阴影
            );
        }

    

        {
            // 绘制预警指示器
            if (!GameData.Config.Early.Enable) return;
            const float Scale = (float)GameData.Config.Early.FontSize / 14.f;
            float Size = 200.f * Scale;
            float AngleXSize = 2.f * Scale;
            float AngleYSize = 5.f * Scale;

            for (const auto& Item : Players)
            {
                const Player& Player = Item.second;

                if (Player.IsMe || Player.IsMyTeam || (Player.State == CharacterState::Dead))
                {
                    continue;
                }

                float Distance = (GameData.Camera.Location).Distance(Player.Location) / 100;
                float Angle = atan2(GameData.Camera.Location.Y - Player.Location.Y, GameData.Camera.Location.X - Player.Location.X);
                Angle = Angle * 180 / M_PI;
                Angle = GameData.Camera.Rotation.Yaw + 360 - Angle; // Note: Y is actually X due to storage order differences

                ImVec2 A, B, C;
                A.x = (Size + AngleYSize) * sin(Angle * M_PI / 180) + GameData.Config.Overlay.ScreenWidth / 2;
                A.y = (Size + AngleYSize) * cos(Angle * M_PI / 180) + GameData.Config.Overlay.ScreenHeight / 2;

                B.x = (Size - AngleYSize) * sin((Angle - AngleXSize) * M_PI / 180) + GameData.Config.Overlay.ScreenWidth / 2;
                B.y = (Size - AngleYSize) * cos((Angle - AngleXSize) * M_PI / 180) + GameData.Config.Overlay.ScreenHeight / 2;

                C.x = (Size - AngleYSize) * sin((Angle + AngleXSize) * M_PI / 180) + GameData.Config.Overlay.ScreenWidth / 2;
                C.y = (Size - AngleYSize) * cos((Angle + AngleXSize) * M_PI / 180) + GameData.Config.Overlay.ScreenHeight / 2;

                ImU32 Color = ImColor(255, 0, 0, 255);

                float MaxDistance = GameData.Config.Early.DistanceMax;
                float Segment = MaxDistance / 4.0f;

                if (Distance < MaxDistance)
                {
                    // 根据距离设置颜色
                    if (Distance >= Segment && Distance < Segment * 2)
                    {
                        Color = ImColor(255, 165, 0, 255); // 橙色
                    }
                    else if (Distance >= Segment * 2 && Distance < Segment * 3)
                    {
                        Color = ImColor(0, 255, 0, 255); // 绿色
                    }
                    else if (Distance >= Segment * 3)
                    {
                        Color = ImColor(255, 255, 255, 255); // 白色
                    }

                    // 显示距离文本
                    if (GameData.Config.Early.ShowDistance) {
                        ImVec2 HeadInfoSize = RenderHelper::StrokeText(
                            std::format("{}M", (int)Distance).c_str(),
                            { A.x + 13 * Scale, A.y },
                            Color,
                            13 * Scale,
                            false,
                            true
                        );
                    }

                    // 绘制三角形指示器
                    RenderHelper::DrawTriangle(A, B, C, Color, Player.IsAimMe);
                }
            }
        }
    }

    // 绘制包裹信息（空投、死亡箱）
    static void DrawPackages(const std::unordered_map<uint64_t, PackageInfo>& Packages)
    {
        if (!GameData.Config.AirDrop.Enable && !GameData.Config.DeadBox.Enable) return;

        std::vector<std::pair<uint64_t, PackageInfo>> VectorItems(Packages.begin(), Packages.end());

        std::sort(VectorItems.begin(), VectorItems.end(), [](const std::pair<uint64_t, PackageInfo>& a, const std::pair<uint64_t, PackageInfo>& b) {
            return a.second.Distance > b.second.Distance;
            });

        for (auto& Item : VectorItems)
        {
            PackageInfo& Package = Item.second;

            // 处理空投
            if (Package.Type == EntityType::AirDrop)
            {
                if (Package.Distance > GameData.Config.AirDrop.DistanceMax || !GameData.Config.AirDrop.Enable) continue;

                Package.ScreenLocation = VectorHelper::WorldToScreen(Package.Location);
                Package.Distance = GameData.Camera.Location.Distance(Package.Location) / 100.0f;

                std::string Text = Utils::StringToUTF8(std::format(
                    "{} [{}{}]",
                    Localize("空投", "Airdrop"),
                    (int)Package.Distance,
                    Localize("M", "M")
                ));
                ImColor InfoColor = Utils::FloatToImColor(GameData.Config.AirDrop.Color);

                ImVec2 HeadInfoSize = RenderHelper::StrokeText(Text.c_str(), { Package.ScreenLocation.X, Package.ScreenLocation.Y }, InfoColor, GameData.Config.AirDrop.FontSize, false, false);

                // 显示空投内物品
                if (GameData.Config.AirDrop.ShowItems)
                {
                    int Index = 0;
                    int FistIndex = 0;
                    FVector2D FistPos;

                    for (auto& ItemInfo : Package.Items)
                    {
                        std::string Text = Utils::StringToUTF8(std::format("{}", ItemInfo.DisplayName));
                        int GroupIndex = GameData.Config.Item.Lists[ItemInfo.Name].Group;
                        ImColor TextColor = InfoColor;
                        float FontSize = GameData.Config.AirDrop.FontSize - 1;

                        // 根据物品分组设置颜色
                        switch (GroupIndex)
                        {
                        case 1:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupAColor);
                            break;
                        case 2:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupBColor);
                            break;
                        case 3:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupCColor);
                            break;
                        case 4:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupDColor);
                            break;
                        default:
                            continue;
                            break;
                        }

                        FistPos = Package.ScreenLocation;
                        FistPos.Y += HeadInfoSize.y;

                        ImVec2 ScreenLocation = { FistPos.X, FistPos.Y + ((FontSize + 1) * Index) };
                        ImVec2 TextSize = RenderHelper::StrokeText(Text.c_str(), { ScreenLocation.x, ScreenLocation.y }, TextColor, FontSize, false, false);

                        // 绘制物品图标
                        if (GameData.Config.Item.ShowIcon) {
                            float Scale = (FontSize / 14.f);
                            std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";
                            if (GImGuiTextureMap[IconUrl].Width > 0) {
                                float TargetHeight = 14.f * Scale;
                                float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                                float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom;
                                float IconHeight = TargetHeight;
                                ScreenLocation.x -= (FontSize + 4);
                                ScreenLocation.y -= 5.0f;
                                RenderHelper::Image(GImGuiTextureMap[IconUrl].Texture, ScreenLocation, ImVec2(IconWidth, IconHeight), false);
                            }
                        }

                        Index++;
                    }
                }
            }
            else {
                // 处理死亡箱
                if (Package.Distance > GameData.Config.DeadBox.DistanceMax || !GameData.Config.DeadBox.Enable) continue;

                Package.ScreenLocation = VectorHelper::WorldToScreen(Package.Location);
                Package.Distance = GameData.Camera.Location.Distance(Package.Location) / 100.0f;

                std::string Text = Utils::StringToUTF8(std::format(
                    "{} [{}{}]",
                    Localize("骨灰盒", "Ash Box"),
                    (int)Package.Distance,
                    Localize("M", "M")
                ));
                ImColor InfoColor = Utils::FloatToImColor(GameData.Config.DeadBox.Color);

                ImVec2 HeadInfoSize = RenderHelper::StrokeText(Text.c_str(), { Package.ScreenLocation.X, Package.ScreenLocation.Y }, InfoColor, GameData.Config.DeadBox.FontSize, false, false);

                // 显示死亡箱内物品
                if (GameData.Config.DeadBox.ShowItems)
                {
                    int Index = 0;
                    int FistIndex = 0;
                    FVector2D FistPos;

                    for (auto& ItemInfo : Package.Items)
                    {
                        std::string Text = Utils::StringToUTF8(std::format("{}", ItemInfo.DisplayName));
                        int GroupIndex = GameData.Config.Item.Lists[ItemInfo.Name].Group;
                        ImColor TextColor = InfoColor;
                        float FontSize = GameData.Config.DeadBox.FontSize - 1;

                        // 根据物品分组设置颜色
                        switch (GroupIndex)
                        {
                        case 1:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupAColor);
                            break;
                        case 2:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupBColor);
                            break;
                        case 3:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupCColor);
                            break;
                        case 4:
                            TextColor = Utils::FloatToImColor(GameData.Config.Item.GroupDColor);
                            break;
                        default:
                            continue;
                            break;
                        }

                        FistPos = Package.ScreenLocation;
                        FistPos.Y += HeadInfoSize.y + 2;

                        ImVec2 ScreenLocation = { FistPos.X, FistPos.Y + ((FontSize + 1) * Index) };
                        ImVec2 TextSize = RenderHelper::StrokeText(Text.c_str(), { ScreenLocation.x, ScreenLocation.y }, TextColor, FontSize, false, false);

                        // 绘制物品图标
                        if (GameData.Config.Item.ShowIcon) {
                            float Scale = (FontSize / 14.f);
                            std::string IconUrl = "Assets/image/All/" + ItemInfo.Name + ".png";
                            if (GImGuiTextureMap[IconUrl].Width > 0) {
                                float TargetHeight = 14.f * Scale;
                                float HeightZoom = TargetHeight / GImGuiTextureMap[IconUrl].Height;
                                float IconWidth = GImGuiTextureMap[IconUrl].Width * HeightZoom;
                                float IconHeight = TargetHeight;
                                ScreenLocation.x -= (FontSize + 4);
                                ScreenLocation.y -= 5.0f;
                                RenderHelper::Image(GImGuiTextureMap[IconUrl].Texture, ScreenLocation, ImVec2(IconWidth, IconHeight), false);
                            }
                        }

                        Index++;
                    }
                }
            }
        }
    }

    // 绘制网格调试数据
    static void DrawNextHintMesh() {
        auto meshPtr = std::atomic_load(&GameData.NextHintMeshData);
        if (meshPtr != nullptr && GameData.Config.ESP.PhysXDebug) {
            const auto& mesh = *meshPtr;
            if (!mesh.HasDebugMesh()) {
                return;
            }

            const auto& debugMesh = *mesh.DebugMesh;

            try {
                // 如果mesh有顶点数据则渲染
                if (!debugMesh.Vertices.empty()
                    && !debugMesh.Indices.empty() && static_cast<uint32_t>(mesh.Type) < 10
                    && static_cast<uint32_t>(mesh.Type) >= 0
                    && debugMesh.Vertices.size() < 1000000
                    && debugMesh.Indices.size() < 1000000)
                {
                    for (size_t i = 0; i < debugMesh.Indices.size(); i += 3)
                    {
                        if (i + 2 >= debugMesh.Indices.size())
                            break;

                        // 获取三角形的三个顶点
                        Vector3 v0 = debugMesh.Vertices[debugMesh.Indices[i]];
                        Vector3 v1 = debugMesh.Vertices[debugMesh.Indices[i + 1]];
                        Vector3 v2 = debugMesh.Vertices[debugMesh.Indices[i + 2]];

                        // 转换到屏幕坐标
                        auto p0 = VectorHelper::WorldToScreen((FVector&)v0, true);
                        auto p1 = VectorHelper::WorldToScreen((FVector&)v1, true);
                        auto p2 = VectorHelper::WorldToScreen((FVector&)v2, true);

                        if (i == 0) {
                            // 将flags转换为二进制字符串
                            std::string binary;
                            for (int bit = 31; bit >= 0; bit--) {
                                binary += ((mesh.Flags >> bit) & 1) ? '1' : '0';
                            }
                        }

                        // 检查是否在屏幕内
                        if (VectorHelper::IsInScreen(p0) || VectorHelper::IsInScreen(p1) || VectorHelper::IsInScreen(p2))
                        {
                            auto A = ImVec2(p0.X, p0.Y);
                            auto B = ImVec2(p1.X, p1.Y);
                            auto C = ImVec2(p2.X, p2.Y);
                            // 绘制三角形边框
                            RenderHelper::DrawTriangle(A, B, C, ImColor(0, 255, 0, 200));
                        }
                    }
                }
            }
            catch (...) {
                // 捕获任何异常但不处理
            }
        }
    }

    // 主ESP绘制函数
    static void DrawESP()
    {
        static uint64_t lastSceneSnapshotTick = 0;
        static std::unordered_map<uint64_t, Player> Players{};
        static std::unordered_map<uint64_t, VehicleInfo> Vehicles{};
        static std::unordered_map<uint64_t, ItemInfo> Items{};
        static std::unordered_map<uint64_t, ProjectInfo> Projects{};
        static std::unordered_map<uint64_t, PackageInfo> Packages{};

        // 融合模式处理
        if (GameData.Config.Overlay.FusionMode) {
            ImGui::GetBackgroundDrawList()->AddRectFilled({ 0 ,0 }, { (float)GameData.Config.Overlay.ScreenWidth, (float)GameData.Config.Overlay.ScreenHeight }, ImColor(0, 0, 0, 255));
        }

        // 只在游戏中绘制
        if (GameData.Scene != Scene::Gaming)
        {
            Players.clear();
            Vehicles.clear();
            Items.clear();
            Projects.clear();
            Packages.clear();
            lastSceneSnapshotTick = 0;
            return;
        }

        // 获取游戏数据快照，避免跟随渲染帧率重复拷贝大容器
        const uint64_t snapshotNow = GetTickCount64();
        if (Players.empty() || snapshotNow - lastSceneSnapshotTick >= 20)
        {
            Players = Data::GetPlayers();
            Vehicles = Data::GetVehicles();
            Packages = Data::GetPackages();

            if (GameData.Config.Item.Enable && !GameData.Config.ESP.FocusMode) {
                Items = Data::GetItems();
            }
            else {
                Items.clear();
            }

            if (GameData.Config.Project.Enable) {
                Projects = Data::GetProjects();
            }
            else {
                Projects.clear();
            }

            lastSceneSnapshotTick = snapshotNow;
        }

        // 当不显示鼠标且不锁定目标时绘制基础信息
        if (!GameData.bShowMouseCursor && !GameData.AimBot.Lock)
        {
            if (!GameData.Config.ESP.FocusMode)
            {
                DrawItems(Items);
                DrawVehicles(Vehicles);
                DrawPackages(Packages);
            }

            DrawFOV();
            DrawPlayersEarly(Players);
        }

        // 雷达未显示时绘制玩家
        if (!GameData.Radar.Visibility)
        {
            DrawPlayers(Players);
            DrawMortaring(Players, Vehicles);
        }

        // 其他元素绘制
        DrawLocalPlayerProject();
        DrawAimBotPoint();
        DrawProjects(Projects);
        DrawRadars(Players, Vehicles, Packages);
        DrawNextHintMesh();
    };
};

