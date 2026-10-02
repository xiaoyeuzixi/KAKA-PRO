#pragma once
#include <Common/Data.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <Mosquitto/mosquitto.h>
#include <TlHelp32.h>
#pragma comment(lib, "mosquitto.lib")

static struct mosquitto* mosq;
static int rc = -1;

class WebRadar {
public:
	static void on_connect(struct mosquitto* mosq, void*, int rc) {
		if (rc == 0) {
			GameData.Config.WebRadar.isWebRadarConnect = true;
		}
		else {
			std::cerr << "Failed to connect to MQTT broker with code " << rc << std::endl;
		}
	}
	static void on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
		std::cout << "Received message on topic " << message->topic << ": " << (char*)message->payload << std::endl;
	}
	static void on_disconnec(struct mosquitto* mosq, void*, int rc) {
		while (true)
		{
			if (mosquitto_reconnect(mosq) == MOSQ_ERR_SUCCESS)
				break;
			Sleep(5000);
		}
	}
	static std::string GB2312ToUTF8(const std::string& gb2312Str) {
		int unicodeLen = MultiByteToWideChar(CP_ACP, 0, gb2312Str.c_str(), -1, NULL, 0);
		wchar_t* wideStr = new wchar_t[unicodeLen + 1];
		MultiByteToWideChar(CP_ACP, 0, gb2312Str.c_str(), -1, wideStr, unicodeLen);
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
		char* utf8Str = new char[utf8Len + 1];
		WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, utf8Len, NULL, NULL);
		std::string result(utf8Str);
		delete[] wideStr;
		delete[] utf8Str;
		return result;
	}
	static void Init() {
		std::unordered_map<std::string, int> nameToIndexVehicle = {
		   {"直升机", 0},
		   {"紧急呼救器", 1},
		   {"飞机", 2},
		   {"自行车", 3},
		   {"装甲车", 4},
		   {"吉普", 5},
		   {"皮卡", 6},
		   {"滑翔机", 7},
		   {"物资车", 8},
		   {"摩托艇", 9},
		   {"快艇", 10},
		   {"汽艇", 11},
		   {"皮卡车", 12},
		   {"跑车", 13},
		   {"超跑", 14},
		   {"摩托车", 15},
		   {"雪地吉普", 16},
		   {"皮卡", 17},
		   {"公交车", 18},
		   {"踏板车", 19},
		   {"雪地摩托", 20},
		   {"沙地车", 21},
		   {"汽车", 22},
		   {"双人跑车", 23},
		   {"越野摩托", 24},
		   {"全地形摩托车", 25},
		   {"货车", 26},
		   {"警车", 27},
		   {"食品运输车", 28},
		   {"NEOX BLANC SUV", 29}
		};
		int vehicleIndex = 13;
		while (true)
		{
			try
			{

				if (std::strlen(GameData.Config.WebRadar.SubTitle) == 0) {
					continue;
				};
				if (!GameData.Config.WebRadar.isWebRadarEnable && !GameData.Config.WebRadar.isWebRadarConnect) {
					Sleep(500);
					continue;
				}
				if (GameData.Config.WebRadar.isWebRadarEnable == false && GameData.Config.WebRadar.isWebRadarConnect)
				{
					Destroy();
				}
				if (GameData.Config.WebRadar.isWebRadarConnect == false && GameData.Config.WebRadar.isWebRadarEnable)
				{
					Connect();
				}
				nlohmann::json Config;
				if (GameData.Scene == Scene::Gaming) {

					Config["Map"] = GameData.MapName;
					Config["MapX"] = GameData.Radar.WorldOriginLocation.X;
					Config["MapY"] = GameData.Radar.WorldOriginLocation.Y;
					Config["Game"][0][0] = GameData.Radar.SafetyZonePosition.X;
					Config["Game"][0][1] = GameData.Radar.SafetyZonePosition.Y;
					Config["Game"][0][2] = GameData.Radar.SafetyZoneRadius;
					Config["Game"][1][0] = GameData.Radar.BlueZonePosition.X;
					Config["Game"][1][1] = GameData.Radar.BlueZonePosition.Y;
					Config["Game"][1][2] = GameData.Radar.BlueZoneRadius;
					auto Players = Data::GetPlayers();
					int i = 0;
					for (auto Item : Players) {
						auto Player = Item.second;
						if (Player.InFog || (Player.State == CharacterState::Dead))
						{
							continue;
						}
						FVector AimDirection = FRotator(0.0f, Player.AimOffsets.Yaw, 0.0f).GetUnitVector();
						FVector2D Direction = FVector2D{ AimDirection.X, AimDirection.Y };
						float AngleRadians = atan2(Direction.Y, Direction.X);
						float AngleDegrees = AngleRadians;
						float sin_a = sinf(AngleDegrees), cos_a = cosf(AngleDegrees);
						Config["Player"][i][0] = Player.Location.X + GameData.Radar.WorldOriginLocation.X;
						Config["Player"][i][1] = Player.Location.Y + GameData.Radar.WorldOriginLocation.Y;
						Config["Player"][i][2] = Player.Distance;
						Config["Player"][i][3] = Player.TeamID;
						Config["Player"][i][4] = Player.Health;
						Config["Player"][i][5] = Player.KillCount;
						Config["Player"][i][6] = Player.SpectatedCount;
						Config["Player"][i][7] = Player.AimOffsets.Yaw;
						Config["Player"][i][8] = Player.IsMyTeam;
						Config["Player"][i][9] = Player.Type == EntityType::AI;
						Config["Player"][i][10] = Player.IsVisible;
						Config["Player"][i][11] = Player.IsMe;
						Config["Player"][i][12] = Player.IsAimMe;
						Config["Player"][i][13] = Player.GroggyHealth;
						Config["Player"][i][14] = Player.WeaponName;
						Config["Player"][i][15] = Player.SquadMemberIndex;
						Config["Player"][i][16] = Player.Name;
						int* teamNumberColor = GetColorForNumber(Player.TeamID);
						Config["Player"][i][17] = Player.Name;
						auto infoColor = GameData.Config.ESP.Color.Default.Info;
						Config["Player"][i][17] = rgb2hex(teamNumberColor[0], teamNumberColor[1], teamNumberColor[2], true);
						Config["Player"][i][18] = cos_a;
						Config["Player"][i][19] = sin_a;
						Config["Player"][i][20] = Player.ClanName;
						i++;
					}

					std::unordered_map<uint64_t, VehicleInfo> Vehicles = Data::GetVehicles();
					for (auto item : Vehicles)
					{
						auto Vehicle = item.second;
						auto it = nameToIndexVehicle.find(Vehicle.Name);
						if (it != nameToIndexVehicle.end()) {
							vehicleIndex = it->second;
						}
						else {
							vehicleIndex = 13;
						}
						Config["Car"][i][0] = vehicleIndex;
						Config["Car"][i][1] = Vehicle.Location.X + GameData.Radar.WorldOriginLocation.X;
						Config["Car"][i][2] = Vehicle.Location.Y + GameData.Radar.WorldOriginLocation.Y;
					}
					Send(Config.dump());
				}
				Sleep(50);
			}
			catch (const std::exception&e)
			{
			}
		}
		Destroy();
	}
	static void Connect() {
		try
		{
			OpenMqtt();
			mosquitto_lib_init();
			mosq = mosquitto_new(NULL, true, NULL);
			if (!mosq) {
				return;
			}
			mosquitto_connect_callback_set(mosq, on_connect);
			mosquitto_disconnect_callback_set(mosq, on_disconnec);
			while (rc != MOSQ_ERR_SUCCESS)
			{
				rc = mosquitto_connect(mosq, GameData.Config.WebRadar.IP, std::atoi(GameData.Config.WebRadar.Port), 600);
				if (rc != MOSQ_ERR_SUCCESS) {
				}
				GameData.Config.WebRadar.isWebRadarConnect = true;
				GameData.Config.WebRadar.RadarUrl = "http://27.25.140.207:8888/?27.25.140.207&sub=" + std::string(GameData.Config.WebRadar.SubTitle);
			}
			

		}
		catch (const std::exception&)
		{

		}
	}
	static void Destroy() {
		GameData.Config.WebRadar.isWebRadarConnect = false;
		mosquitto_lib_cleanup();
		mosquitto_destroy(mosq);
	}
	static bool Send(std::string message) {
		auto ret = mosquitto_pub_topic_check(GameData.Config.WebRadar.SubTitle);
		if (mosquitto_publish(mosq, NULL, GameData.Config.WebRadar.SubTitle, strlen(message.c_str()), message.c_str(), 0, false) == MOSQ_ERR_SUCCESS) {
			return true;
		}
		return false;
	}


	static bool getProcess(const char* processName)
	{
		WCHAR pName[MAX_PATH];
		mbstowcs(pName, processName, MAX_PATH);  // 将 char 转换为 WCHAR
		CharLowerBuffW(pName, MAX_PATH);  // 使用 CharLowerBuffW 处理小写

		PROCESSENTRY32 currentProcess;
		currentProcess.dwSize = sizeof(currentProcess);
		HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hProcess == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		bool bMore = Process32First(hProcess, &currentProcess);
		while (bMore)
		{
			CharLowerBuffW(currentProcess.szExeFile, MAX_PATH);  // 使用 CharLowerBuffW 处理小写
			if (wcscmp(currentProcess.szExeFile, pName) == 0)  // 使用 wcscmp 比较宽字符字符串
			{
				CloseHandle(hProcess);
				return true;
			}
			bMore = Process32Next(hProcess, &currentProcess);
		}

		CloseHandle(hProcess);
		return false;
	}

	static void OpenMqtt() {
		if (getProcess("mqtt.exe"))
		{
			return;
		}
		char pBuf[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, pBuf);
		char str3[MAX_PATH];
		strcpy(str3, pBuf);
		strcat(str3, "\\mqtt");
		SHELLEXECUTEINFOA sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = "open";
		sei.lpFile = "mqtt.exe";
		sei.lpParameters = "-c mqtt.conf";
		sei.lpDirectory = str3;
		sei.nShow = SW_HIDE;
		ShellExecuteExA(&sei);
		CloseHandle(sei.hProcess);

	}
	static std::string rgb2hex(int r, int g, int b, bool with_head)
	{
		std::stringstream ss;
		if (with_head)
			ss << "#";
		ss << std::hex << (r << 16 | g << 8 | b);
		return ss.str();
	}
	static int* SetColor(int a, int b, int c) {
		int* temp = new int[3];
		temp[0] = a;
		temp[1] = b;
		temp[2] = c;
		return temp;
	}
	static int* GetColorForNumber(int number) {
		switch (number) {
		case 1:  return SetColor(247, 248, 19);    // Yellow
		case 2:  return SetColor(250, 127, 73);    // Orange
		case 3:  return SetColor(90, 198, 227);    // Light Blue
		case 4:  return SetColor(90, 189, 77);     // Green
		case 5:  return SetColor(225, 99, 120);    // Pink
		case 6:  return SetColor(115, 129, 168);   // Purple
		case 7:  return SetColor(159, 126, 105);   // Indigo
		case 8:  return SetColor(255, 134, 200);   // Light Cyan
		case 9:  return SetColor(210, 224, 191);   // Pale Green
		case 10: return SetColor(154, 52, 142);    // Violet
		case 11: return SetColor(98, 146, 158);       // Red
		case 12: return SetColor(226, 214, 239);       // Green
		case 13: return SetColor(4, 167, 119);       // Blue
		case 14: return SetColor(115, 113, 252);     // Yellow
		case 15: return SetColor(255, 0, 255);     // Magenta
		case 16: return SetColor(93, 46, 140);     // Cyan
		case 17: return SetColor(0, 255, 0);       // Lime
		case 18: return SetColor(0, 0, 255);       // Blue
		case 19: return SetColor(255, 165, 0);     // Orange
		case 20: return SetColor(128, 0, 128);     // Purple
		case 21: return SetColor(255, 192, 203);   // Pink
		case 22: return SetColor(128, 128, 0);     // Olive
		case 23: return SetColor(255, 215, 0);     // Gold
		case 24: return SetColor(75, 0, 130);      // Indigo
		case 25: return SetColor(0, 191, 255);     // Deep Sky Blue
		case 26: return SetColor(255, 105, 180);   // Hot Pink
		case 27: return SetColor(139, 69, 19);     // Saddle Brown
		case 28: return SetColor(220, 20, 60);     // Crimson
		case 29: return SetColor(0, 255, 127);     // Spring Green
		case 30: return SetColor(0, 250, 154);     // Medium Spring Green
		case 31: return SetColor(72, 61, 139);     // Dark Slate Blue
		case 32: return SetColor(143, 188, 143);   // Dark Sea Green
		case 33: return SetColor(178, 34, 34);     // Firebrick
		case 34: return SetColor(153, 50, 204);    // Dark Orchid
		case 35: return SetColor(233, 150, 122);   // Dark Salmon
		case 36: return SetColor(148, 0, 211);     // Dark Violet
		case 37: return SetColor(95, 158, 160);    // Cadet Blue
		case 38: return SetColor(127, 255, 212);   // Aquamarine
		case 39: return SetColor(218, 112, 214);   // Orchid
		case 40: return SetColor(244, 164, 96);    // Sandy Brown
		case 41: return SetColor(210, 105, 30);    // Chocolate
		case 42: return SetColor(222, 184, 135);   // Burlywood
		case 43: return SetColor(255, 228, 181);   // Moccasin
		case 44: return SetColor(255, 239, 213);   // Papaya Whip
		case 45: return SetColor(175, 238, 238);   // Pale Turquoise
		case 46: return SetColor(100, 149, 237);   // Cornflower Blue
		case 47: return SetColor(219, 112, 147);   // Pale Violet Red
		case 48: return SetColor(173, 216, 230);   // Light Blue
		case 49: return SetColor(240, 128, 128);   // Light Coral
		case 50: return SetColor(255, 248, 220);   // Cornsilk
		default: return SetColor(102, 102, 102);   // Gray
		}
	}
};