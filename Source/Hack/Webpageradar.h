#pragma once
#include <Common/Data.h>
#include <nlohmann/json.hpp>
#include "WebSocketClient.h"
#include <wininet.h>
#include <iostream>
#include <string>
#include <zlib.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <TlHelp32.h>
#include <tchar.h>
#include <codecvt>
#include <chrono>
#include <boost/asio.hpp>

#pragma comment(lib, "wininet.lib")

using json = nlohmann::json;
using namespace std::chrono;

// 检查字符串是否为合法 UTF-8
bool isValidUTF8(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	try {
		converter.from_bytes(str);
		return true;
	}
	catch (...) {
		return false;
	}
}

// 将 GBK 编码字符串转换为 UTF-8
std::string gbkToUtf8(const std::string& gbkStr) {
	try {
		std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> converter(
			new std::codecvt_byname<wchar_t, char, std::mbstate_t>("zh_CN.GBK"));
		std::wstring wideStr = converter.from_bytes(gbkStr);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8Converter;
		return utf8Converter.to_bytes(wideStr);
	}
	catch (...) {
		return ""; // 转换失败返回空字符串
	}
}

// Base64编码表
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

// Base64编码函数
static std::string base64_encode(const std::string& input) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	size_t in_len = input.length();
	const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*>(input.c_str());

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; i < 4; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; j < i + 1; j++)
			ret += base64_chars[char_array_4[j]];

		while (i++ < 3)
			ret += '=';
	}

	return ret;
}

std::string compressString(const std::string& str) {
	try {
		uLongf compressedSize = compressBound(str.size());
		std::vector<unsigned char> compressedData(compressedSize);

		if (compress(compressedData.data(), &compressedSize,
			reinterpret_cast<const Bytef*>(str.data()), str.size()) != Z_OK) {
			throw std::runtime_error("Compression failed");
		}

		compressedData.resize(compressedSize);
		return std::string(compressedData.begin(), compressedData.end());
	}
	catch (const std::exception& e) {
		std::cerr << "Compression error: " << e.what() << std::endl;
		return str; // 如果压缩失败，返回原始字符串
	}
}

class WebRadar
{
private:
	static std::unique_ptr<WebSocketClient> wsClient;
public:
	static bool InitWebSocket() {
		if (!wsClient) {
			wsClient = std::make_unique<WebSocketClient>();
		}

		if (!wsClient->IsConnected()) {
			// 从用户配置中获取IP和PIN
			std::string ip(GameData.Config.WebRadar.IP);
			std::string port(GameData.Config.WebRadar.Port);
			std::string pin(GameData.Config.WebRadar.PIN);

			// 建立新连接
			bool success = wsClient->Connect(ip + ":" + port, pin);
			if (!success) {
				if (wsClient->GetPinStatus() == WebSocketClient::PinStatus::Error) {
					GameData.Config.WebRadar.isWebRadarEnable = false;
					GameData.Config.WebRadar.isWebRadarConnect = false;
				}
				return false;
			}
			return success;
		}
		return true;
	}

	static inline ImVec4 GetColorFor_Number(int number) {
		switch (number) {
		case 1:  return ImVec4(247, 248, 19, 255);    // Yellow
		case 2:  return ImVec4(250, 127, 73, 255);    // Orange
		case 3:  return ImVec4(90, 198, 227, 255);    // Light Blue
		case 4:  return ImVec4(90, 189, 77, 255);     // Green
		case 5:  return ImVec4(225, 99, 120, 255);    // Pink
		case 6:  return ImVec4(115, 129, 168, 255);   // Purple
		case 7:  return ImVec4(159, 126, 105, 255);   // Indigo
		case 8:  return ImVec4(255, 134, 200, 255);   // Light Cyan
		case 9:  return ImVec4(210, 224, 191, 255);   // Pale Green
		case 10: return ImVec4(154, 52, 142, 255);    // Violet
		case 11: return ImVec4(98, 146, 158, 255);       // Red
		case 12: return ImVec4(226, 214, 239, 255);       // Green
		case 13: return ImVec4(4, 167, 119, 255);       // Blue
		case 14: return ImVec4(115, 113, 252, 255);     // Yellow
		case 15: return ImVec4(255, 0, 255, 255);     // Magenta
		case 16: return ImVec4(93, 46, 140, 255);     // Cyan
		case 17: return ImVec4(0, 255, 0, 255);       // Lime
		case 18: return ImVec4(0, 50, 255, 255);       // Blue
		case 19: return ImVec4(255, 165, 0, 255);     // Orange
		case 20: return ImVec4(128, 0, 128, 255);     // Purple
		case 21: return ImVec4(255, 192, 203, 255);   // Pink
		case 22: return ImVec4(128, 128, 0, 255);     // Olive
		case 23: return ImVec4(255, 215, 0, 255);     // Gold
		case 24: return ImVec4(75, 0, 130, 255);      // Indigo
		case 25: return ImVec4(0, 191, 255, 255);     // Deep Sky Blue
		case 26: return ImVec4(255, 105, 180, 255);   // Hot Pink
		case 27: return ImVec4(139, 69, 19, 255);     // Saddle Brown
		case 28: return ImVec4(220, 20, 60, 255);     // Crimson
		case 29: return ImVec4(0, 255, 127, 255);     // Spring Green
		case 30: return ImVec4(0, 250, 154, 255);     // Medium Spring Green
		case 31: return ImVec4(72, 61, 139, 255);     // Dark Slate Blue
		case 32: return ImVec4(143, 188, 143, 255);   // Dark Sea Green
		case 33: return ImVec4(178, 34, 34, 255);     // Firebrick
		case 34: return ImVec4(153, 50, 204, 255);    // Dark Orchid
		case 35: return ImVec4(233, 150, 122, 255);   // Dark Salmon
		case 36: return ImVec4(148, 0, 211, 255);     // Dark Violet
		case 37: return ImVec4(95, 158, 160, 255);    // Cadet Blue
		case 38: return ImVec4(127, 255, 212, 255);   // Aquamarine
		case 39: return ImVec4(218, 112, 214, 255);   // Orchid
		case 40: return ImVec4(244, 164, 96, 255);    // Sandy Brown
		case 41: return ImVec4(210, 105, 30, 255);    // Chocolate
		case 42: return ImVec4(222, 184, 135, 255);   // Burlywood
		case 43: return ImVec4(255, 228, 181, 255);   // Moccasin
		case 44: return ImVec4(255, 239, 213, 255);   // Papaya Whip
		case 45: return ImVec4(175, 238, 238, 255);   // Pale Turquoise
		case 46: return ImVec4(100, 149, 237, 255);   // Cornflower Blue
		case 47: return ImVec4(219, 112, 147, 255);   // Pale Violet Red
		case 48: return ImVec4(173, 216, 230, 255);   // Light Blue
		case 49: return ImVec4(240, 128, 128, 255);   // Light Coral
		case 50: return ImVec4(255, 248, 220, 255);   // Cornsilk
		default: return ImVec4(102, 102, 102, 255);   // Gray
		}
	}

	static int ScaleConversion(float Location)
	{
		float Scale = 1.f;
		if (GameData.MapName == "Baltic_Main") Scale = 0.99609375f;
		return static_cast<int>(std::round(Location * Scale));
	}
	static void Rundata() {
		std::vector<std::pair<std::string, int>> playerArray;

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
		auto last_time = high_resolution_clock::now();
		auto nextTime = steady_clock::now() + milliseconds(10);
		while (true) {
			std::this_thread::sleep_until(nextTime);
			nextTime += milliseconds((1000 / GameData.Config.WebRadar.fps));

			auto now = high_resolution_clock::now();

			bool send_item = false;

			auto elapsed = duration_cast<milliseconds>(now - last_time).count();
			if (elapsed >= 1000) {
				send_item = true;
				last_time = now;
			}

			try
			{
				if (GameData.Config.WebRadar.isWebRadarEnable)
				{
					if (!InitWebSocket()) {
						Sleep(GameData.ThreadSleep);
						continue;
					}
				}

				if (!GameData.Config.WebRadar.isWebRadarEnable) {
					if (wsClient) {
						wsClient->Disconnect();
						wsClient.reset();
					}
					Sleep(GameData.ThreadSleep);
					continue;
				}
			}
			catch (...) {}

			json j = json::array();
			if (GameData.Scene != Scene::Gaming)
			{
				j = json::array({
				   json::array({
					   GameData.Scene
				  })
					});

				try
				{
					std::string data_str = j.dump();
					std::string compressed = compressString(data_str);

					//std::cout << "SendMessage:Wait" << std::endl;
					if (wsClient && wsClient->IsConnected()) {
						//std::cout << "SendMessage:Success" << std::endl;
						wsClient->SendBinaryMessage(compressed);
					}
				}
				catch (...) {}

				// Sleep(5);

				continue;
			}
			// 添加雷达信息
			json i = json::array();
			json v = json::array();
			json items = json::array();
			json packages = json::array();

			std::unordered_map<uint64_t, Player> Players = Data::GetPlayers();

			if (playerArray.size() > 4)
				playerArray.clear();

			int a = 0;
			for (auto& pair : Players) {
				Player& detail = pair.second;

				if (detail.bAlwaysCreatePhysicsState == 4)
					continue;

				// 检查 detail.Name 是否为合法 UTF-8
				if (!isValidUTF8(detail.Name)) {
					detail.Name = gbkToUtf8(detail.Name);
					if (detail.Name.empty()) {
						detail.Name = "AI";
					}
				}

				// 检查 playerArray 是否包含 detail.Name
				auto it = std::find_if(playerArray.begin(), playerArray.end(), [&detail](const std::pair<std::string, int>& p) {
					return p.first == detail.Name;
					});

				if (detail.IsMyTeam && !detail.IsMe) {
					if (it == playerArray.end()) {
						// 如果 detail.Name 不在数组中，添加并赋值 a+1
						playerArray.push_back(std::make_pair(detail.Name, a + 1));
						a = a + 1;
					}
					else {
						// 如果 detail.Name 已经在数组中，使用对应的 int 值
						a = it->second;
					}
				}

				FVector AimDirection = FRotator(0.0f, detail.AimOffsets.Yaw, 0.0f).GetUnitVector();
				FVector2D Direction = FVector2D{ AimDirection.X, AimDirection.Y };
				float AngleRadians = atan2(Direction.Y, Direction.X);
				float AngleDegrees = AngleRadians;

				// std::cout << AngleDegrees << std::endl;
				float sin_a = sinf(AngleDegrees), cos_a = cosf(AngleDegrees);

				ImVec4 TeanColor = GetColorFor_Number(detail.TeamID);
				if (detail.TeamID >= 200)
				{
					TeanColor = { 0 , 255 , 0 ,0 };
				}
				ImVec4 NameColor = { 255, 255, 255, 0 };

				if (detail.PartnerLevel > 0) {
					NameColor = { 0, 255, 0, 0 };
				}

				if (detail.Health == 0 and detail.GroggyHealth <= 100) {
					TeanColor = { 255, 0, 0, 0 };
				}

				json playerJson = json::array({
					detail.ClanName,
					detail.Name,
					ScaleConversion(detail.Location.X + GameData.Radar.WorldOriginLocation.X),
					ScaleConversion(detail.Location.Y + GameData.Radar.WorldOriginLocation.Y),
					ScaleConversion(detail.Location.Z + GameData.Radar.WorldOriginLocation.Z),
					static_cast<int>(std::round(AngleDegrees * 100000.0)) / 100000.0,
					detail.PartnerLevel,
					detail.TeamID,
					static_cast<int>(detail.Health),
					static_cast<int>(detail.GroggyHealth),
					detail.IsMyTeam,
					detail.KillCount,
					//detail.DamageDealtOnEnemy,
					detail.SpectatedCount,
					detail.SquadMemberIndex,
					});
				i.push_back(playerJson);
			}

			std::unordered_map<uint64_t, VehicleInfo> Vehicles = Data::GetVehicles();
			for (auto& pair : Vehicles) {
				VehicleInfo& detail = pair.second;

				auto it = nameToIndexVehicle.find(detail.Name);
				if (it != nameToIndexVehicle.end()) {
					vehicleIndex = it->second;
				}
				else {
					vehicleIndex = 13;
				}

				json playerJson = json::array({
					vehicleIndex,
					ScaleConversion(detail.Location.X + GameData.Radar.WorldOriginLocation.X),
					ScaleConversion(detail.Location.Y + GameData.Radar.WorldOriginLocation.Y),
					ScaleConversion(detail.Location.Z + GameData.Radar.WorldOriginLocation.Z),
					});
				v.push_back(playerJson);
			}

			std::unordered_map<uint64_t, ItemInfo> gameItems = Data::GetItems();
			for (const auto& pair : gameItems) {
				const ItemInfo& item = pair.second;
				/*if (item.bHidden || GameData.Config.Item.Lists[item.Name].Group == 0)
					continue;*/

				json itemJson = json::array({
					item.Name,
					ScaleConversion(item.Location.X + GameData.Radar.WorldOriginLocation.X),
					ScaleConversion(item.Location.Y + GameData.Radar.WorldOriginLocation.Y)
					});
				items.push_back(itemJson);
			}

			std::unordered_map<uint64_t, PackageInfo> Packages = Data::GetPackages();
			for (const auto& pair : Packages) {
				const PackageInfo& item = pair.second;
				if (item.Type != EntityType::AirDrop) {
					continue;
				}
				json itemJson = json::array({
					ScaleConversion(item.Location.X + GameData.Radar.WorldOriginLocation.X),
					ScaleConversion(item.Location.Y + GameData.Radar.WorldOriginLocation.Y)
					});

				json packageItems = json::array();
				for (const auto& packageItem : item.Items) {
					packageItems.push_back(packageItem.Name);
				}
				itemJson.push_back(packageItems);
				packages.push_back(itemJson);

			}

			// 填充主 JSON 对象 j
			j = json::array({
				json::array({
					2,
					GameData.MapName,
					static_cast<int>(std::round(GameData.Radar.SafetyZonePosition.X)),
					static_cast<int>(std::round(GameData.Radar.SafetyZonePosition.Y)),
					static_cast<int>(std::round(GameData.Radar.SafetyZoneRadius)),
					static_cast<int>(std::round(GameData.Radar.BlueZonePosition.X)),
					static_cast<int>(std::round(GameData.Radar.BlueZonePosition.Y)),
					static_cast<int>(std::round(GameData.Radar.BlueZoneRadius)),
					GameData.FogPlayerCount,
					GameData.NumAliveTeams
				}),
				i,
				v,
				(send_item ? items : "CACHE"),
				(send_item ? packages : "CACHE")
				});

			try
			{
				std::string data_str = j.dump();
				std::string compressed = compressString(data_str);

				if (wsClient && wsClient->IsConnected()) {
					wsClient->SendBinaryMessage(compressed);
				}
			}
			catch (...) {}


			// Sleep(1);
		}
	}
};

std::unique_ptr<WebSocketClient> WebRadar::wsClient;