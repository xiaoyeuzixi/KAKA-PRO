//#pragma once
//#include <DMALibrary/Memory/Memory.h>
//#include <Common/Data.h>
//#include <Common/Entitys.h>
//#include <Utils/Utils.h>
//#include <Hack/GNames.h>
//#include <Hack/Decrypt.h>
//#include <Common/VisibleScene.h>
//#include <mutex> // 确保包含互斥锁头文件
//
//namespace LineTrace
//{
//    // 静态互斥锁，保护GameData的访问
//    static std::mutex gameDataMutex;
//
//    // 单线追踪函数，检查从 TraceStart 到 TraceEnd 的路径是否被任何物体阻挡
//    static bool LineTraceSingle(FVector TraceStart, FVector TraceEnd)
//    {
//        // 获取互斥锁，确保GameData的访问是线程安全的
//        std::lock_guard<std::mutex> lock(gameDataMutex);
//
//        // 检查当前场景是否为游戏场景，且各个动态场景是否初始化
//        if (GameData.Scene != Scene::Gaming ||
//            GameData.DynamicLoadScene == nullptr ||
//            GameData.HeightFieldScene == nullptr ||
//            GameData.DynamicRigidScene == nullptr) {
//            return true; // 如果不满足条件则返回 true，表示没有阻挡
//        }
//
//        // 将轨迹起点和终点转换为世界坐标
//        FVector origin = TraceStart + GameData.Radar.WorldOriginLocation;
//        FVector target = TraceEnd + GameData.Radar.WorldOriginLocation;
//
//        // 在动态加载场景中进行射线检测
//        auto dynamicRayHit = GameData.DynamicLoadScene->Raycast(origin, target);
//        if (dynamicRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            return false; // 如果有碰撞，返回 false
//        }
//
//        // 在高度场场景中进行射线检测
//        auto heightFieldRayHit = GameData.HeightFieldScene->Raycast(origin, target);
//        if (heightFieldRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            return false; // 如果有碰撞，返回 false
//        }
//
//        // 在全局动态刚体场景中进行射线检测
//        auto globalSceneRayHit = GameData.DynamicRigidScene->Raycast(origin, target);
//        if (globalSceneRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            return false; // 如果有碰撞，返回 false
//        }
//
//        return true; // 如果没有任何碰撞，返回 true
//    }
//
//    // 获取下一个提示的函数
//    static TriangleMeshData* getNextHint() {
//        // 获取互斥锁，确保GameData的访问是线程安全的
//        std::lock_guard<std::mutex> lock(gameDataMutex);
//
//        // 检查当前场景是否为游戏场景，且各个动态场景是否初始化
//        if (GameData.Scene != Scene::Gaming ||
//            GameData.DynamicLoadScene == nullptr ||
//            GameData.HeightFieldScene == nullptr ||
//            GameData.DynamicRigidScene == nullptr) {
//            return nullptr; // 如果不满足条件则返回 nullptr
//        }
//
//        // 根据摄像机的旋转计算前方方向向量
//        FVector forwardVector;
//        forwardVector.X = cos(GameData.Camera.Rotation.Yaw * M_PI / 180.0f) * cos(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);
//        forwardVector.Y = sin(GameData.Camera.Rotation.Yaw * M_PI / 180.0f) * cos(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);
//        forwardVector.Z = sin(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);
//
//        // 计算射线起点和终点
//        FVector origin = GameData.Camera.Location + GameData.Radar.WorldOriginLocation;
//        FVector target = origin + forwardVector * 100000.0f; // 射线延伸至 100000 单位
//
//        // 在动态加载场景中进行射线检测
//        auto dynamicRayHit = GameData.DynamicLoadScene->Raycast(origin, target);
//        // 在高度场场景中进行射线检测
//        auto heightFieldRayHit = GameData.HeightFieldScene->Raycast(origin, target);
//        // 在全局动态刚体场景中进行射线检测
//        auto globalSceneRayHit = GameData.DynamicRigidScene->Raycast(origin, target);
//
//        // 如果没有任何碰撞，返回 nullptr
//        if (dynamicRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID &&
//            heightFieldRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID &&
//            globalSceneRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
//            return nullptr; // 没有碰撞物体
//        }
//
//        // 如果有多个碰撞,返回最近的碰撞数据
//        float minDist = FLT_MAX; // 初始化最小距离
//        TriangleMeshData* result = nullptr; // 声明结果指针
//
//        // 处理动态加载场景的碰撞
//        if (dynamicRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            if (dynamicRayHit.ray.tfar < minDist) { // 如果当前碰撞距离小于最小距离
//                minDist = dynamicRayHit.ray.tfar; // 更新最小距离
//                result = GameData.DynamicLoadScene->GetGeomeoryData(dynamicRayHit.hit.geomID); // 获取碰撞数据
//            }
//        }
//
//        // 处理高度场场景的碰撞
//        if (heightFieldRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            if (heightFieldRayHit.ray.tfar < minDist) { // 如果当前碰撞距离小于最小距离
//                minDist = heightFieldRayHit.ray.tfar; // 更新最小距离
//                result = GameData.HeightFieldScene->GetGeomeoryData(heightFieldRayHit.hit.geomID); // 获取碰撞数据
//            }
//        }
//
//        // 处理全局动态刚体场景的碰撞
//        if (globalSceneRayHit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
//            if (globalSceneRayHit.ray.tfar < minDist) { // 如果当前碰撞距离小于最小距离
//                minDist = globalSceneRayHit.ray.tfar; // 更新最小距离
//                result = GameData.DynamicRigidScene->GetGeomeoryData(globalSceneRayHit.hit.geomID); // 获取碰撞数据
//            }
//        }
//
//        return result; // 返回最近的碰撞数据
//    }
//}


#pragma once // 防止头文件被重复包含
#include <DMALibrary/Memory/Memory.h> // 引入内存管理库
#include <Common/Data.h> // 引入通用数据定义
#include <Common/Entitys.h> // 引入实体相关定义
#include <Utils/Utils.h> // 引入工具库
#include <Utils/Throttler.h> // 引入节流器
#include <Hack/GNames.h> // 引入游戏名相关的黑客库
#include <Hack/Decrypt.h> // 引入解密相关的黑客库
#include <Common/VisibleScene.h> // 引入可见场景相关定义

namespace LineTrace
{
    static std::mutex lineTraceMutex;

    static void AddTraceOriginCandidate(std::vector<FVector>& origins, const FVector& candidate)
    {
        if (candidate.IsNearlyZero()) {
            return;
        }

        for (const auto& origin : origins) {
            if (origin.Distance(candidate) <= 3.0f) {
                return;
            }
        }

        origins.push_back(candidate);
    }

    static std::vector<FVector> BuildLocalTraceOrigins(FVector fallbackStart)
    {
        std::vector<FVector> origins{};
        const auto& localPlayer = GameData.LocalPlayerInfo;

        if (!localPlayer.Location.IsNearlyZero()) {
            AddTraceOriginCandidate(origins, localPlayer.Location + FVector(0.0f, 0.0f, 55.0f));
            AddTraceOriginCandidate(origins, localPlayer.Location + FVector(0.0f, 0.0f, 75.0f));
            AddTraceOriginCandidate(origins, localPlayer.Location + FVector(0.0f, 0.0f, 92.0f));
        }

        AddTraceOriginCandidate(origins, fallbackStart);
        return origins;
    }

    static bool HasDirectLine(const std::shared_ptr<Physics::VisibleScene>& scene, const FVector& localOrigin, const FVector& localTarget)
    {
        FVector origin = localOrigin + GameData.Radar.WorldOriginLocation;
        FVector target = localTarget + GameData.Radar.WorldOriginLocation;
        FVector direction = target - origin;
        const float distance = direction.Length();
        if (distance <= 1.0f) {
            return true;
        }

        direction.Normalize();
        origin = origin + direction * 8.0f;
        target = target - direction * 2.0f;

        auto rayHit = scene->Raycast(origin, target);
        return rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID;
    }

    static bool LineTraceSingle(FVector TraceStart, FVector TraceEnd)
    {
        std::lock_guard<std::mutex> lock(lineTraceMutex);
        auto scene = std::atomic_load(&GameData.PhysxScene);
        if (GameData.Scene != Scene::Gaming || scene == nullptr) {
            return true;
        }

        const auto origins = BuildLocalTraceOrigins(TraceStart);
        for (const auto& origin : origins) {
            if (HasDirectLine(scene, origin, TraceEnd)) {
                return true;
            }
        }

        return false;
    }

    static std::shared_ptr<const Physics::SceneGeometryData> getNextHint()
    {
        auto scene = std::atomic_load(&GameData.PhysxScene);
        if (GameData.Scene != Scene::Gaming || scene == nullptr) {
            return nullptr;
        }

        FVector forwardVector;
        forwardVector.X = cos(GameData.Camera.Rotation.Yaw * M_PI / 180.0f) * cos(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);
        forwardVector.Y = sin(GameData.Camera.Rotation.Yaw * M_PI / 180.0f) * cos(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);
        forwardVector.Z = sin(GameData.Camera.Rotation.Pitch * M_PI / 180.0f);

        FVector origin = GameData.Camera.Location + GameData.Radar.WorldOriginLocation;
        FVector target = origin + forwardVector * 100000.0f;

        auto rayHit = scene->Raycast(origin, target);
        if (rayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
            return nullptr;
        }

        return scene->GetGeometryData(rayHit.hit.geomID);
    }
}

