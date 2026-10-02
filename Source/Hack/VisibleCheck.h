#pragma once

#include <algorithm>
#include <chrono>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

#include <Common/Data.h>
#include <Common/VisibleScene.h>
#include <Hack/Physx.h>

namespace VisibleCheck {

    struct LoaderState {
        uint32_t staticTimestamp = 0;
        std::unordered_map<PrunerPayload, PxTransformT, PrunerPayloadHash> staticPoseCache{};
        std::unordered_map<PrunerPayload, PxGeometryType, PrunerPayloadHash> staticTypeCache{};
        std::unordered_map<PrunerPayload, bool, PrunerPayloadHash> staticRenderableCache{};
        std::unordered_map<PrunerPayload, uint64_t, PrunerPayloadHash> staticGeometryPtrCache{};
        std::unordered_map<PrunerPayload, uint64_t, PrunerPayloadHash> staticHeightFieldSamplePtrCache{};
        std::set<PrunerPayload> staticSceneObjects{};

        std::unordered_map<PrunerPayload, PxTransformT, PrunerPayloadHash> dynamicPoseCache{};
        std::unordered_map<PrunerPayload, uint64_t, PrunerPayloadHash> dynamicGeometryPtrCache{};
        std::set<PrunerPayload> dynamicSceneObjects{};
    };

    static bool ShouldRun(uint64_t generation) {
        auto scene = std::atomic_load(&GameData.PhysxScene);
        return GameData.Scene == Scene::Gaming &&
            !GameData.PhysxLoaderStopRequested.load() &&
            GameData.PhysxLoaderGeneration.load() == generation &&
            scene != nullptr;
    }

    static std::chrono::steady_clock::time_point ComputeNextWake(
        std::chrono::steady_clock::time_point nextStatic,
        std::chrono::steady_clock::time_point nextDynamic
    ) {
        return nextStatic < nextDynamic ? nextStatic : nextDynamic;
    }

    static Vector3 ToScenePosition(const FVector& localPosition) {
        return {
            localPosition.X + GameData.Radar.WorldOriginLocation.X,
            localPosition.Y + GameData.Radar.WorldOriginLocation.Y,
            localPosition.Z + GameData.Radar.WorldOriginLocation.Z
        };
    }

    static bool IsSecondaryFocusCandidate(const Player& player) {
        return !player.IsMe &&
            !player.IsMyTeam &&
            player.InScreen &&
            (player.State == CharacterState::Alive || player.State == CharacterState::Groggy) &&
            !player.Location.IsNearlyZero();
    }

    static void TryAddSecondaryFocusPosition(
        std::vector<Vector3>& focusPositions,
        const Vector3& cameraPosition,
        const Vector3& focusPosition,
        double triggerDistanceSquare,
        double dedupeDistanceSquare,
        size_t maxFocusCount
    ) {
        if (focusPositions.size() >= maxFocusCount) {
            return;
        }

        if ((focusPosition - cameraPosition).Length2DSquare() <= triggerDistanceSquare) {
            return;
        }

        for (const auto& existingPosition : focusPositions) {
            if ((focusPosition - existingPosition).Length2DSquare() <= dedupeDistanceSquare) {
                return;
            }
        }

        focusPositions.push_back(focusPosition);
    }

    static std::vector<Vector3> CollectSecondaryFocusPositions(
        const Vector3& cameraPosition,
        double triggerDistance,
        double secondaryRadius,
        size_t maxFocusCount = 3
    ) {
        std::vector<Vector3> focusPositions{};
        if (maxFocusCount == 0 || secondaryRadius <= 0.0) {
            return focusPositions;
        }

        const double triggerDistanceSquare = triggerDistance * triggerDistance;
        const double dedupeDistance = secondaryRadius * 0.75;
        const double dedupeDistanceSquare = dedupeDistance * dedupeDistance;
        const auto players = Data::GetPlayers();

        auto targetIt = players.find(GameData.AimBot.Target);
        if (targetIt != players.end() && IsSecondaryFocusCandidate(targetIt->second)) {
            TryAddSecondaryFocusPosition(
                focusPositions,
                cameraPosition,
                ToScenePosition(targetIt->second.Location),
                triggerDistanceSquare,
                dedupeDistanceSquare,
                maxFocusCount
            );
        }

        std::vector<std::pair<double, Vector3>> candidates{};
        candidates.reserve(players.size());

        for (const auto& [entity, player] : players) {
            if (entity == GameData.AimBot.Target || !IsSecondaryFocusCandidate(player)) {
                continue;
            }

            const auto worldPosition = ToScenePosition(player.Location);
            const double distanceSquare = (worldPosition - cameraPosition).Length2DSquare();
            if (distanceSquare <= triggerDistanceSquare) {
                continue;
            }

            candidates.emplace_back(distanceSquare, worldPosition);
        }

        std::sort(candidates.begin(), candidates.end(),
            [](const auto& lhs, const auto& rhs) {
                return lhs.first < rhs.first;
            }
        );

        for (const auto& candidate : candidates) {
            TryAddSecondaryFocusPosition(
                focusPositions,
                cameraPosition,
                candidate.second,
                triggerDistanceSquare,
                dedupeDistanceSquare,
                maxFocusCount
            );

            if (focusPositions.size() >= maxFocusCount) {
                break;
            }
        }

        return focusPositions;
    }

    static void RunPhysxLoader(uint64_t generation) {
        using clock = std::chrono::steady_clock;
        LoaderState state{};

        auto now = clock::now();
        auto nextStaticUpdate = now;
        auto nextDynamicUpdate = now;
        const auto warmupEnd = now + std::chrono::seconds(4);

        while (ShouldRun(generation)) {
            auto scene = std::atomic_load(&GameData.PhysxScene);
            if (!scene) {
                break;
            }

            if (GameData.Radar.WorldOriginLocation.IsNearlyZero(1.0f) || GameData.Camera.Location.IsNearlyZero(1.0f)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            const auto cameraPosition = GameData.Camera.Location + GameData.Radar.WorldOriginLocation;
            if (cameraPosition.IsNearlyZero()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::vector<TriangleMeshData> meshes{};
            std::set<SceneGeometryKey> removeKeys{};
            const Vector3 currentPosition = { cameraPosition.X, cameraPosition.Y, cameraPosition.Z };
            const auto staticIntervalMs = (std::max)(250, (std::min)(
                GameData.Config.ESP.PhysxStaticRefreshInterval,
                GameData.Config.ESP.HeightFieldMutex
            ));
            const auto dynamicIntervalMs = (std::max)(100, GameData.Config.ESP.PhysxDynamicRefreshInterval);
            const uint32_t baseBuildBudget = static_cast<uint32_t>((std::max)(32, GameData.Config.ESP.PhysxRefreshLimit));
            const double primaryRadius = GameData.Config.ESP.PhysxLoadRadius * 100.0;
            const double secondaryRadius = (std::max)(15000.0, (std::min)(primaryRadius * 0.35, 30000.0));
            const double focusTriggerDistance = (std::max)(secondaryRadius, primaryRadius * 0.7);
            const auto extraFocusPositions = CollectSecondaryFocusPositions(
                currentPosition,
                focusTriggerDistance,
                secondaryRadius
            );
            now = clock::now();
            const bool isWarmup = now < warmupEnd;
            const int activeStaticIntervalMs = isWarmup ? (std::max)(120, staticIntervalMs / 3) : staticIntervalMs;
            const int activeDynamicIntervalMs = isWarmup ? (std::max)(50, dynamicIntervalMs / 3) : dynamicIntervalMs;
            const uint32_t buildBudget = isWarmup ? (std::min)(baseBuildBudget * 3, 1024u) : baseBuildBudget;
            meshes.reserve(static_cast<size_t>(buildBudget) * 2);

            if (now >= nextStaticUpdate) {
                auto staticMeshes = physx::LoadStaticSceneByRange(
                    state.staticTimestamp,
                    state.staticPoseCache,
                    state.staticTypeCache,
                    state.staticRenderableCache,
                    state.staticGeometryPtrCache,
                    state.staticHeightFieldSamplePtrCache,
                    state.staticSceneObjects,
                    removeKeys,
                    currentPosition,
                    primaryRadius,
                    extraFocusPositions,
                    secondaryRadius,
                    buildBudget
                );
                meshes.insert(
                    meshes.end(),
                    std::make_move_iterator(staticMeshes.begin()),
                    std::make_move_iterator(staticMeshes.end())
                );
                nextStaticUpdate = now + std::chrono::milliseconds(activeStaticIntervalMs);
            }

            if (now >= nextDynamicUpdate) {
                auto dynamicMeshes = physx::LoadDynamicRigidShape(
                    state.dynamicSceneObjects,
                    state.dynamicPoseCache,
                    state.dynamicGeometryPtrCache,
                    removeKeys,
                    currentPosition,
                    primaryRadius,
                    extraFocusPositions,
                    secondaryRadius,
                    buildBudget
                );
                meshes.insert(
                    meshes.end(),
                    std::make_move_iterator(dynamicMeshes.begin()),
                    std::make_move_iterator(dynamicMeshes.end())
                );
                nextDynamicUpdate = now + std::chrono::milliseconds(activeDynamicIntervalMs);
            }

            if (!meshes.empty() || !removeKeys.empty()) {
                scene->UpdateMesh(meshes, removeKeys);
            }

            const auto nextWake = ComputeNextWake(nextStaticUpdate, nextDynamicUpdate);
            const auto sleepUntil = nextWake < clock::now() ? clock::now() + std::chrono::milliseconds(25) : nextWake;
            std::this_thread::sleep_until(sleepUntil);
        }
    }

} // namespace VisibleCheck
