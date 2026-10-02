#pragma once

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <Common/Data.h>
#include <DMALibrary/Memory/Memory.h>
#include <thread>
#include <Common/Constant.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Common/Entitys.h>
#include <Common/Offset.h>
#include <Common/VectorHelper.h>
#include <cstdint>
#include <psapi.h>
#include <Utils/Engine.h>
#include <Utils/ue4math/rotator.h>
#include <Utils/ue4math/transform.h>
#include <Utils/Timer.h>
#include <Hack/KeyState.h>
#include <Hack/Decrypt.h>
#include <Hack/GNames.h>
#include <Hack/AimBot.h>
#include <Hack/Players.h>
#include <Hack/Actors.h>
#include <Hack/Vehicles.h>
#include <Hack/Items.h>
#include <Hack/Projects.h>
#include <Hack/Radar.h>
#include <Hack/Segment.h>
//#include <Hack/Process.h>
#include <Common/Config.h>

#include <Utils/KmBox.h>
#include <Utils/KmBoxNet.h>
#include <Utils/Lurker.h>
#include <Utils/MoBox.h>
#include "Webpageradar.h"
#include "autoRecoil.h"

#include <Hack/VisibleCheck.h>

class Hack
{
public:

	static void Update()
	{
		auto hScatter = mem.CreateScatterHandle();
		while (true)
		{
			mem.RefreshTLB();

			GameData.UWorld = Decrypt::Xe(mem.Read<uint64_t>(GameData.GameBase + GameData.Offset["UWorld"]));
			GameData.GameInstance = Decrypt::Xe(mem.Read<uint64_t>(GameData.UWorld + GameData.Offset["GameInstance"]));
			GameData.GNames = GNames::GetGNamesPtr();

			if (Utils::ValidPtr(GameData.UWorld) || GameData.Scene != Scene::Gaming)
			{
				Sleep(GameData.ThreadSleep);
				continue;
			}

			GameData.GameState = Decrypt::Xe(mem.Read<uint64_t>(GameData.UWorld + GameData.Offset["GameState"]));
			GameData.LocalPlayer = Decrypt::Xe(mem.Read<uint64_t>(mem.Read<uint64_t>(GameData.GameInstance + GameData.Offset["LocalPlayer"])));
			GameData.PlayerController = Decrypt::Xe(mem.Read<uint64_t>(GameData.LocalPlayer + GameData.Offset["PlayerController"]));
			GameData.AcknowledgedPawn = Decrypt::Xe(mem.Read<uint64_t>(GameData.PlayerController + GameData.Offset["AcknowledgedPawn"]));
			GameData.CurrentLevel = Decrypt::Xe(mem.Read<uint64_t>(GameData.UWorld + GameData.Offset["CurrentLevel"]));
			GameData.ActorArray = Decrypt::Xe(mem.Read<uint64_t>(GameData.CurrentLevel + GameData.Offset["Actors"]));

			const uint64_t viewTargetOffset = GameData.Offset["ViewTarget"];

			uint64_t PlayerCameraManager;
			uint64_t MyHUD;
			BYTE bShowMouseCursor;
			uint64_t PlayerInput;
			uint64_t AntiCheatCharacterSyncManager;
			uint64_t CacheCameraViewTarget = 0;

			mem.AddScatterRead(hScatter, GameData.PlayerController + GameData.Offset["PlayerCameraManager"], &PlayerCameraManager);
			mem.AddScatterRead(hScatter, GameData.PlayerController + GameData.Offset["MyHUD"], &MyHUD);
			mem.AddScatterRead(hScatter, GameData.PlayerController + GameData.Offset["bShowMouseCursor"], &bShowMouseCursor);
			mem.AddScatterRead(hScatter, GameData.PlayerController + GameData.Offset["PlayerInput"], &PlayerInput);
			mem.AddScatterRead(hScatter, GameData.PlayerController + GameData.Offset["AntiCheatCharacterSyncManager"], &AntiCheatCharacterSyncManager);
			if (viewTargetOffset != 0) {
				mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + viewTargetOffset, &CacheCameraViewTarget);
			}
			mem.ExecuteReadScatter(hScatter);

			GameData.PlayerCameraManager = PlayerCameraManager;
			GameData.MyHUD = MyHUD;
			GameData.bShowMouseCursor = bShowMouseCursor == 0x25 ? true : false;
			GameData.PlayerInput = PlayerInput;
			GameData.AntiCheatCharacterSyncManager = AntiCheatCharacterSyncManager;
			GameData.CameraViewTarget = 0;


			if (viewTargetOffset != 0 && !Utils::ValidPtr(CacheCameraViewTarget)) {
				int ViewTargetID = Decrypt::CIndex(mem.Read<int>(CacheCameraViewTarget + GameData.Offset["ObjID"]));
				auto ViewTargetEntityInfo = Data::GetGNameListsByIDItem(ViewTargetID);
				if (ViewTargetEntityInfo.Type == EntityType::Player || ViewTargetEntityInfo.Type == EntityType::AI)
				{
					int TeamID = mem.Read<int>(CacheCameraViewTarget + GameData.Offset["LastTeamNum"]);
					GameData.CameraViewTarget = CacheCameraViewTarget;
					GameData.LocalPlayerTeamID = (TeamID >= 100000) ? (TeamID - 100000) : TeamID;
				}
			}

			Players::UpdatePlayerLists();
			Radar::UpdateWorldMapInfo();

			Sleep(300);
		}
		mem.CloseScatterHandle(hScatter);
	}





	static void ReleaseLoadedModel() {
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T initialMemory = pmc.WorkingSetSize / (1024 * 1024);

		GameData.PhysxLoaderStopRequested = true;
		{
			std::lock_guard<std::mutex> lock(GameData.physxLoaderMutex);
			if (GameData.PhysxLoaderThread && GameData.PhysxLoaderThread->joinable()) {
				GameData.PhysxLoaderThread->join();
			}
			GameData.PhysxLoaderThread.reset();
		}

		{
			std::lock_guard<std::mutex> lock(GameData.physxSceneMutex);
			std::atomic_store(&GameData.PhysxScene, std::shared_ptr<Physics::VisibleScene>{});
		}
		std::atomic_store(&GameData.NextHintMeshData, std::shared_ptr<const Physics::SceneGeometryData>{});

		EmptyWorkingSet(GetCurrentProcess());

		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T finalMemory = pmc.WorkingSetSize / (1024 * 1024);

		Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u8D44\u6E90\u91CA\u653E\u5B8C\u6210\u3002\u5185\u5B58\u53D8\u5316: %zu MB -> %zu MB (ï¿½ï¿½ï¿½ï¿½ %zd MB)"),
			initialMemory, finalMemory, initialMemory - finalMemory);
	}

	static void StartLoadMapModel() {
		ReleaseLoadedModel();
		Utils::Log(1, U8("[-] [OUTPUT] \u5F00\u59CB\u52A0\u8F7D\u5730\u56FE\u6A21\u578B..."));

		if (GameData.Config.ESP.LowModel)
		{
			GameData.Config.ESP.PhysxLoadRadius = 350;
			GameData.Config.ESP.PhysxDynamicRefreshInterval = 900;
			GameData.Config.ESP.HeightFieldMutex = 1800;
			GameData.Config.ESP.PhysxStaticRefreshInterval = 1400;
			GameData.Config.ESP.PhysxRefreshLimit = 96;
		}

		if (GameData.Config.ESP.MediumModel)
		{
			GameData.Config.ESP.PhysxLoadRadius = 500;
			GameData.Config.ESP.PhysxDynamicRefreshInterval = 700;
			GameData.Config.ESP.HeightFieldMutex = 1500;
			GameData.Config.ESP.PhysxStaticRefreshInterval = 1200;
			GameData.Config.ESP.PhysxRefreshLimit = 192;
		}

		if (GameData.Config.ESP.HighModel)
		{
			GameData.Config.ESP.PhysxLoadRadius = 700;
			GameData.Config.ESP.PhysxDynamicRefreshInterval = 500;
			GameData.Config.ESP.HeightFieldMutex = 1200;
			GameData.Config.ESP.PhysxStaticRefreshInterval = 900;
			GameData.Config.ESP.PhysxRefreshLimit = 320;
		}

		{
			std::lock_guard<std::mutex> lock(GameData.physxSceneMutex);
			std::atomic_store(&GameData.PhysxScene, std::make_shared<Physics::VisibleScene>());
		}

		GameData.PhysxLoaderStopRequested = false;
		const auto generation = GameData.PhysxLoaderGeneration.fetch_add(1) + 1;
		{
			std::lock_guard<std::mutex> lock(GameData.physxLoaderMutex);
			GameData.PhysxLoaderThread = std::make_unique<std::thread>(VisibleCheck::RunPhysxLoader, generation);
		}
	}

	static void UpdateCamera()
	{
		Throttler Throttlered;
		auto hScatter = mem.CreateScatterHandle();
		CameraData Camera;
		float DeltaSeconds = 0.f;
		float TimeSeconds = 0.f;

		while (true)
		{
			if (GameData.Scene != Scene::Gaming || !GameData.Config.Overlay.UseThread)
			{
				Sleep(GameData.ThreadSleep);
				continue;
			}

			Throttlered.executeTaskWithSleep("UpdateCameraSleep", std::chrono::milliseconds(4), [] {});

			if (!GameData.Config.Overlay.UseLastFrameCameraCache)
			{
				mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + GameData.Offset["CameraCacheLocation"], &Camera.Location);
				mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + GameData.Offset["CameraCacheRotation"], &Camera.Rotation);
				mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + GameData.Offset["CameraCacheFOV"], &Camera.FOV);
			}
			mem.AddScatterRead(hScatter, GameData.UWorld + GameData.Offset["TimeSeconds"], &TimeSeconds);
			mem.ExecuteReadScatter(hScatter);

			GameData.Camera = Camera;
			GameData.WorldTimeSeconds = TimeSeconds;
		}
		mem.CloseScatterHandle(hScatter);
	}

	static void UpdatePID()
	{
		Throttler throttler;
		bool gameProcessFound = false;
		bool prevGameProcessFound = false;
		int pidFailureCount = 0;
		const int PID_FAILURE_TOLERANCE = 10;
		auto lastHealthyProcessSignal = std::chrono::steady_clock::now();

		GameData.Scene = Scene::FindProcess;

		while (true)
		{
			throttler.executeTask("UpdatePID", 500ms, [&] {
				auto hasCachedProcessSignal = [&]() -> bool {
					if (GameData.PID == 0 || GameData.GameBase == 0 || GameData.Offset["UWorld"] == 0) {
						return false;
					}

					const uint64_t cachedUWorldToken = mem.Read<uint64_t>(GameData.GameBase + GameData.Offset["UWorld"], GameData.PID);
					return cachedUWorldToken > 0;
				};

				DWORD PID = mem.GetTslGamePID();
				if (PID == 0 && hasCachedProcessSignal())
				{
					PID = GameData.PID;
				}

				if (PID == 0)
				{
					pidFailureCount++;
					const bool processSignalExpired = (std::chrono::steady_clock::now() - lastHealthyProcessSignal) >= 8s;
					if (pidFailureCount >= PID_FAILURE_TOLERANCE && processSignalExpired)
					{
						gameProcessFound = false;
						if (prevGameProcessFound) {
							Utils::Log(2, U8("[-] [OUTPUT] \u6E38\u620F\u8FDB\u7A0B\u5DF2\u4E22\u5931\uFF0C\u5C1D\u8BD5\u6267\u884C\u9996\u6B21\u521D\u59CB\u5316..."));
							mem.Init("", false, false);
							GameData.GameBase = 0;
							GameData.PID = 0;
							Decrypt::DestroyXe();
						}
						mem.RefreshAll();
					}
				}
				else
				{
					lastHealthyProcessSignal = std::chrono::steady_clock::now();
					if (pidFailureCount > 0) {
						pidFailureCount = 0;
					}

					gameProcessFound = true;
					if (!prevGameProcessFound) {
						Utils::Log(1, U8("[-] [OUTPUT] \u68C0\u6D4B\u5230\u65B0\u6E38\u620F\u8FDB\u7A0B\uFF0C\u6B63\u5728\u9644\u52A0..."));
						EntityLists.clear();
						EntityInit();
						Data::SetGNameLists(EntityLists);
						Data::SetGNameListsByID({});
						GameData.HookBase = mem.GetHookModuleBase();
						KeyState::Init();
						Utils::Log(1, U8("[-] [OUTPUT] \u6E38\u620F\u52A0\u8F7D\u6210\u529F\uFF0C\u5DF2\u9644\u52A0\u5230\u6E38\u620F"));
					}
				}
				prevGameProcessFound = gameProcessFound;
			});


			// =========================================================================
			// ï¿½ï¿½ï¿½ï¿½ï¿\u573A\u666F\u72B6\u6001ï¿½ï¿½ï¿½ß¼ï¿½ï¿½ï¿½ï¿½Ö²ï¿½ï¿½ä£¬ï¿½ï¿½ï¿½Ñ¾ï¿½ï¿½Ç³ï¿½ï¿½ï¿½×³ï¿½ï¿½
			// =========================================================================
			Scene currentRealScene = GameData.Scene;
			if (gameProcessFound && !Utils::ValidPtr(GameData.UWorld))
			{
				int mapID = Decrypt::CIndex(mem.Read<uint64_t>(GameData.UWorld + GameData.Offset["ObjID"]));
				GameData.MapName = GNames::GetNameByID(mapID);
				currentRealScene = Utils::IsLobby(GameData.MapName) ? Scene::Lobby : Scene::Gaming;
			}
			else if (!gameProcessFound)
			{
				currentRealScene = Scene::FindProcess;
			}


			if (currentRealScene != GameData.Scene)
			{
				Scene oldScene = GameData.Scene;
				GameData.Scene = currentRealScene;

				Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u72B6\u6001ï¿½Ð»ï¿½: ï¿½ï¿½ %d -> ï¿½ï¿½ %d"), (int)oldScene, (int)GameData.Scene);

				if (oldScene == Scene::Gaming && GameData.Scene != Scene::Gaming)
				{
					Utils::Log(1, U8("[-] [OUTPUT] \u68C0\u6D4B\u5230\u73A9\u5BB6\u9000ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä£ï¿½ï¿?.."));
					ReleaseLoadedModel();

				}

				if (GameData.Scene == Scene::Gaming)
				{
					Utils::Log(0, U8("[-] [OUTPUT] \u5DF2\u8BFB\u53D6\u5230ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½×¼ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä£ï¿½ï¿½..."));
					Players::ReadPlayerLists();
					const auto loadDelay = 1s;
					std::this_thread::sleep_for(loadDelay);

					// --- ï¿½Þ¸Äµï\u4FEE\u6539\u70B9 ---
					// ï¿½Æ³ï¿½ï¿½Ë¹\u79FB\u9664\u4E86\u5173\u4E8E GameData.Config.ESP.UseEngineVisibilityCheck ï¿½ï¿½ï¿½Ð¶ï¿½
					// ï¿½ï¿½ï¿½Ú»ï\u7ACB\u5373\u52A0\u8F7D\u6A21\u578B
					GameData.ReloadModelRequested.store(false);
					StartLoadMapModel();
				}

				if (GameData.Scene == Scene::Lobby || GameData.Scene == Scene::FindProcess)
				{
					if (oldScene == Scene::Lobby) {
						ReleaseLoadedModel();
						//ï¿½Æ¸ï¿½ï¿½Â\u5730\u56FE\u66F4\u65B0\u540E
						std::thread([]() {
							}).detach();

					}
				}
			}

			if (GameData.ReloadModelRequested.exchange(false))
			{
				if (GameData.Scene == Scene::Gaming)
				{
					Utils::Log(1, U8("[-] [OUTPUT] ï¿½Õµï¿½ï¿½Ö¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â¼ï¿½ï¿½Øµï¿½Í¼Ä£ï¿½ï¿½..."));
					StartLoadMapModel();
				}
				else
				{
					Utils::Log(2, U8("[-] [OUTPUT] ï¿½ï¿½Ç°ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ï·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Þ·ï¿½ï¿½ï¿½ï¿½ï¿½Ä£ï¿½Í¡ï¿½"));
				}
			}
			//if (currentRealScene != GameData.Scene)
			//{
			//    Scene oldScene = GameData.Scene;
			//    GameData.Scene = currentRealScene;

			//    Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u72B6\u6001\u5207\u6362: \u4ECE %d -> \u5230 %d"), (int)oldScene, (int)GameData.Scene);

			//    if (oldScene == Scene::Gaming && GameData.Scene != Scene::Gaming)
			//    {
			//        Utils::Log(1, U8("[-] [OUTPUT] \u68C0\u6D4B\u5230\u73A9\u5BB6\u9000\u51FA\u6E38\u620F\uFF0C\u91CA\u653E\u573A\u666F\u6A21\u578Bï¿½ï¿½..."));
			//        ReleaseLoadedModel();
			//    }

			//    if (GameData.Scene == Scene::Gaming)
			//    {
			//        Utils::Log(0, U8("[-] [OUTPUT] \u5DF2\u8BFB\u53D6\u5230\u5730\u56FE\uFF0C\u51C6\u5907\u52A0\u8F7D\u6A21\u578B..."));
			//        Players::ReadPlayerLists();
			//        const auto loadDelay = 10s;
			//        std::this_thread::sleep_for(loadDelay);
			//        if (!GameData.Config.ESP.UseEngineVisibilityCheck)
			//        {
			//            StartLoadMapModel();
			//        }
			//        else
			//        {
			//            Utils::Log(1, U8("[-] [OUTPUT] ï¿½ï¿½ï¿½Ü¿ï¿½ï¿½ï¿½Ä£Ê½ï¿½Ñ¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä£ï¿½Í¡ï¿½"));
			//        }
			//    }





			//    if (GameData.Scene == Scene::Lobby || GameData.Scene == Scene::FindProcess)
			//    {
			//        if (oldScene == Scene::Lobby) {
			//            ReleaseLoadedModel();
			//        }
			//    }
			//}
			std::this_thread::sleep_for(50ms);
		}
	}

	//static void UpdatePID()
	//{
	//	Throttler Throttlered;

	//	bool GameProcessFound = false;
	//	bool PrevGameProcessFound = false;

	//	while (true)
	//	{
	//		Throttlered.executeTask("UpdatePID", std::chrono::milliseconds(1500), [&GameProcessFound, &PrevGameProcessFound] {
	//			DWORD PID = mem.GetTslGamePID();
	//			int PIDi = 0;
	//			while (PID == 0)
	//			{
	//				PID = mem.GetTslGamePID();
	//				Sleep(100);
	//				PIDi++;
	//				if (PIDi > 200)
	//				{
	//					break;
	//				}
	//			}
	//			if (PID == 0)
	//			{
	//				mem.RefreshAll();
	//				GameData.Scene = Scene::FindProcess;
	//				GameProcessFound = false;
	//				GameData.GameBase = 0;
	//				GameData.PID = 0;
	//				Decrypt::DestroyXe();

	//				if (GameProcessFound != PrevGameProcessFound) {
	//					Utils::Log(2, U8("[-] [OUTPUT] \u672A\u68C0\u6D4B\u5230\u6E38\u620F\uFF0C\u91CD\u7F6E\u6E38\u620F\u72B6\u6001"));
	//				}
	//			}
	//			else {
	//				GameProcessFound = true;
	//				if (GameProcessFound != PrevGameProcessFound) {
	//					GameData.HookBase = mem.GetHookModuleBase();
	//					Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u8D44\u6E90\u91CA\u653E\u5B8C\u6210[1/3]"));
	//					Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u8D44\u6E90\u91CA\u653E\u5B8C\u6210[2/3]"));
	//					Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u8D44\u6E90\u91CA\u653E\u5B8C\u6210[3/3]"));
	//					Utils::Log(1, U8("[-] [OUTPUT] \u573A\u666F\u8D44\u6E90\u91CA\u653E\u5B8C\u6210\u5931\u8D25\uFF0C\u53EF\u80FD\u5DF2\u9644\u52A0\u6E38\u620F"));
	//					Utils::Log(1, U8("[-] [OUTPUT] \u6E38\u620F\u52A0\u8F7D\u6210\u529F\uFF0C\u5DF2\u9644\u52A0\u5230\u6E38\u620F"));
	//					EntityInit();
	//					Data::SetGNameLists(EntityLists);
	//					Data::SetGNameListsByID({});
	//				}
	//			}
	//			PrevGameProcessFound = GameProcessFound;
	//			});

	//		if (GameData.PID != 0) {
	//			int MapID = Decrypt::CIndex(mem.Read<uint64_t>(GameData.UWorld + GameData.Offset["ObjID"]));
	//			GameData.MapName = GNames::GetNameByID(MapID);

	//			if (Utils::IsLobby(GameData.MapName)) {
	//				GameData.Scene = Scene::Lobby;
	//			}
	//			else {
	//				GameData.Scene = Scene::Gaming;
	//			}
	//		}

	//		if (GameData.Scene != GameData.PreviousScene) {
	//			GameData.PreviousScene = GameData.Scene;
	//			switch (GameData.Scene) {
	//			case Scene::FindProcess:
	//				break;
	//			case Scene::Lobby:
	//				Sleep(GameData.ThreadSleep);
	//				ReleaseLoadedModel();
	//				break;
	//			case Scene::Gaming:
	//				Utils::Log(0, U8("[-] [OUTPUT] ï¿½Ñ¶ï¿½È¡ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ï¿½ï¿½ë¿ªÊ¼ï¿½ï¿½ï¿½ï¿½"));
	//				//Utils::Log(0, "Entered the game scene, World name is [%s]", GameData.MapName.c_str());
	//				Players::ReadPlayerLists();
	//				StartLoadMapModel();
	//				break;
	//			default:
	//				break;
	//			}
	//		}

	//		Sleep(1500);
	//	}
	//}

	static void WriteFunction()
	{
		auto hScatter = mem.CreateScatterHandle();
		while (true)
		{

			if (!Utils::ValidPtr(GameData.GameInstance))
			{
				//Utils::Log(1, "GameData.GameInstance: %p", GameData.GameInstance);
				int Value = 2;
				mem.AddScatterRead(hScatter, GameData.GameInstance + 0x17fd, (int*)&Value);
				mem.ExecuteWriteScatter(hScatter);
			}

			Sleep(100);


		}

		mem.CloseScatterHandle(hScatter);
	}

	static void Init()
	{
		//Offset::Init();

		Offset::Sever_Init();
		if (GameData.Offset["UWorld"] == 0 || GameData.Offset["GameInstance"] == 0 || GameData.Offset["PlayerController"] == 0) {
			Utils::Log(2, "Offset init failed. Check Offset.h for hardcoded values.");
			system("pause");
			return;
		}

		EntityInit();

		for (const auto& item : EntityLists)
		{
			if (item.second.Type == EntityType::Item)
			{
				ItemDetail Detail;
				Detail.Name = item.first;
				Detail.DisplayName = item.second.DisplayName;
				Detail.Type = item.second.WeaponType;

				std::unordered_map<std::string, int> AWeaponGroupOverrides = {
					{"Groza", 1}, {"Beryl M762", 1}, {"M416", 1}, {"ACE32", 1}, {"FAMASI", 1},
					{"AUG", 1}, {"°ÍÀ×ÌØ", 1}, {"M24", 1}, {"AWM", 1}, {"Kar98k", 1},
					{"Mk14", 1}, {"Mk12", 1}, {"SKS", 1}, {"S1897", 1}, {"P90", 1},
					{"SLR", 1}, {"ÐÅºÅÇ¹", 1}, {"MG3", 1}
				};

				std::unordered_map<std::string, int> BWeaponGroupOverrides = {
					{"²¹³¥Æ÷ (²½Ç¹)", 2}, {"ÏûÒôÆ÷ (¾Ñ»÷Ç¹)", 2}, {"ºìµã", 2}, {"6±¶¾µ", 2},
					{"8±¶¾µ", 2}, {"ÈÈ³ÉÏñÃé×¼¾µ", 2}, {"15±¶¾µ", 2}, {"Õ½ÊõÇ¹ÍÐ", 2},
					{"´¹Ö±ÎÕ°Ñ", 2}, {"°ë½ØÎÕ°Ñ", 2}, {"¿ìËÙÀ©ÈÝµ¯Ï» (²½Ç¹)", 2}, {"¿ìËÙÀ©ÈÝµ¯Ï» (¾Ñ»÷Ç¹)", 2},
					{"À©ÈÝµ¯Ï» (¾Ñ»÷Ç¹)", 2}, {"Õ½ÊõÇ¹ÍÐ", 2}, {"ÍÐÈù°å", 2}, {"×Óµ¯´ü", 2}
				};

				std::unordered_map<std::string, int> CWeaponGroupOverrides = {
					{"·Àµ¯ÒÂLv3", 3}, {"Í·¿øLv3", 3}, {"Í·¿øLv2", 3}, {"±³°üLv3", 3},
					{"µç²¨¸ÉÈÅ±³°ü", 3}, {"±³°üLv2", 3}, {"·Àµ¯ÒÂLv2", 3}, {"¼ªÀû·þ", 3},
					{"ÊÖÀ×", 3}, {"ÑÌÎíµ¯", 3}, {"Æ½µ×¹ø", 3}, {"×Ô¾ÈÆ÷", 3},
					{"½ô¼±ºô¾ÈÆ÷", 3}, {"À¶É«¾§Æ¬·¢ÉäÆ÷", 3}, {"À¶É«¾§Æ¬", 3}, {"ÈýºÏÒ»Î¬ÐÞÌ×¼þ", 3}
				};

				std::unordered_map<std::string, int> DWeaponGroupOverrides = {
					{"ºÖÍåÔ¿³×", 4}, {"ÅÁÀ­ÄªÔ¿³×", 4}, {"Ì©¸êÔ¿³×", 4}, {"º£µºÔ¿³×", 4},
					{"Ñ©µØÃÅ½û¿¨", 4}, {"µÙË¹¶ÙÃÅ½û¿¨", 4}, {"¼±¾È°ü", 4}, {"ÄÜÁ¿ÒûÁÏ", 4},
					{"Ö¹ÌÛÒ©", 4}
				};

				// **ï¿½ï¿½Ê¼ï¿½ï¿½ÎªÎ´ï¿½ï¿½ï¿½ï¿½**
				int group = 0;

				auto it = AWeaponGroupOverrides.find(Detail.DisplayName);
				if (it != AWeaponGroupOverrides.end()) group = it->second;

				it = BWeaponGroupOverrides.find(Detail.DisplayName);
				if (it != BWeaponGroupOverrides.end()) group = it->second;

				it = CWeaponGroupOverrides.find(Detail.DisplayName);
				if (it != CWeaponGroupOverrides.end()) group = it->second;

				it = DWeaponGroupOverrides.find(Detail.DisplayName);
				if (it != DWeaponGroupOverrides.end()) group = it->second;

				// **Ö±ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ÕµÄ·ï¿½ï¿½ï¿½**
				Detail.Group = group;
				GameData.Config.Item.Lists[item.first] = Detail;
			}
		}

		Config::Load();

		if (!mem.Init("TslGame.exe", false, false))
		{
			Utils::Log(2, U8("[-] [OUTPUT] \u672A\u68C0\u6D4B\u5230\u786C\u4EF6\u5339\u914D\u7684\u52A0\u8F7D\u7B56\u7565"));
			system("pause");
		}
		else {
			Utils::Log(1, U8("[-] [OUTPUT] \u786C\u4EF6\u521D\u59CB\u5316\u6210\u529F\uFF0C\u7B49\u5F85\u70ED\u952E\u52A0\u8F7D"));
		}

		KeyState::Init();

		Data::SetGNameLists(EntityLists);
		Data::SetGNameListsByID({});


		std::thread UpdatePIDThread(UpdatePID);
		std::thread UpdateThread(Update);
		std::thread UpdateKeyStateThread(KeyState::Update);
		std::thread UpdateActorsThread(Actors::Update);
		std::thread UpdatePlayersThread(Players::Update);
		//std::thread UpdatePlayerFogsThread(Players::UpdateFogPlayers);
		std::thread UpdateVehiclesThread(Vehicles::Update);
		std::thread AimBotRunThread(AimBot::Run);
		std::thread UpdateCameraThread(UpdateCamera);
		std::thread UpdateItemsThread(Items::Update);
		std::thread UpdateProjectsThread(Projects::Update);
		std::thread UpdateRadarThread(Radar::Update);
		std::thread UpdateSegmentThread(Segment::Update);
		std::thread UpdateWebRadarThread(WebRadar::Rundata);
		std::thread recoilThread(Recoil::autoRecoil);
		//std::thread WriteFunctionThread(WriteFunction);
		UpdatePIDThread.join();

	}
};
