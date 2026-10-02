#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/Decrypt.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <Hack/GNames.h>
#include <Utils/Timer.h>
#include <Hack/Process.h>
#include <Hack/LineTrace.h>
#include <Common/BoneSmoother.h>
inline std::queue<std::pair<std::string, PlayerRankList>> RankWorkQueue;              // 鎺掕闃熷垪
inline std::mutex RankQueueMutex;                                                     // 闃熷垪浜掓枼閿?
inline std::condition_variable RankQueueCondition;                                    // 闃熷垪鏉′欢鍙橀噺
inline int PlayerRankWorkCount = 0;                                                   // 鎺掕宸ヤ綔璁℃暟

inline uint8_t* xe_decrypt_container = (uint8_t*)VirtualAlloc(nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // 瑙ｅ瘑瀹瑰櫒

inline BoneSmoother g_BoneSmoother;

class Players
{
public:

	static void DecryptedFloat(float* a1, unsigned int a2, char a3) {
		uint32_t v7[16] = { GameData.Offset["DecryptedHealthOffsets0"], GameData.Offset["DecryptedHealthOffsets1"], GameData.Offset["DecryptedHealthOffsets2"], GameData.Offset["DecryptedHealthOffsets3"],
							GameData.Offset["DecryptedHealthOffsets4"], GameData.Offset["DecryptedHealthOffsets5"], GameData.Offset["DecryptedHealthOffsets6"], GameData.Offset["DecryptedHealthOffsets7"],
							GameData.Offset["DecryptedHealthOffsets8"], GameData.Offset["DecryptedHealthOffsets9"], GameData.Offset["DecryptedHealthOffsets10"], GameData.Offset["DecryptedHealthOffsets11"],
							GameData.Offset["DecryptedHealthOffsets12"], GameData.Offset["DecryptedHealthOffsets13"], GameData.Offset["DecryptedHealthOffsets14"], GameData.Offset["DecryptedHealthOffsets15"] };

		auto* ptr = reinterpret_cast<unsigned char*>(a1); // 灏唂loat鎸囬拡杞崲涓哄瓧鑺傛寚閽?
		for (unsigned int i = 0; i < a2; ++i) {
			char v5 = i + a3;
			ptr[i] ^= reinterpret_cast<char*>(v7)[v5 & 0x3F];
		}
	}

	static void LoadLists(int type)
	{
		std::unordered_map <std::string, int> PlayerWhiteLists;
		std::unordered_map <std::string, int> PlayerBlackLists;
		std::string filename = "Config/BlackLists.txt";
		if (type == 2) {
			filename = "Config/WhiteLists.txt";
		}

		std::ifstream file(filename);
		if (!file.is_open()) {
			return;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (type == 2) {
				PlayerWhiteLists[line] = 1;
			}
			else {
				PlayerBlackLists[line] = 1;
			}
		}

		if (type == 2) {
			Data::SetPlayerWhiteLists(PlayerWhiteLists);
		}
		else {
			Data::SetPlayerBlackLists(PlayerBlackLists);
		}

		file.close();
	}

	static void SaveLists(int type) {
		std::unordered_map <std::string, int> PlayerWhiteLists = Data::GetPlayerWhiteLists();
		std::unordered_map <std::string, int> PlayerBlackLists = Data::GetPlayerBlackLists();
		std::string filename;
		std::unordered_map<std::string, int> currentList;

		if (type == 2) {
			filename = "Config/WhiteLists.txt";
			currentList = PlayerWhiteLists;
		}
		else {
			filename = "Config/BlackLists.txt";
			currentList = PlayerBlackLists;
		}

		std::ofstream file(filename);
		if (!file.is_open()) {
			return;
		}

		for (const auto& entry : currentList) {
			file << entry.first << std::endl;
		}

		file.close();
	}

	static void AddToBlackList(const std::string& name) {
		Data::DeletePlayerWhiteListsItem(name);
		SaveLists(2);
		Data::SetPlayerBlackListsItem(name, 1);
		SaveLists(1);
	}

	static void AddToWhiteList(const std::string& name) {
		Data::DeletePlayerBlackListsItem(name);
		SaveLists(1);
		Data::SetPlayerWhiteListsItem(name, 1);
		SaveLists(2);
	}

	static void RemoveList(const std::string& name) {
		Data::DeletePlayerBlackListsItem(name);
		Data::DeletePlayerWhiteListsItem(name);
		SaveLists(1);
		SaveLists(2);
	}

	static void ReadPlayerLists()
	{
		LoadLists(1);
		LoadLists(2);
	}

	static void UpdateFogPlayers()
	{
		while (true)
		{
			if (GameData.Scene != Scene::Gaming)
			{
				Sleep(GameData.ThreadSleep);
				continue;
			}

			std::unordered_map<uint64_t, FogPlayerInfo> FogPlayerInfos;
			TArray<uint64_t> DormantCharacterClientList = mem.Read<TArray<uint64_t>>(GameData.AntiCheatCharacterSyncManager + GameData.Offset["DormantCharacterClientList"]);
			for (auto FogPlayerEntity : DormantCharacterClientList.GetVector())
			{
				FogPlayerInfo FogPlayer;
				FogPlayerInfos[FogPlayerEntity] = FogPlayer;
			}

			Data::SetFogPlayers(std::move(FogPlayerInfos));

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	static void UpdatePlayerLists()
	{
		if (GameData.Scene == Scene::Gaming)
		{
			auto PlayerBlackLists = Data::GetPlayerBlackLists();
			auto PlayerWhiteLists = Data::GetPlayerWhiteLists();
			auto LocalPlayerInfo = GameData.LocalPlayerInfo;
			auto ScatterHandle = mem.CreateScatterHandle();

			std::unordered_map<std::string, GamePlayerInfo> GPlayerLists;
			std::unordered_map<std::string, PlayerRankList> PlayerRankLists = Data::GetPlayerRankLists();
			GameData.PlayerCount = mem.Read<int>(GameData.GameState + GameData.Offset["PlayerArray"] + 0x8);  // 璇诲彇鐜╁鏁伴噺
			GameData.NumAliveTeams = mem.Read<int>(GameData.GameState + GameData.Offset["NumAliveTeams"]);
			if (GameData.PlayerCount <= 0)
			{
				return;
			}
			// 鏍规嵁鐜╁鏁伴噺鍒嗛厤缂撳啿鍖?
			GPlayerLists.reserve(static_cast<size_t>(GameData.PlayerCount));
			std::vector<uint64_t> PlayerArray(static_cast<size_t>(GameData.PlayerCount));
			// 璇诲彇鐜╁鏁扮粍骞跺皢鍏跺瓨鍌ㄥ湪缂撳啿鍖轰腑
			mem.Read(
				mem.Read<uint64_t>(GameData.GameState + GameData.Offset["PlayerArray"]),
				PlayerArray.data(),
				sizeof(uint64_t) * static_cast<size_t>(GameData.PlayerCount)
			);
			std::vector<GamePlayerInfo> playerLists;
			playerLists.reserve(PlayerArray.size());
			// 灏嗙紦鍐插尯鍐呭澶嶅埗鍒颁竴涓?std::vector 涓?

			// 閬嶅巻鐜╁鏁扮粍涓殑姣忎釜鐜╁淇℃伅鎸囬拡
			for (auto& pPlayerInfo : PlayerArray)
			{
				GamePlayerInfo player;             // 鍒涘缓涓€涓?GamePlayerInfo 缁撴瀯浣撳疄渚?
				player.pPlayerInfo = pPlayerInfo;  // 灏嗗綋鍓嶇帺瀹朵俊鎭寚閽堣祴鍊肩粰 player 缁撴瀯浣撲腑鐨?pPlayerInfo 鎴愬憳
				playerLists.push_back(player);     // 灏?player 缁撴瀯浣撴坊鍔犲埌 playerLists 鍚戦噺涓?
			}

			for (GamePlayerInfo& player : playerLists)
			{
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["PlayerName"], (uint64_t*)&player.pPlayerName);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["PlayerStatusType"], (BYTE*)&player.StatusType);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["AccountId"], (uint64_t*)&player.pAccountId);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["TeamNumber"], (int*)&player.TeamID);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["SquadMemberIndex"], (int*)&player.SquadMemberIndex);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["PartnerLevel"], (EPartnerLevel*)&player.PartnerLevel);
				//mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["PubgIdData"], (FWuPubgIdData*)&player.PubgIdData);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["PlayerStatistics"], (int*)&player.KillCount);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["DamageDealtOnEnemy"], (float*)&player.DamageDealtOnEnemy);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["CharacterClanInfo"] + 0x20, (uint64_t*)&player.pClanName);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["SurvivalTier"], (int*)&player.PubgIdData.SurvivalTier);
				mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["SurvivalLevel"], (int*)&player.PubgIdData.SurvivalLevel);
				//mem.AddScatterRead(ScatterHandle, player.pPlayerInfo + GameData.Offset["SpectatedCount"], (int*)&player.SpectatedCount);
			}

			mem.ExecuteReadScatter(ScatterHandle);

			for (GamePlayerInfo& player : playerLists)
			{
				mem.AddScatterRead(ScatterHandle, player.pClanName, (FText*)&player.FClanName);
				mem.AddScatterRead(ScatterHandle, player.pPlayerName, (FText*)&player.FPlayerName);
				mem.AddScatterRead(ScatterHandle, player.pAccountId, (FText*)&player.FAccountId);
			}

			mem.ExecuteReadScatter(ScatterHandle);

			for (GamePlayerInfo& player : playerLists)
			{
				player.ClanName = Utils::UnicodeToAnsi(player.FClanName.buffer);
				player.AccountId = Utils::UnicodeToAnsi(player.FAccountId.buffer);
				player.PlayerName = Utils::UnicodeToAnsi(player.FPlayerName.buffer);
				//player.PubgIdData.SurvivalLevel = (player.PubgIdData.SurvivalTier - 1) * 500 + player.PubgIdData.SurvivalLevel;
				player.TeamID = (player.TeamID >= 100000) ? (player.TeamID - 100000) : player.TeamID;
				player.IsMyTeam = GameData.LocalPlayerInfo.TeamID == player.TeamID;

				player.IsSelf = (player.AccountId == GameData.LocalPlayerInfo.AccountId);

				if (PlayerBlackLists.find(player.AccountId) != PlayerBlackLists.end()) {
					player.ListType = 1;
				}
				else if (PlayerWhiteLists.find(player.AccountId) != PlayerWhiteLists.end()) {
					player.ListType = 2;
				}

				if (player.PubgIdData.SurvivalTier > 0) player.PubgIdData.SurvivalLevel = (player.PubgIdData.SurvivalTier - 1) * 500 + player.PubgIdData.SurvivalLevel;
			}

			// 閬嶅巻 playerLists 鍚戦噺涓殑姣忎釜 GamePlayerInfo 缁撴瀯浣?
			for (GamePlayerInfo& player : playerLists)
			{
				if (player.PlayerName == "") {
					continue;
				}

				// 灏嗗綋鍓嶇帺瀹朵俊鎭瓨鍌ㄥ埌 GPlayerLists 鏄犲皠涓紝浠ョ帺瀹跺悕瀛椾綔涓洪敭
				GPlayerLists[player.PlayerName] = player;

				// 濡傛灉 PlayerRankLists 涓病鏈夊綋鍓嶇帺瀹剁殑鍚嶅瓧锛屽苟涓旂帺瀹剁姸鎬佷负瀛樻椿 (8) 鎴栦汉鏈?(12)
				if (PlayerRankLists.count(player.PlayerName) == 0)
				{
					PlayerRankList playerRankList;
					playerRankList.AccountId = player.AccountId;
					playerRankList.PlayerName = player.PlayerName;
					playerRankList.Tem = player.TeamID;
					playerRankList.DamageAmount = player.DamageDealtOnEnemy;
					playerRankList.Survivallevel = player.PubgIdData.SurvivalLevel;
					Data::SetPlayerRankListsItem(playerRankList.AccountId, playerRankList);
					//PlayerRankList playerRankList;  // 鍒涘缓涓€涓?PlayerRankList 缁撴瀯浣撳疄渚?
					//PlayerRankLists[player.PlayerName] = playerRankList;  // 灏?playerRankList 娣诲姞鍒?PlayerRankLists 鏄犲皠涓?
					//Data::SetPlayerRankListsItem(player.PlayerName, {});  // 璋冪敤鍑芥暟璁剧疆鐜╁鎺掑悕鍒楄〃椤?
				}
			}


			Data::SetPlayerLists(std::move(GPlayerLists));

			mem.CloseScatterHandle(ScatterHandle);
		}
		else {
			Data::SetPlayerLists({});
		}
	}

	static void Update()
	{
		Throttler Throttlered;
		Throttler ThrottleredSleep;
		auto hScatter = mem.CreateScatterHandle();
		std::unordered_map<uint64_t, tMapInfo> EnemyInfoMap;
		float TimeSeconds = 0.f;
		while (true)
		{
			if (GameData.Scene != Scene::Gaming)
			{
				TimeSeconds = 0.f;
				GameData.FogPlayerCount = 0;
				EnemyInfoMap.clear();
				g_BoneSmoother.ClearAllCache();
				GameData.PlayerSegmentLists.clear();
				GameData.PlayerRankLists.clear();
				Data::SetPlayerLists({});
				GameData.LocalPlayerInfo = Player();
				Sleep(GameData.ThreadSleep);
				continue;
			}

			ThrottleredSleep.executeTaskWithSleep("PlayersUpdateSleep", std::chrono::milliseconds(4), [] {});

			int FogPlayerCount = 0;

			std::unordered_map<uint64_t, Player> CachePlayers = Data::GetCachePlayers();
			std::unordered_map<uint64_t, Player> PlayersData = Data::GetPlayersData();
			std::unordered_map<uint64_t, FogPlayerInfo> FogPlayers = Data::GetFogPlayers();
			for (auto it = EnemyInfoMap.begin(); it != EnemyInfoMap.end();)
			{
				if (CachePlayers.find(it->first) == CachePlayers.end())
				{
					it = EnemyInfoMap.erase(it);
				}
				else {
					++it;
				}
			}

			Throttlered.executeTask("UpdatePlayersData", std::chrono::milliseconds(5), [&hScatter, &CachePlayers, &PlayersData] {
				for (auto& Item : CachePlayers)
				{
					Player& Player = Item.second;

					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["RootComponent"], (uint64_t*)&Player.RootComponent);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["CharacterMovement"], (uint64_t*)&Player.CharacterMovement);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["Mesh"], (uint64_t*)&Player.MeshComponent);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["PlayerState"], (uint64_t*)&Player.PlayerState);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["GroggyHealth"], (float*)&Player.GroggyHealth);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["SpectatedCount"], (int*)&Player.SpectatedCount);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["LastTeamNum"], (int*)&Player.TeamID);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["CharacterName"], (uint64_t*)&Player.pCharacterName);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["CharacterState"], (ECharacterState*)&Player.CharacterState);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["AimOffsets"], (FRotator*)&Player.AimOffsets);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["WeaponProcessor"], (uint64_t*)&Player.WeaponProcessor);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["Gender"], (EGender*)&Player.Gender);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["bEncryptedHealth"], (BYTE*)&Player.bEncryptedHealth);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["EncryptedHealthOffset"], (unsigned char*)&Player.EncryptedHealthOffset);
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["DecryptedHealthOffset"], (unsigned char*)&Player.DecryptedHealthOffset);
				}
				mem.ExecuteReadScatter(hScatter);

				for (auto& Item : CachePlayers)
				{
					Player& Player = Item.second;


					Player.RootComponent = Decrypt::Xe(Player.RootComponent);
					Player.CharacterMovement = Decrypt::Xe(Player.CharacterMovement);

					Player.PlayerState = Decrypt::Xe(Player.PlayerState);


					mem.AddScatterRead(hScatter, Player.pCharacterName, (FText*)&Player.CharacterName);
					mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["AnimScriptInstance"], (uint64_t*)&Player.AnimScriptInstance);
					mem.AddScatterRead(hScatter, Player.RootComponent + GameData.Offset["ComponentLocation"], (FVector*)&Player.Location);
				}
				mem.ExecuteReadScatter(hScatter);

				auto GamePlayerLists = Data::GetPlayerLists();

				// 寰幆鏇存柊缂撳瓨涓殑鐜╁淇℃伅
				for (auto& Item : CachePlayers)
				{
					// 鑾峰彇鐜╁瀵硅薄鐨勫紩鐢?
					Player& Player = Item.second;

					// 鍒ゆ柇鐜╁鏄惁涓鸿嚜宸?
					Player.IsMe = Player.Entity == GameData.CameraViewTarget ||
						Player.Entity == GameData.AcknowledgedPawn;

					// 璋冩暣鍥㈤槦ID
					Player.TeamID = (Player.TeamID >= 100000) ? (Player.TeamID - 100000) : Player.TeamID;

					// 妫€鏌ョ帺瀹舵槸鍚︿负鏈槦鎴愬憳
					if (GameData.Config.ESP.duiyou)
						Player.IsMyTeam = false;
					else
						Player.IsMyTeam = Player.TeamID == GameData.LocalPlayerTeamID;

					// 灏嗙帺瀹跺悕绉拌浆鎹负ANSI鏍煎紡锛屽苟鍘婚櫎澶氫綑瀛楃
					Player.Name = Utils::RemoveBracketsAndTrim(Utils::UnicodeToAnsi(Player.CharacterName.buffer));


					// 浠嶨amePlayerLists涓幏鍙栫帺瀹惰缁嗕俊鎭?
					GamePlayerInfo PlayerInfo = GamePlayerLists[Player.Name];

					// 濉厖鐜╁瀵硅薄鐨勮缁嗕俊鎭?
					Player.ClanName = PlayerInfo.ClanName;
					//Player.SpectatedCount = PlayerInfo.SpectatedCount;
					Player.SurvivalLevel = PlayerInfo.PubgIdData.SurvivalLevel;
					Player.PartnerLevel = PlayerInfo.PartnerLevel;
					Player.DamageDealtOnEnemy = PlayerInfo.DamageDealtOnEnemy;
					Player.Alignment = PlayerInfo.Alignment;
					Player.KillCount = PlayerInfo.KillCount;
					Player.ListType = PlayerInfo.ListType;
					Player.SquadMemberIndex = PlayerInfo.SquadMemberIndex;
					Player.AccountId = PlayerInfo.AccountId;

					// 灏嗙帺瀹朵綅缃浆鎹负灞忓箷鍧愭爣
					FVector2D WorldToScreen = VectorHelper::WorldToScreen(Player.Location);

					// 濡傛灉鐜╁涓嶆槸鑷繁涓斾笉鍦ㄥ睆骞曞唴锛屽垯鏍囪涓轰笉鍦ㄥ睆骞曞唴骞惰烦杩囧悗缁鐞?
					if (!Player.IsMe && (WorldToScreen.X < -100 || WorldToScreen.X > GameData.Config.Overlay.ScreenWidth + 100 || WorldToScreen.Y < -100 || WorldToScreen.Y > GameData.Config.Overlay.ScreenHeight + 100))
					{
						Player.InScreen = false;
						continue;
					}

					// 娣诲姞瀵瑰姩鐢荤姸鎬佸拰姝﹀櫒淇℃伅鐨勮鍙?
					mem.AddScatterRead(hScatter, Player.AnimScriptInstance + GameData.Offset["PreEvalPawnState"], (EAnimPawnState*)&Player.PreEvalPawnState);
					mem.AddScatterRead(hScatter, Player.AnimScriptInstance + GameData.Offset["bIsReloading_CP"], (bool*)&Player.IsReloading);
					mem.AddScatterRead(hScatter, Player.WeaponProcessor + GameData.Offset["EquippedWeapons"], (uint64_t*)&Player.EquippedWeapons);
					mem.AddScatterRead(hScatter, Player.WeaponProcessor + GameData.Offset["CurrentWeaponIndex"], (BYTE*)&Player.CurrentWeaponIndex);
				}
				mem.ExecuteReadScatter(hScatter);

				// 璇诲彇鐜╁褰撳墠姝﹀櫒鐨勪俊鎭?
				for (auto& Item : CachePlayers)
				{
					Player& Player = Item.second;
					if (!Player.InScreen)
					{
						continue;
					}
					// 妫€鏌ュ綋鍓嶆鍣ㄧ储寮曟槸鍚﹀湪鏈夋晥鑼冨洿鍐?
					if (Player.CurrentWeaponIndex >= 0 && Player.CurrentWeaponIndex < 8)
					{
						mem.AddScatterRead(hScatter, Player.EquippedWeapons + Player.CurrentWeaponIndex * 8, (uint64_t*)&Player.CurrentWeapon);
					}
				}
				mem.ExecuteReadScatter(hScatter);

				// 璇诲彇鐜╁鏄惁鍦ㄧ瀯鍑嗙姸鎬?
				for (auto& Item : CachePlayers)
				{
					Player& Player = Item.second;
					if (!Player.InScreen)
					{
						continue;
					}
					// 妫€鏌ュ綋鍓嶆鍣ㄦ槸鍚︽湁鏁?
					if (Player.CurrentWeapon > 0)
					{
						Player.AmmoCount = 0;
						// 璇诲彇鐜╁褰撳墠姝﹀櫒鐨処D
						mem.AddScatterRead(hScatter, Player.CurrentWeapon + GameData.Offset["ObjID"], (int*)&Player.WeaponID);
						mem.AddScatterRead(hScatter, Player.CurrentWeapon + GameData.Offset["CurrentAmmoData"], (short*)&Player.AmmoCount);

						// 璇诲彇鐜╁褰撳墠姝﹀櫒鐨勭被鍒?
						//mem.AddScatterRead(hScatter, Player.CurrentWeapon + GameData.Offset["WeaponConfig_WeaponClass"], (EWeaponClass*)&Player.WeaponClass);

						mem.AddScatterRead(hScatter, Player.CurrentWeapon + GameData.Offset["WeaponConfig_WeaponClass"], (BYTE*)&Player.WeaponClassByte);

						mem.AddScatterRead(hScatter, Player.CurrentWeapon + GameData.Offset["ElapsedCookingTime"], (float*)&Player.ElapsedCookingTime);
						if (Player.IsMe)
						{
							mem.AddScatterRead(hScatter, Player.AnimScriptInstance + GameData.Offset["bIsScoping_CP"], (bool*)&Player.IsScoping);
						}
					}
				}

				mem.ExecuteReadScatter(hScatter);

				// 鑾峰彇姝﹀櫒鐨勬樉绀哄悕绉?
				for (auto& Item : CachePlayers)
				{
					// 鑾峰彇鐜╁瀵硅薄鐨勫紩鐢?
					Player& Player = Item.second;
					Player.AmmoCount = (short)Player.AmmoCount;

					// 瑙ｅ瘑鐜╁姝﹀櫒鐨処D
					Player.WeaponID = Decrypt::CIndex(Player.WeaponID);

					// 鏍规嵁瑙ｅ瘑鍚庣殑姝﹀櫒ID鑾峰彇姝﹀櫒瀹炰綋淇℃伅
					Player.WeaponEntityInfo = Data::GetGNameListsByIDItem(Player.WeaponID);

					// 鑾峰彇姝﹀櫒鐨勬樉绀哄悕绉?
					Player.WeaponName = Player.WeaponEntityInfo.DisplayName;

					// 灏嗘洿鏂板悗鐨勭帺瀹朵俊鎭繚瀛樺埌 PlayersData 瀹瑰櫒涓?
					PlayersData[Player.Entity] = Player;
				}

				});

			for (auto& Item : CachePlayers)
			{
				Player& Player = Item.second;

				// 妫€鏌?PlayersData 瀹瑰櫒涓槸鍚﹀寘鍚綋鍓嶇帺瀹剁殑瀹炰綋
				if (PlayersData.count(Player.Entity) > 0)
				{
					// 濡傛灉鍖呭惈锛屽垯浠?PlayersData 瀹瑰櫒涓幏鍙栬鐜╁鐨勪俊鎭?
					Player = PlayersData[Player.Entity];

					// 鏍囪鐜╁鍦ㄥ睆骞曞唴
					Player.InScreen = true;
				}
			}

			for (auto& Item : CachePlayers)
			{
				Player& Player = Item.second;


				if (Player.bEncryptedHealth == 0)
				{
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["Health"], (float*)&Player.Health);
				}
				else {
					mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["Health"] + Player.EncryptedHealthOffset, (float*)&Player.Health);
				}

				// 璇诲彇鐜╁鐨勫叾浠栧睘鎬?
				mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["Eyes"], (int*)&Player.Eyes);
				//mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["StaticMesh"], (uint64_t*)&Player.StaticMesh);
				mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["StaticMesh"] + 0x10, (uint64_t*)&Player.StaticMesh);
				mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["bAlwaysCreatePhysicsState"], (UCHAR*)&Player.bAlwaysCreatePhysicsState);
				mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["ComponentToWorld"], (FTransform*)&Player.ComponentToWorld);
				mem.AddScatterRead(hScatter, Player.RootComponent + GameData.Offset["ComponentLocation"], (FVector*)&Player.Location);

			}
			mem.ExecuteReadScatter(hScatter);

			for (auto& Item : CachePlayers)
			{
				Player& Player = Item.second;

				if (Player.bEncryptedHealth != 0) {
					DecryptedFloat(&Player.Health, sizeof(Player.Health), Player.DecryptedHealthOffset);
				}

				// 灏嗙帺瀹剁殑涓栫晫鍧愭爣杞崲涓哄睆骞曞潗鏍?
				FVector2D WorldToScreen = VectorHelper::WorldToScreen(Player.Location);


				// 妫€鏌ョ帺瀹舵槸鍚﹀湪闆句腑
				if (Player.bAlwaysCreatePhysicsState == 4)
				{
					Player.InFog = true;
					Player.InScreen = false;
					FogPlayerCount++;
				}
				else {
					// 妫€鏌ョ帺瀹舵槸鍚﹀湪灞忓箷鑼冨洿鍐?
					if (!Player.IsMe && (WorldToScreen.X < -100 || WorldToScreen.X > GameData.Config.Overlay.ScreenWidth + 100 || WorldToScreen.Y < -100 || WorldToScreen.Y > GameData.Config.Overlay.ScreenHeight + 100))
					{
						Player.InScreen = false;
						continue;
					}
				}

				// 濡傛灉鐜╁涓嶅湪灞忓箷涓婃垨鑰呮槸鑷繁鎴栬€呮槸鑷繁鐨勯槦鍙嬶紝璺宠繃璇ョ帺瀹?
				if (!Player.InScreen || Player.IsMe || Player.IsMyTeam)
				{
					continue;
				}

				// 閬嶅巻姣忎釜楠ㄩ
				for (EBoneIndex Bone : SkeletonLists::Bones)
				{
					int BoneIndex = Bone;

					if (Player.Gender != EGender::Male && BoneIndex >= 82)
					{
						BoneIndex += 7;
					}

					mem.AddScatterRead(hScatter, Player.StaticMesh + (static_cast<unsigned long long>(BoneIndex) * 0x30), (FTransform*)&Player.Skeleton.Bones[Bone]);
				}
			}
			mem.ExecuteReadScatter(hScatter);

			if (!GameData.AimBot.Lock)
			{
				GameData.AimBot.Type = EntityType::Player;
				GameData.AimBot.Target = 0;
				GameData.AimBot.Bone = 0;
				GameData.AimBot.ScreenDistance = 1000.0f;
			}

			AimBotConfig Config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];
			if (Config.DynamicFov) {
				Config.FOV = Config.FOV * (90.0f / GameData.Camera.FOV);
				Config.WheelFOV = Config.FOV * (90.0f / GameData.Camera.FOV);
			}

			bool IsWheelKeyDown = GameData.Keyboard.IsKeyDown(Config.Wheel.Key);

			// 閬嶅巻缂撳瓨鐨勭帺瀹舵暟鎹?
			for (auto& Item : CachePlayers)
			{
				Player& Player = Item.second; // 鑾峰彇鐜╁瀵硅薄

				// 璁＄畻鐜╁鏄惁鍙
				Player.IsVisible = Player.Eyes + 0.05 >= GameData.LocalPlayerInfo.Eyes;

				// 璁＄畻鐜╁涓庢湰鍦扮帺瀹剁殑璺濈
				Player.Distance = GameData.Camera.Location.Distance(Player.Location) / 100.0f;

				// 鍒濆鍖栫帺瀹剁殑鐬勫噯鐘舵€佸拰鐢熷瓨鐘舵€?
				Player.IsAimMe = false;
				//Player.State =
				//	Player.Health > 0.0f ? CharacterState::Alive :
				//	Player.GroggyHealth > 0.0f ? CharacterState::Groggy :
				//	CharacterState::Dead;
				if (Player.GroggyHealth == 0.0)
				{
					Player.State = CharacterState::Dead;
				}
				else if (Player.GroggyHealth > 0.0f && Player.GroggyHealth < 99.0f)
				{
					Player.State = CharacterState::Groggy;
				}
				else if (Player.GroggyHealth >= 99.0f)
				{
					Player.State = CharacterState::Alive;
				}

				// 鏍规嵁涓嶅悓鐨勭帺瀹剁姸鎬佽缃浄杈惧浘鏍?
				if (Player.CharacterState == ECharacterState::Offline)
				{
					Player.RadarState = ECharacterIconType::Quitter; // 鎺夌嚎
				}
				else if (Player.State == CharacterState::Groggy)
				{
					Player.RadarState = ECharacterIconType::Groggy; // 鍊掑湴
				}
				else if (Player.State == CharacterState::Alive)
				{
					Player.RadarState = ECharacterIconType::Normal;
				}


				// 鏇存柊鐜╁浣嶇疆
				Player.Location = Player.ComponentToWorld.Translation;
				Player.LastUpdateTime = GameData.WorldTimeSeconds;

				// 璁＄畻鏃堕棿宸拰鐜╁浣嶇疆淇℃伅
				float TimeStampDelta = GameData.WorldTimeSeconds - EnemyInfoMap[Player.Entity].TimeStamp;
				EnemyInfoMap[Player.Entity].TimeStamp = GameData.WorldTimeSeconds;

				// 鏇存柊鐜╁浣嶇疆淇℃伅
				[&] {
					auto& PosInfo = EnemyInfoMap[Player.Entity].PosInfo.Info;

					if (Player.State == CharacterState::Dead) {
						PosInfo.clear(); // 鐜╁姝讳骸鏃舵竻绌轰綅缃俊鎭?
					}
					else {
						// 鏇存柊浣嶇疆淇℃伅闃熷垪
						if (TimeStampDelta)
							PosInfo.push_front({ GameData.WorldTimeSeconds, Player.Location });

						if (PosInfo.size() > 200)
							PosInfo.pop_back(); // 鎺у埗浣嶇疆淇℃伅闃熷垪鐨勬渶澶ч暱搴?

						// 璁＄畻鐜╁鐨勯€熷害
						float SumTimeDelta = 0.0f;
						FVector SumPosDif;

						for (size_t i = 1; i < PosInfo.size(); i++) {
							const float DeltaTime = PosInfo[i - 1].Time - PosInfo[i].Time;
							const FVector DeltaPos = PosInfo[i - 1].Pos - PosInfo[i].Pos;
							const FVector DeltaVelocity = DeltaPos * (1.0f / DeltaTime);
							const float DeltaSpeedPerHour = DeltaVelocity.Length() / 100.0f * 3.6f;

							if (DeltaTime > 0.05f || DeltaSpeedPerHour > 500.0f) {
								PosInfo.clear(); // 杩囧ぇ鐨勬椂闂村樊鎴栭€熷害锛屾竻绌轰綅缃俊鎭槦鍒?
							}
							else {
								SumTimeDelta = SumTimeDelta + DeltaTime;
								SumPosDif = SumPosDif + DeltaPos;

								if (SumTimeDelta > 0.15f)
									break;
							}
						}
						if (SumTimeDelta > 0.1f) {
							Player.Velocity = SumPosDif * (1.0f / SumTimeDelta); // 璁＄畻骞冲潎閫熷害
						}
					}
					}();

				PlayersData[Player.Entity] = Player; // 鏇存柊鐜╁鏁版嵁

				if (Player.IsMe)
				{
					GameData.LocalPlayerInfo = Player; // 鏇存柊鏈湴鐜╁淇℃伅
					GameData.LocalPlayerTeamID = Player.TeamID; // 鏇存柊鏈湴鐜╁闃熶紞ID);
					//printf("[LocalPlayer] Position: X=%.2f, Y=%.2f, Z=%.2f\n",
					//	Player.Location.X+GameData.Radar.WorldOriginLocation.X, Player.Location.Y+ GameData.Radar.WorldOriginLocation.Y, Player.Location.Z+ GameData.Radar.WorldOriginLocation.Z);
				}

				if (!Player.InScreen || Player.IsMe || Player.IsMyTeam)
				{
					continue; // 濡傛灉鐜╁涓嶅湪灞忓箷鍐呮垨鏄湰鍦扮帺瀹舵垨鏄湰闃熺帺瀹讹紝缁х画涓嬩竴娆″惊鐜?
				}

				// 璁＄畻鐜╁涓庢湰鍦扮帺瀹剁殑瑙掑害宸?
				FVector AimFov = VectorHelper::CalculateAngles(Player.Location, GameData.LocalPlayerInfo.Location);
				FRotator AmiMz = Player.AimOffsets;
				AmiMz.Clamp();
				int32_t AimX = int32_t(abs(AimFov.X - AmiMz.Yaw));
				int32_t AimY = int32_t(abs(AimFov.Y - AmiMz.Pitch));

				// 鍒ゆ柇鐜╁鏄惁鍦ㄧ瀯鍑嗚寖鍥村唴
				if (AimX <= 2 && AimY <= 2)
				{
					Player.IsAimMe = true;
				}

				EAnimPawnState PreEvalPawnState;
				if (Player.State == CharacterState::Alive || Player.State == CharacterState::Groggy)
				{
					float smoothingAlpha = 0.36f;
					if (Player.Distance > 180.0f) smoothingAlpha = 0.22f;
					else if (Player.Distance > 100.0f) smoothingAlpha = 0.26f;
					else if (Player.Distance > 45.0f) smoothingAlpha = 0.30f;

					bool likelyInVehicle = BoneSmoother::IsPlayerLikelyInVehicle(Player);
					g_BoneSmoother.SetMaxHistorySize(4);
					g_BoneSmoother.SetMaxSpeedThreshold(likelyInVehicle ? 5000.0f : 2500.0f);

					if (Player.Distance > 200.0f || Player.Velocity.Length() > 500.0f || likelyInVehicle)
					{
						smoothingAlpha = likelyInVehicle ? 0.20f : 0.24f;

						if (likelyInVehicle)
						{
							g_BoneSmoother.EnableVehicleMode(true);
						}
					}

					g_BoneSmoother.SetSmoothingFactor(smoothingAlpha);

					for (EBoneIndex Bone : SkeletonLists::Bones)
					{
						FVector rawBoneLocation = VectorHelper::GetBoneWithRotation(Player.Skeleton.Bones[Bone], Player.ComponentToWorld);
						FVector smoothBoneLocation = g_BoneSmoother.SmoothWorldPosition(Player.Entity, Bone, rawBoneLocation, GameData.WorldTimeSeconds);
						FVector2D rawBoneScreen = VectorHelper::WorldToScreen(rawBoneLocation);
						FVector2D smoothBoneScreen = VectorHelper::WorldToScreen(smoothBoneLocation);
						Player.Skeleton.RawLocationBones[Bone] = rawBoneLocation;
						Player.Skeleton.RawScreenBones[Bone] = rawBoneScreen;
						Player.Skeleton.LocationBones[Bone] = smoothBoneLocation;
						Player.Skeleton.ScreenBones[Bone] = smoothBoneScreen;
					}

					if (likelyInVehicle)
					{
						g_BoneSmoother.EnableVehicleMode(false);
					}
				}
				else
				{
					g_BoneSmoother.ClearEntityCache(Player.Entity);
				}

				if (GameData.LocalPlayerInfo.CurrentWeaponIndex != 255 && GameData.LocalPlayerInfo.CurrentWeaponIndex != 4)
				{
					if (Player.State == CharacterState::Alive || Player.State == CharacterState::Groggy)
					{
						for (EBoneIndex Bone : SkeletonLists::Bones)
						{
							Player.Skeleton.VisibleBones[Bone] = LineTrace::LineTraceSingle(GameData.Camera.Location, Player.Skeleton.RawLocationBones[Bone]);
						}
					}

					Player.IsVisible =
						Player.Skeleton.VisibleBones[EBoneIndex::ForeHead] ||
						Player.Skeleton.VisibleBones[EBoneIndex::Head] ||
						Player.Skeleton.VisibleBones[EBoneIndex::Neck_01] ||
						Player.Skeleton.VisibleBones[EBoneIndex::Spine_03];
				}

				// 鑷瀯绛涢€?
				if ((!GameData.AimBot.Lock || Utils::ValidPtr(GameData.AimBot.Target)) && !IsWheelKeyDown)
				{
					bool AllowAim = Player.State != CharacterState::Dead;
					auto Bones = Config.First.Bones;

					bool firstKeyCondition = GameData.Keyboard.IsKeyDown(Config.First.Key);
					bool secondKeyCondition = GameData.Keyboard.IsKeyDown(Config.Second.Key);

					// 鏍规嵁鐬勫噯蹇嵎閿拰鍚堝苟瑙勫垯璁剧疆鐬勫噯楠ㄩ
					if (firstKeyCondition && Config.HotkeyMerge && secondKeyCondition)
					{
						Bones = Config.Second.Bones;
					}
					else if (!Config.HotkeyMerge && secondKeyCondition)
					{
						Bones = Config.Second.Bones;
					}

					if (GameData.Keyboard.IsKeyDown(Config.Groggy.Key))
					{
						Bones = Config.Groggy.Bones;
						AllowAim = Player.State == CharacterState::Groggy;
					}

					if (AllowAim && !Player.IsMyTeam && !Player.IsMe)
					{
						for (int i = 0; i < 17; i++)
						{
							if (Bones[i])
							{
								FVector2D ScreenLocation = Player.Skeleton.RawScreenBones[BoneIndex[i]];
								float Distance = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, ScreenLocation.X, ScreenLocation.Y);

								//閿佸畾鏂瑰紡 true璺濈  false鑼冨洿
								if (GameData.Config.AimBot.LockMode)
								{
									Distance = Player.Distance;
									if (Distance < GameData.AimBot.ScreenDistance) {
										GameData.AimBot.ScreenDistance = Distance;
										GameData.AimBot.Target = Player.Entity;
										GameData.AimBot.Bone = BoneIndex[i];
										GameData.AimBot.Type = EntityType::Player;
									}
								}
								else {
									if (Distance <= Config.FOV) {
										if (Distance < GameData.AimBot.ScreenDistance) {
											GameData.AimBot.ScreenDistance = Distance;
											GameData.AimBot.Target = Player.Entity;
											GameData.AimBot.Bone = BoneIndex[i];
											GameData.AimBot.Type = EntityType::Player;
										}
									}
								}

							}
						}
					}
				}

				if (GameData.AimBot.Target == Player.Entity) {
					GameData.AimBot.TargetPlayerInfo = Player;
				}
			}

			if ((!GameData.AimBot.Lock || Utils::ValidPtr(GameData.AimBot.Target)) && IsWheelKeyDown && Config.AimWheel)
			{
				std::unordered_map<uint64_t, VehicleWheelInfo> VehicleWheels = Data::GetVehicleWheels();

				for (auto& Item : VehicleWheels)
				{
					auto& Wheel = Item.second;

					bool AllowAim = Wheel.State == WheelState::Normal;

					Wheel.ScreenLocation = VectorHelper::WorldToScreen(Wheel.Location);
					float Distance = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, Wheel.ScreenLocation.X, Wheel.ScreenLocation.Y);
					if (Distance <= Config.WheelFOV) {
						if (Distance < GameData.AimBot.ScreenDistance) {
							GameData.AimBot.ScreenDistance = Distance;
							GameData.AimBot.Target = Wheel.Wheel;
							GameData.AimBot.Bone = 0;
							GameData.AimBot.Type = EntityType::Wheel;
						}
					}
				}
			}

			bool IsGrenadeKeyDown2 = GameData.Keyboard.IsKeyDown(GameData.Config.AimBot.Grenade);
			bool IsMortarKeyDown2 = GameData.Keyboard.IsKeyDown(GameData.Config.AimBot.Mortar);
			bool allowProjectAim = IsGrenadeKeyDown2 || IsMortarKeyDown2;
			if (!allowProjectAim && GameData.Config.AimBot.ProjectAutoLock)
			{
				if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade && GameData.Config.AimBot.GrenadePredict)
					allowProjectAim = true;
				if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1 && GameData.Config.AimBot.PanzerFaust)
					allowProjectAim = true;
			}
			if ((!GameData.AimBot.Lock || Utils::ValidPtr(GameData.AimBot.Target)) && allowProjectAim)
			{
				auto Projects = Data::GetProjects();
				for (auto& kv : Projects)
				{
					auto& Proj = kv.second;
					bool wanted = false;
					if ((IsGrenadeKeyDown2 || (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade && GameData.Config.AimBot.GrenadePredict)) &&
						(Proj.EntityName == "ProjMolotov_C" || Proj.EntityName.find("Molotov") != std::string::npos)) wanted = true;
					if (IsMortarKeyDown2 || (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1 && GameData.Config.AimBot.PanzerFaust))
					{
						if (Proj.EntityName.find("Panzer") != std::string::npos || Proj.EntityName.find("Faust") != std::string::npos || Proj.EntityName.find("Rocket") != std::string::npos)
							wanted = true;
					}
					if (!wanted) continue;
					FVector2D scr = VectorHelper::WorldToScreen(Proj.Location);
					float d = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, scr.X, scr.Y);
					if (d <= Config.FOV)
					{
						if (d < GameData.AimBot.ScreenDistance)
						{
							GameData.AimBot.ScreenDistance = d;
							GameData.AimBot.Target = Proj.Entity;
							GameData.AimBot.Bone = 0;
							GameData.AimBot.Type = EntityType::Project;
						}
					}
				}
			}

			Data::SetPlayers(std::move(CachePlayers));
			Data::SetPlayersData(std::move(PlayersData));
			GameData.FogPlayerCount = FogPlayerCount;

		}
		mem.CloseScatterHandle(hScatter);
	}
};
