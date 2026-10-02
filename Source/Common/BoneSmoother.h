#pragma once

#include <array>
#include <unordered_map>
#include <algorithm>
#include <cstddef>
#include <cfloat>
#include <cmath>
#include <Utils/ue4math/ue4math.h>
#include <Common/Data.h>
#include <Common/Bone.h>

class BoneSmoother
{
private:
    static constexpr size_t kBoneCacheCount = static_cast<size_t>(EBoneIndex::VB_Lowerarm_L_Hand_L) + 1;

    struct BoneState
    {
        bool worldInitialized = false;
        bool screenInitialized = false;
        FVector worldPosition{};
        FVector2D screenPosition{};
        float worldTimestamp = 0.0f;
        float screenTimestamp = 0.0f;
    };

    struct EntityCache
    {
        std::array<BoneState, kBoneCacheCount> bones{};
    };

    std::unordered_map<uint64_t, EntityCache> boneHistoryCache;
    float smoothingFactor = 0.22f;
    size_t maxHistorySize = 4;
    float maxSpeedThreshold = 2500.0f;

public:
    BoneSmoother() = default;

    void SetSmoothingFactor(float factor)
    {
        smoothingFactor = std::clamp(factor, 0.08f, 0.85f);
    }

    void SetMaxHistorySize(size_t size)
    {
        maxHistorySize = (size < 2) ? 2 : size;
    }

    void SetMaxSpeedThreshold(float threshold)
    {
        maxSpeedThreshold = (threshold > 10.0f) ? threshold : 10.0f;
    }

    FVector SmoothWorldPosition(uint64_t entityId, int boneIndex, const FVector& currentWorldPos, float currentTime)
    {
        if (!IsTrackedBoneIndex(boneIndex) || !IsValidPosition(currentWorldPos))
        {
            return currentWorldPos;
        }

        BoneState& boneState = boneHistoryCache[entityId].bones[static_cast<size_t>(boneIndex)];
        if (!boneState.worldInitialized)
        {
            boneState.worldInitialized = true;
            boneState.worldPosition = currentWorldPos;
            boneState.worldTimestamp = currentTime;
            return currentWorldPos;
        }

        float deltaTime = currentTime - boneState.worldTimestamp;
        if (deltaTime <= 0.0f)
        {
            deltaTime = 0.016f;
        }

        const float adaptiveFactor = GetAdaptiveWorldFactor(boneState.worldPosition, currentWorldPos, deltaTime);
        boneState.worldPosition = LerpVector(boneState.worldPosition, currentWorldPos, adaptiveFactor);
        boneState.worldTimestamp = currentTime;
        return boneState.worldPosition;
    }

    FVector2D SmoothScreenPosition(uint64_t entityId, int boneIndex, const FVector2D& currentScreenPos, float currentTime)
    {
        if (!IsTrackedBoneIndex(boneIndex))
        {
            return currentScreenPos;
        }

        BoneState& boneState = boneHistoryCache[entityId].bones[static_cast<size_t>(boneIndex)];
        if (!IsValidScreenPosition(currentScreenPos))
        {
            return boneState.screenInitialized ? boneState.screenPosition : currentScreenPos;
        }

        if (!boneState.screenInitialized)
        {
            boneState.screenInitialized = true;
            boneState.screenPosition = currentScreenPos;
            boneState.screenTimestamp = currentTime;
            return currentScreenPos;
        }

        const float adaptiveFactor = GetAdaptiveScreenFactor(boneState.screenPosition, currentScreenPos);
        boneState.screenPosition = LerpVector2D(boneState.screenPosition, currentScreenPos, adaptiveFactor);
        boneState.screenTimestamp = currentTime;
        return boneState.screenPosition;
    }

    void SmoothPlayerBones(Player& player, float currentTime)
    {
        for (EBoneIndex bone : SkeletonLists::Bones)
        {
            const int boneIdx = static_cast<int>(bone);
            auto screenIt = player.Skeleton.ScreenBones.find(boneIdx);
            if (screenIt != player.Skeleton.ScreenBones.end())
            {
                screenIt->second = SmoothScreenPosition(player.Entity, boneIdx, screenIt->second, currentTime);
            }
        }
    }

    void EnableVehicleMode(bool enable)
    {
        if (enable)
        {
            smoothingFactor = 0.16f;
            maxHistorySize = 3;
            maxSpeedThreshold = 5000.0f;
        }
        else
        {
            smoothingFactor = 0.22f;
            maxHistorySize = 4;
            maxSpeedThreshold = 2500.0f;
        }
    }

    static bool IsPlayerLikelyInVehicle(const Player& player)
    {
        const float speedKmh = player.Velocity.Length() / 100.0f * 3.6f;
        if (speedKmh > 40.0f)
        {
            return true;
        }

        if (speedKmh > 25.0f && player.Location.Z > 100.0f)
        {
            return true;
        }

        return false;
    }

    void ClearEntityCache(uint64_t entityId)
    {
        boneHistoryCache.erase(entityId);
    }

    void ClearAllCache()
    {
        boneHistoryCache.clear();
    }

private:
    static bool IsTrackedBoneIndex(int boneIndex)
    {
        return boneIndex >= 0 && boneIndex < static_cast<int>(kBoneCacheCount);
    }

    static float Distance2D(const FVector2D& a, const FVector2D& b)
    {
        const float dx = a.X - b.X;
        const float dy = a.Y - b.Y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool IsValidPosition(const FVector& pos) const
    {
        return !std::isnan(pos.X) && !std::isnan(pos.Y) && !std::isnan(pos.Z) &&
            pos.X != FLT_MAX && pos.Y != FLT_MAX && pos.Z != FLT_MAX &&
            pos.X < 100000.0f && pos.X > -100000.0f &&
            pos.Y < 100000.0f && pos.Y > -100000.0f &&
            pos.Z < 100000.0f && pos.Z > -100000.0f;
    }

    bool IsValidScreenPosition(const FVector2D& screenPos) const
    {
        return !std::isnan(screenPos.X) && !std::isnan(screenPos.Y) &&
            screenPos.X != FLT_MAX && screenPos.Y != FLT_MAX &&
            screenPos.X >= -1000.0f && screenPos.X <= 5000.0f &&
            screenPos.Y >= -1000.0f && screenPos.Y <= 5000.0f;
    }

    float GetAdaptiveWorldFactor(const FVector& previousPos, const FVector& currentPos, float deltaTime) const
    {
        const FVector delta = currentPos - previousPos;
        const float safeDeltaTime = (deltaTime > 0.016f) ? deltaTime : 0.016f;
        const float speed = delta.Length() / safeDeltaTime;
        if (speed >= maxSpeedThreshold)
        {
            return 0.85f;
        }

        const float normalized = std::clamp(speed / maxSpeedThreshold, 0.0f, 1.0f);
        return std::clamp(smoothingFactor + normalized * 0.35f, smoothingFactor, 0.85f);
    }

    float GetAdaptiveScreenFactor(const FVector2D& previousPos, const FVector2D& currentPos) const
    {
        const float pixelDelta = Distance2D(previousPos, currentPos);
        const float normalized = std::clamp(pixelDelta / 120.0f, 0.0f, 1.0f);
        return std::clamp(smoothingFactor + normalized * 0.40f, smoothingFactor, 0.85f);
    }

    static FVector LerpVector(const FVector& from, const FVector& to, float factor)
    {
        return FVector(
            from.X + (to.X - from.X) * factor,
            from.Y + (to.Y - from.Y) * factor,
            from.Z + (to.Z - from.Z) * factor
        );
    }

    static FVector2D LerpVector2D(const FVector2D& from, const FVector2D& to, float factor)
    {
        return FVector2D(
            from.X + (to.X - from.X) * factor,
            from.Y + (to.Y - from.Y) * factor
        );
    }
};
