#pragma once
#include <Winsock2.h>
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Utils/KmBox.h>
#include <Utils/KmBoxNet.h>
#include <Hack/Players.h>
#include <iostream>

namespace KeyState
{
	void Init()
	{
		GameData.Keyboard = mem.GetKeyboard();

		if (!GameData.Keyboard.InitKeyboard())
		{
			Utils::Log(2, U8("[-] [OUTPUT] \u521d\u59cb\u5316\u70ed\u952e\u5931\u8d25\uff01\u8bf7\u91cd\u542f\u4e3b\u673a\uff01\uff01"));
		}
		else {
			Utils::Log(1, U8("[-] [OUTPUT] \u521d\u59cb\u5316\u70ed\u952e\u6210\u529f\uff01\u8bf7\u5f00\u59cb\u8868\u6f14\uff01\uff01"));
			GameData.Keyboard.InitCursorPosition();//获取失败无法使用游戏机鼠标
			//Utils::Log(1, U8("[-] [OUTPUT] 游戏机鼠标获取成功，当前可使用游戏机鼠标： % llx" ), GameData.Keyboard.GetAddrss());
		}
	}
	void Update() {
		Utils::Log(1, U8("[-] [OUTPUT] \u5f00\u59cb\u521d\u59cb\u5316 \u7ed8\u5236 & \u96f7\u8fbe"));
		while (true)
		{
			GameData.Keyboard.UpdateKeys();

			std::unordered_map<int, std::vector<std::string>> Keys;
			Keys[GameData.Config.Menu.ShowKey].push_back("Menu");
			Keys[GameData.Config.Overlay.Quit_key].push_back("DEAD");
			Keys[GameData.Config.AimBot.Configs[0].Key].push_back("AimBotConfig0");
			Keys[GameData.Config.AimBot.Configs[1].Key].push_back("AimBotConfig1");
			Keys[VK_DELETE].push_back("RecoverOverlay");
			Keys[GameData.Config.Function.ClearKey].push_back("Clear");
			Keys[GameData.Config.Item.GroupKey].push_back("GroupKey");
			Keys[GameData.Config.Item.GroupAKey].push_back("GroupAKey");
			Keys[GameData.Config.Item.GroupBKey].push_back("GroupBKey");
			Keys[GameData.Config.Item.GroupCKey].push_back("GroupCKey");
			Keys[GameData.Config.Item.GroupDKey].push_back("GroupDKey");
			Keys[GameData.Config.Vehicle.EnableKey].push_back("VehicleEnable");
			Keys[GameData.Config.PlayerList.MarkKey].push_back("PlayerListMarkType");
			Keys[GameData.Config.ESP.FocusModeKey].push_back("FocusModeKey");
			Keys[GameData.Config.AirDrop.EnableKey].push_back("AirDropEnableKey");
			Keys[GameData.Config.DeadBox.EnableKey].push_back("DeadBoxEnableKey");
			Keys[GameData.Config.Overlay.ModeKey].push_back("FusionModeKEY");
			Keys[GameData.Config.ESP.duiyouKey].push_back("duiyouKey");
			Keys[GameData.Config.ESP.AimbotHotkey].push_back("AimbotHotkey");
			Keys[GameData.Config.ESP.DataSwitchkey].push_back("DataSwitchkey");
			Keys[GameData.Config.ESP.Playerskey].push_back("Playerskey");
			Keys[GameData.Config.Overlay.ObtainModel].push_back("ObtainModel");
			Keys[GameData.Config.Overlay.ReloadModel].push_back("ReloadModel");
			//Keys[GameData.Config.ESP.Mousekey].push_back("Mousekey");

			for (auto Key : Keys)
			{
				if (GameData.Keyboard.WasKeyPressed(Key.first))
				{
					for (auto KeyName : Key.second)
					{
						if (KeyName == "AirDropEnableKey")
						{
							GameData.Config.AirDrop.Enable = !GameData.Config.AirDrop.Enable;
						}

						if (KeyName == "duiyouKey")
						{
							GameData.Config.ESP.duiyou = !GameData.Config.ESP.duiyou;
						}

						if (KeyName == "AimbotHotkey")
						{
							GameData.Config.AimBot.Enable = !GameData.Config.AimBot.Enable;
						}

						if (KeyName == "DataSwitchkey")
						{
							GameData.Config.ESP.DataSwitch = !GameData.Config.ESP.DataSwitch;
						}
						/*if (KeyName == "Mousekey")
						{
							GameData.Config.ESP.Mouse = !GameData.Config.ESP.Mouse;
						}*/
						if (KeyName == "Playerskey")
						{
							GameData.Config.Window.Players = !GameData.Config.Window.Players;
						}

						if (KeyName == "FusionModeKEY")
						{
							// �л� FusionMode ״̬
							GameData.Config.Overlay.FusionMode = !GameData.Config.Overlay.FusionMode;

						}


						if (KeyName == "DeadBoxEnableKey")
						{
							GameData.Config.DeadBox.Enable = !GameData.Config.DeadBox.Enable;
						}

						if (KeyName == "FocusModeKey")
						{
							GameData.Config.ESP.FocusMode = !GameData.Config.ESP.FocusMode;
						}

						if (KeyName == "ReloadModel")
						{
							GameData.ReloadModelRequested.store(true);
						}

						if (KeyName == "ObtainModel")//  �����ǻ�ȡģ��ȥ��˿��

						{
							auto mesh = std::atomic_load(&GameData.NextHintMeshData);
							if (!mesh || !mesh->HasDebugMesh()) {
								Utils::Log(1, "[Debug] No debug mesh captured. Enable model preview before exporting.");
								return;
							}
							const auto& debugMesh = *mesh->DebugMesh;
							// ========== 2. C++20 std::format �����ļ���������+���Ͱ�ȫ�� ==========
							std::string filename = std::format("{}_{}.obj", debugMesh.Vertices.size(), debugMesh.Indices.size());
							std::ofstream outFile(filename, std::ios::out | std::ios::trunc); // ��ʽָ��дģʽ����ѡ��

							if (!outFile.is_open()) {
								return;
							}

							// д�붥������
							for (const auto& vertex : debugMesh.Vertices) {
								outFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
							}

							// д��������
							for (size_t i = 0; i < debugMesh.Indices.size(); i += 3) {
								// OBJ�ļ���������1��ʼ,����Ҫ+1
								outFile << "f " << debugMesh.Indices[i] + 1 << " "
									<< debugMesh.Indices[i + 1] + 1 << " "
									<< debugMesh.Indices[i + 2] + 1 << "\n";
							}

							outFile.close();
							Utils::Log(1, "[Debug] Exported mesh with %zu vertices and %zu indices", debugMesh.Vertices.size(), debugMesh.Indices.size());
						}

						if (KeyName == "DEAD")//  �����ǽ���
						{

							HWND Progman = FindWindowA("Progman", NULL);
							HWND TrayWnd = FindWindowA("Shell_TrayWnd", NULL);
							ShowWindow(Progman, SW_SHOW);
							ShowWindow(TrayWnd, SW_SHOW);

							ExitProcess(1);
							exit(1);
						}

						if (KeyName == "Clear")
						{
							bool shouldLog = true; // ������Ҫʱ��¼��־

							if (GameData.Config.AimBot.Connected) {
								if (GameData.Config.AimBot.Controller == 0) {
									KmBox::Clear();
									if (shouldLog) Utils::Log(1, "KMBOX Clear Success");
								}
								else if (GameData.Config.AimBot.Controller == 1) {
									KmBoxNet::Clear();
									if (shouldLog) Utils::Log(1, "KMBOXNET Clear Success");
								}
							}

							Data::SetCacheEntitys({});
							Data::SetCachePlayers({});
							Data::SetPlayers({});
							Data::SetPlayersData({});
							Data::SetCacheVehicles({});
							Data::SetVehicles({});
							Data::SetVehiclWheels({});
							Data::SetItems({});
							Data::SetCacheDroppedItems({});
							Data::SetCacheDroppedItemGroups({});
							GameData.AimBot.Target = 0;
							GameData.AimBot.Lock = false;
							bool shouldRefresh = false;
							// ֻ�е�ȷʵ��Ҫˢ��ʱ�ŵ���
							if (shouldRefresh) {
								mem.RefreshAll();
							}
						}
						

						if (KeyName == "GroupKey")
						{
							if (GameData.Config.Item.ShowGroup != 6)
							{
								GameData.Config.Item.Enable = 1;
								GameData.Config.Item.ShowGroup++;
								GameData.Config.Item.ShowGroups.clear();
							}
							else {
								GameData.Config.Item.ShowGroup = 0;
								GameData.Config.Item.Enable = 0;
								GameData.Config.Item.ShowGroups.clear();
							}
						}

						if (KeyName == "GroupAKey")
						{
							GameData.Config.Item.Enable = 1;
							if (GameData.Config.Item.ShowGroups.count(1)) {
								GameData.Config.Item.ShowGroups.erase(1);
							} else {
								GameData.Config.Item.ShowGroups.insert(1);
							}
						}

						if (KeyName == "GroupBKey")
						{
							GameData.Config.Item.Enable = 1;
							if (GameData.Config.Item.ShowGroups.count(2)) {
								GameData.Config.Item.ShowGroups.erase(2);
							} else {
								GameData.Config.Item.ShowGroups.insert(2);
							}
						}

						if (KeyName == "GroupCKey")
						{
							GameData.Config.Item.Enable = 1;
							if (GameData.Config.Item.ShowGroups.count(3)) {
								GameData.Config.Item.ShowGroups.erase(3);
							} else {
								GameData.Config.Item.ShowGroups.insert(3);
							}
						}

						if (KeyName == "GroupDKey")
						{
							GameData.Config.Item.Enable = 1;
							if (GameData.Config.Item.ShowGroups.count(4)) {
								GameData.Config.Item.ShowGroups.erase(4);
							} else {
								GameData.Config.Item.ShowGroups.insert(4);
							}
						}


						if (KeyName == "VehicleEnable")
						{
							GameData.Config.Vehicle.Enable = !GameData.Config.Vehicle.Enable;
						}
						if (KeyName == "Menu")
						{
							GameData.Config.Menu.Show = !GameData.Config.Menu.Show;

							if (GameData.Config.Menu.Show)
							{
								SetForegroundWindow(GameData.Config.Overlay.hWnd);
							}
							else {
								SetForegroundWindow(GetDesktopWindow());
							}
						}
						if (KeyName == "AimBotConfig0")
						{
							GameData.Config.AimBot.ConfigIndex = 0;
						}
						if (KeyName == "AimBotConfig1")
						{
							GameData.Config.AimBot.ConfigIndex = 1;
						}
					}
				}
			}

			Sleep(10);
		}
	}
}
