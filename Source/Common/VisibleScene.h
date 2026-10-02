#pragma once

#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include <Common/Data.h>

namespace Physics {

	static void embreeErrorFunction(void* userPtr, RTCError code, const char* str) {
		(void)userPtr;
		(void)code;
		(void)str;
	}

	class VisibleScene {
	public:
		VisibleScene() : device(nullptr), scene(nullptr), isShuttingDown(false) {
			try {
				device = rtcNewDevice(nullptr);
				if (!device) {
					return;
				}

				rtcSetDeviceErrorFunction(device, embreeErrorFunction, nullptr);
				scene = rtcNewScene(device);
				if (!scene) {
					rtcReleaseDevice(device);
					device = nullptr;
					return;
				}

				rtcSetDeviceProperty(device, RTC_DEVICE_PROPERTY_TASKING_SYSTEM, 1);
				rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_LOW);
				rtcSetSceneFlags(scene, RTC_SCENE_FLAG_DYNAMIC);
				rtcCommitScene(scene);
			}
			catch (...) {
				if (scene) {
					rtcReleaseScene(scene);
					scene = nullptr;
				}
				if (device) {
					rtcReleaseDevice(device);
					device = nullptr;
				}
			}
		}

		~VisibleScene() {
			isShuttingDown = true;

			try {
				std::unique_lock<std::shared_mutex> lock(sceneRWMutex);
				geometryDatas.clear();
				geometry_id_map.clear();
				geometries.clear();

				if (scene) {
					rtcReleaseScene(scene);
					scene = nullptr;
				}

				if (device) {
					rtcReleaseDevice(device);
					device = nullptr;
				}
			}
			catch (...) {
			}
		}

		void UpdateMesh(
			const std::vector<TriangleMeshData>& willAddMeshes,
			const std::set<SceneGeometryKey>& removeKeys
		) {
			if (isShuttingDown) {
				return;
			}

			if (willAddMeshes.empty() && removeKeys.empty()) {
				return;
			}

			std::unique_lock<std::shared_mutex> lock(sceneRWMutex);
			if (!scene || !device) {
				return;
			}

			bool sceneChanged = false;

			try {
				for (const auto& key : removeKeys) {
					auto idIt = geometry_id_map.find(key);
					if (idIt == geometry_id_map.end()) {
						continue;
					}

					const uint32_t geometry_id = idIt->second;
					rtcDetachGeometry(scene, geometry_id);
					geometries.erase(geometry_id);
					geometry_id_map.erase(idIt);
					geometryDatas.erase(key);
					sceneChanged = true;
				}

				for (const auto& mesh : willAddMeshes) {
					if (mesh.Vertices.empty() || mesh.Indices.empty()) {
						continue;
					}

					RTCGeometry geom = nullptr;
					bool shouldRelease = false;
					uint32_t geometry_id = 0;
					auto existingGeometryIt = geometry_id_map.find(mesh.SceneKey);

					if (existingGeometryIt != geometry_id_map.end()) {
						geometry_id = existingGeometryIt->second;
						geom = rtcGetGeometry(scene, geometry_id);
						if (!geom) {
							geometryDatas.erase(mesh.SceneKey);
							geometry_id_map.erase(existingGeometryIt);
							geometry_id = 0;
						}
					}

					if (!geom) {
						geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
						shouldRelease = true;
					}

					if (!geom) {
						continue;
					}

					auto geometryData = BuildGeometryData(mesh);

					float* vertices = static_cast<float*>(rtcSetNewGeometryBuffer(
						geom,
						RTC_BUFFER_TYPE_VERTEX,
						0,
						RTC_FORMAT_FLOAT3,
						3 * sizeof(float),
						mesh.Vertices.size()
					));
					if (!vertices) {
						geometry_id_map.erase(mesh.SceneKey);
						geometryDatas.erase(mesh.SceneKey);
						handleGeometryError(geom, shouldRelease, geometry_id);
						continue;
					}

					for (size_t i = 0; i < mesh.Vertices.size(); ++i) {
						vertices[i * 3] = mesh.Vertices[i].x;
						vertices[i * 3 + 1] = mesh.Vertices[i].y;
						vertices[i * 3 + 2] = mesh.Vertices[i].z;
					}

					const size_t triangleCount = mesh.Indices.size() / 3;
					unsigned int* indices = static_cast<unsigned int*>(rtcSetNewGeometryBuffer(
						geom,
						RTC_BUFFER_TYPE_INDEX,
						0,
						RTC_FORMAT_UINT3,
						3 * sizeof(unsigned int),
						triangleCount
					));
					if (!indices) {
						geometry_id_map.erase(mesh.SceneKey);
						geometryDatas.erase(mesh.SceneKey);
						handleGeometryError(geom, shouldRelease, geometry_id);
						continue;
					}

					std::memcpy(indices, mesh.Indices.data(), mesh.Indices.size() * sizeof(uint32_t));

					rtcSetGeometryUserData(geom, geometryData.get());
					rtcCommitGeometry(geom);

					if (shouldRelease) {
						try {
							geometry_id = rtcAttachGeometry(scene, geom);
							geometries[geometry_id] = std::shared_ptr<void>(geom, [](RTCGeometry geometry) {
								if (geometry) {
									rtcReleaseGeometry(geometry);
								}
							});
						}
						catch (...) {
							rtcReleaseGeometry(geom);
							continue;
						}
					}

					geometry_id_map[mesh.SceneKey] = geometry_id;
					geometryDatas[mesh.SceneKey] = std::move(geometryData);
					sceneChanged = true;
				}

				if (sceneChanged) {
					rtcCommitScene(scene);
				}
			}
			catch (...) {
			}
		}

		RTCRayHit Raycast(FVector& origin, FVector& target) {
			RTCRayHit rayhit{};
			rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

			if (isShuttingDown) {
				return rayhit;
			}

			try {
				std::shared_lock<std::shared_mutex> lock(sceneRWMutex);
				if (!scene || !device) {
					return rayhit;
				}

				if (!std::isfinite(origin.X) || !std::isfinite(origin.Y) || !std::isfinite(origin.Z) ||
					!std::isfinite(target.X) || !std::isfinite(target.Y) || !std::isfinite(target.Z)) {
					return rayhit;
				}

				RTCRay ray{};
				ray.org_x = origin.X;
				ray.org_y = origin.Y;
				ray.org_z = origin.Z;
				ray.dir_x = target.X - origin.X;
				ray.dir_y = target.Y - origin.Y;
				ray.dir_z = target.Z - origin.Z;

				const float dir_length = std::sqrt(
					ray.dir_x * ray.dir_x +
					ray.dir_y * ray.dir_y +
					ray.dir_z * ray.dir_z
				);
				if (dir_length < 1e-6f) {
					return rayhit;
				}

				ray.dir_x /= dir_length;
				ray.dir_y /= dir_length;
				ray.dir_z /= dir_length;
				ray.tnear = 0.0f;
				ray.tfar = dir_length;
				ray.mask = -1;
				ray.flags = 0;

				rayhit.ray = ray;

				RTCRayQueryContext context{};
				rtcInitRayQueryContext(&context);

				RTCIntersectArguments intersectArgs{};
				rtcInitIntersectArguments(&intersectArgs);
				intersectArgs.context = &context;

				rtcIntersect1(scene, &rayhit, &intersectArgs);
				return rayhit;
			}
			catch (...) {
				rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
				return rayhit;
			}
		}

		std::shared_ptr<const SceneGeometryData> GetGeometryData(uint32_t geomId) const {
			if (isShuttingDown) {
				return nullptr;
			}

			try {
				std::shared_lock<std::shared_mutex> lock(sceneRWMutex);
				if (!scene) {
					return nullptr;
				}

				RTCGeometry geometry = rtcGetGeometry(scene, geomId);
				if (!geometry) {
					return nullptr;
				}

				const auto* geometryData = static_cast<const SceneGeometryData*>(rtcGetGeometryUserData(geometry));
				if (!geometryData) {
					return nullptr;
				}

				auto it = geometryDatas.find(geometryData->SceneKey);
				if (it == geometryDatas.end()) {
					return nullptr;
				}

				return it->second;
			}
			catch (...) {
				return nullptr;
			}
		}

		std::vector<std::shared_ptr<const SceneGeometryData>> GetNearMesh(FVector position, double radius) const {
			if (isShuttingDown) {
				return {};
			}

			std::vector<std::shared_ptr<const SceneGeometryData>> result;
			const Vector3 queryPosition = { position.X, position.Y, position.Z };
			const double radiusSquare = radius * radius;

			try {
				std::shared_lock<std::shared_mutex> lock(sceneRWMutex);
				for (const auto& [key, geometryData] : geometryDatas) {
					(void)key;
					const auto distance = (geometryData->Transform.mPosition - queryPosition).Length2DSquare();
					if (distance <= radiusSquare) {
						result.push_back(geometryData);
					}
				}
			}
			catch (...) {
			}

			return result;
		}

	private:
		std::shared_ptr<SceneGeometryData> BuildGeometryData(const TriangleMeshData& mesh) const {
			auto geometryData = std::make_shared<SceneGeometryData>();
			geometryData->SceneKey = mesh.SceneKey;
			geometryData->Flags = mesh.Flags;
			geometryData->QueryFilterData = mesh.QueryFilterData;
			geometryData->SimulationFilterData = mesh.SimulationFilterData;
			geometryData->UniqueKey1 = mesh.UniqueKey1;
			geometryData->UniqueKey2 = mesh.UniqueKey2;
			geometryData->Type = mesh.Type;
			geometryData->Transform = mesh.Transform;
			geometryData->VertexCount = mesh.Vertices.size();
			geometryData->IndexCount = mesh.Indices.size();

			if (GameData.Config.ESP.PhysXDebug) {
				geometryData->DebugMesh = std::make_shared<DebugMeshData>();
				geometryData->DebugMesh->Vertices = mesh.Vertices;
				geometryData->DebugMesh->Indices = mesh.Indices;
			}

			return geometryData;
		}

		void handleGeometryError(RTCGeometry geom, bool shouldRelease, uint32_t geometry_id) {
			if (shouldRelease) {
				rtcReleaseGeometry(geom);
			}
			else if (geom) {
				rtcDetachGeometry(scene, geometry_id);
				geometries.erase(geometry_id);
			}
		}

		RTCDevice device;
		RTCScene scene;
		std::unordered_map<SceneGeometryKey, uint32_t, SceneGeometryKeyHash> geometry_id_map{};
		std::unordered_map<SceneGeometryKey, std::shared_ptr<SceneGeometryData>, SceneGeometryKeyHash> geometryDatas{};
		std::unordered_map<uint32_t, std::shared_ptr<void>> geometries{};
		mutable std::shared_mutex sceneRWMutex;
		std::atomic<bool> isShuttingDown;
	};

} // namespace Physics
