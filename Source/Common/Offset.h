#pragma once
#include <cstdint>
#include "common/Data.h"

// ============================================================
//  Offsets — 本地下发，不依赖云拉取
//  当前北京时间：2026年6月19日
//  版本号：2605.x.x.x（已更新）
//
//  注：Decrypt::CIndex / Decrypt::Xe / Decrypt::DestroyXe 定义在 Hack/Decrypt.h
// ============================================================
namespace Offset
{

	constexpr uint64_t XenuineDecrypt = 0x10565A28;
	constexpr uint64_t UWorld = 0x11DA1828;                     // 更新
	constexpr uint64_t GNames = 0x1202A4B0;                     // 更新
	constexpr uint64_t GNamesPtr = 0x0;
	constexpr uint64_t ChunkSize = 0x3E74;
	constexpr uint64_t GObjects = 0x11DBACE0;
	constexpr uint64_t CurrentLevel = 0x850;                    // 更新
	constexpr uint64_t Actors = 0x188;                          // 更新
	constexpr uint64_t ActorsForGC = 0x358;
	constexpr uint64_t GameInstance = 0x888;                    // 更新
	constexpr uint64_t GameState = 0x3E0;
	constexpr uint64_t LocalPlayer = 0x70;
	constexpr uint64_t PlayerController = 0x38;                // 更新
	constexpr uint64_t AcknowledgedPawn = 0x4B0;                // 更新
	constexpr uint64_t PlayerCameraManager = 0x4D8;             // 更新

	constexpr uint64_t ObjID = 0xC;
	constexpr uint64_t DecryptNameIndexRor = 0x1;
	constexpr uint64_t DecryptNameIndexXorKey1 = 0xF1A6D750;
	constexpr uint64_t DecryptNameIndexXorKey2 = 0x48925E3D;
	constexpr uint64_t DecryptNameIndexXorKey3 = 0xFFFF0000;
	constexpr uint64_t DecryptNameIndexRval = 0xC;
	constexpr uint64_t DecryptNameIndexSval = 0x4;
	constexpr uint64_t DecryptNameIndexDval = 0x0;

	constexpr uint64_t ViewTarget = 0x1070;
	constexpr uint64_t CameraCacheLocation = 0x1090;
	constexpr uint64_t CameraCacheRotation = 0x162C;
	constexpr uint64_t CameraCacheFOV = 0x1644;

	constexpr uint64_t LastTeamNum = 0x1190;                   // 更新
	constexpr uint64_t TeamNumber = 0x9A8;

	constexpr uint64_t MyHUD = 0x4E8;
	constexpr uint64_t BlockInputWidgetList = 0x5D0;
	constexpr uint64_t bShowMouseCursor = 0x678;
	constexpr uint64_t ComponentLocation = 0x300;              // 更新
	constexpr uint64_t ComponentToWorld = 0x2F0;               // 更新
	constexpr uint64_t CharacterState = 0x1698;
	constexpr uint64_t CharacterName = 0x2B10;                 // 更新
	constexpr uint64_t CharacterMovement = 0x4A0;
	constexpr uint64_t WorldToMap = 0x844;                     // 更新
	constexpr uint64_t LayoutData = 0x40;
	constexpr uint64_t Offsets = 0x0;
	constexpr uint64_t Alignment = 0x20;
	constexpr uint64_t Visibility = 0xA9;
	constexpr uint64_t SelectMinimapSizeIndex = 0x5C8;
	constexpr uint64_t Slot = 0x38;
	constexpr uint64_t WidgetStateMap = 0x558;

	constexpr uint64_t FeatureRepObject = 0xCF0;
	constexpr uint64_t SafetyZonePosition = 0xB0;
	constexpr uint64_t SafetyZoneRadius = 0xBC;
	constexpr uint64_t BlueZoneRadius = 0xCC;
	constexpr uint64_t BlueZonePosition = 0xC0;
	constexpr uint64_t NumAliveTeams = 0x4B8;

	constexpr uint64_t HeaFlag = 0x208;
	constexpr uint64_t Health1 = 0xA38;
	constexpr uint64_t Health2 = 0xA3C;
	constexpr uint64_t Health3 = 0xA64;
	constexpr uint64_t Health4 = 0xA50;
	constexpr uint64_t Health5 = 0xA65;
	constexpr uint64_t Health6 = 0xA60;
	constexpr uint64_t Health_keys0 = 0xCEC7A590;
	constexpr uint64_t Health_keys1 = 0x9B63B26A;
	constexpr uint64_t Health_keys2 = 0xCA150EA5;
	constexpr uint64_t Health_keys3 = 0x6A38488D;
	constexpr uint64_t Health_keys4 = 0xA911D0A;
	constexpr uint64_t Health_keys5 = 0x23DDA248;
	constexpr uint64_t Health_keys6 = 0x9458DC8;
	constexpr uint64_t Health_keys7 = 0xA521B921;
	constexpr uint64_t Health_keys8 = 0xBA27A58;
	constexpr uint64_t Health_keys9 = 0xB0EF6A87;
	constexpr uint64_t Health_keys10 = 0xE2757FB9;
	constexpr uint64_t Health_keys11 = 0x878ADB16;
	constexpr uint64_t Health_keys12 = 0xBD0AAFD5;
	constexpr uint64_t Health_keys13 = 0x6A938D07;
	constexpr uint64_t Health_keys14 = 0x8D099E38;
	constexpr uint64_t Health_keys15 = 0xEDD82AB0;
	constexpr uint64_t GroggyHealth = 0x1118;                  // 更新
	constexpr uint64_t BlueBlockerGaugeTotalMax = 0x10;

	constexpr uint64_t PlayerArray = 0x430;
	constexpr uint64_t AccountId = 0x578;
	constexpr uint64_t PlayerName = 0x430;
	constexpr uint64_t PlayerStatusType = 0x440;
	constexpr uint64_t SquadMemberIndex = 0x5B4;
	constexpr uint64_t PlayerState = 0x448;                    // 更新
	constexpr uint64_t PlayerStatistics = 0xA2C;
	constexpr uint64_t DamageDealtOnEnemy = 0x80C;
	constexpr uint64_t SpectatedCount = 0x2730;                // 更新
	constexpr uint64_t ping = 0x448;
	constexpr uint64_t MatchId = 0x488;
	constexpr uint64_t CapsuleComponent = 0x6E8;
	constexpr uint64_t CustomTimeDilation = 0x198;

	constexpr uint64_t PartnerLevel = 0x6C6;                   // 更新
	constexpr uint64_t SurvivalTier = 0xCE8;
	constexpr uint64_t SurvivalLevel = 0xCEC;
	constexpr uint64_t PubgIdData = 0xCE0;

	constexpr uint64_t CharacterClanInfo = 0x9B0;

	constexpr uint64_t EquippedWeapons = 0x208;                // 更新
	constexpr uint64_t WeaponProcessor = 0x9E8;                // 更新
	constexpr uint64_t CurrentWeaponIndex = 0x319;             // 更新
	constexpr uint64_t WeaponTrajectoryData = 0x11D8;          // 更新
	constexpr uint64_t TrajectoryGravityZ = 0x3BC;
	constexpr uint64_t FiringAttachPoint = 0x8E0;
	constexpr uint64_t ScopingAttachPoint = 0xB50;
	constexpr uint64_t TrajectoryConfig = 0x108;               // 更新
	constexpr uint64_t BallisticCurve = 0x28;
	constexpr uint64_t FloatCurves = 0x38;
	constexpr uint64_t Mesh3P = 0x820;

	constexpr uint64_t Keys = 0x60;

	constexpr uint64_t AttachedStaticComponentMap = 0x1508;

	constexpr uint64_t WeaponConfig_WeaponClass = 0x72D;
	constexpr uint64_t ElapsedCookingTime = 0xB30;

	constexpr uint64_t PlayerInput = 0x568;
	constexpr uint64_t InputAxisProperties = 0x138;

	constexpr uint64_t LastUpdateVelocity = 0x3E0;
	constexpr uint64_t ComponentVelocity = 0x234;
	constexpr uint64_t Mesh = 0x480;                           // 更新
	constexpr uint64_t RootComponent = 0x1D0;                  // 更新
	constexpr uint64_t StaticMesh = 0xB08;                     // 更新
	constexpr uint64_t Eyes = 0x75C;
	constexpr uint64_t bAlwaysCreatePhysicsState = 0x4A8;      // 更新

	constexpr uint64_t VehicleMovement = 0x488;
	constexpr uint64_t VehicleRiderComponent = 0x2110;         // 更新
	constexpr uint64_t ReplicatedMovement = 0xD0;              // 更新
	constexpr uint64_t LastVehiclePawn = 0x270;                // 更新
	constexpr uint64_t SeatIndex = 0x230;                      // 更新

	constexpr uint64_t Wheels = 0x328;
	constexpr uint64_t WheelLocation = 0x100;
	constexpr uint64_t DampingRate = 0x54;
	constexpr uint64_t ShapeRadius = 0x48;

	constexpr uint64_t DroppedItemGroup = 0xF8;
	constexpr uint64_t ItemPackageItems = 0x598;
	constexpr uint64_t DroppedItemGroupUItem = 0x870;

	constexpr uint64_t AttachedItems = 0x878;
	constexpr uint64_t WeaponAttachmentData = 0x128;
	constexpr uint64_t ItemTable = 0xB0;
	constexpr uint64_t ItemID = 0x240;
	constexpr uint64_t DroppedItem = 0x478;

	constexpr uint64_t AnimScriptInstance = 0xE50;             // 更新
	constexpr uint64_t PreEvalPawnState = 0x638;

	constexpr uint64_t bIsInVehicle_CP = 0x63C;
	constexpr uint64_t bIsParachuting_CP = 0x92E;
	constexpr uint64_t bIsFreefalling_CP = 0x92D;
	constexpr uint64_t bEmergencyPickup_Flying_CP = 0x92F;
	constexpr uint64_t bIsReviving_CP = 0x932;
	constexpr uint64_t bIsSwimming_CP = 0x935;

	constexpr uint64_t VTable = 0xA68;
	constexpr uint64_t bIsDBNO_CP = 0x931;
	constexpr uint64_t bIsDBNO0 = 0x353A;
	constexpr uint64_t bIsDBNO1 = 0x3538;
	constexpr uint64_t bIsDBNO2 = 0x3539;
	constexpr uint64_t bIsScoping_CP = 0x85D;
	constexpr uint64_t bIsPreparingThrow_CP = 0x540;
	constexpr uint64_t bIsThrowing_CP = 0x938;
	constexpr uint64_t bIsFlashed_CP = 0x63F;
	constexpr uint64_t bIsReloading_CP = 0x73D;
	constexpr uint64_t RecoilADSRotation_CP = 0x824;           // 更新
	constexpr uint64_t ControlRotation_CP = 0x654;             // 更新
	constexpr uint64_t LeanLeftAlpha_CP = 0x69C;               // 更新
	constexpr uint64_t LeanRightAlpha_CP = 0x6A0;              // 更新
	constexpr uint64_t CurrentAmmoData = 0xCA8;

	constexpr uint64_t StaticSockets = 0xC8;
	constexpr uint64_t StaticSocketName = 0x30;
	constexpr uint64_t StaticRelativeScale = 0x50;
	constexpr uint64_t StaticRelativeLocation = 0x38;
	constexpr uint64_t StaticRelativeRotation = 0x44;

	constexpr uint64_t InputYawScale = 0x66C;

	constexpr uint64_t AimOffsets = 0x1C78;

	constexpr uint64_t AntiCheatCharacterSyncManager = 0x1168;

	constexpr uint64_t TimeSeconds = 0x954;
	constexpr uint64_t TimeTillExplosion = 0x844;
	constexpr uint64_t ExplodeState = 0x648;
	constexpr uint64_t TrainingMapGrid = 0x5E0;
	constexpr uint64_t RecentlyRendered = 0xBD8;
	constexpr uint64_t LastSubmitTime = 0x778;
	constexpr uint64_t LastRenderTimeOnScreen = 0x77C;
	constexpr uint64_t MortarRotation = 0x540;
	constexpr uint64_t MortarEntity = 0xA8;

	constexpr uint64_t MapGrid_Map = 0x4A8;
	constexpr uint64_t Gender = 0xB70;                          // 更新

	constexpr uint64_t MouseX = 0x5016;
	constexpr uint64_t MouseY = 0x5017;

	constexpr uint64_t LineTraceSingle = 0xCBED628;
	constexpr uint64_t HOOK = 0x117F9B58;
	constexpr uint64_t HOOK_TWO = 0xCEDB7E4;

	constexpr uint64_t InventoryFacade = 0x1F00;
	constexpr uint64_t Inventory = 0xB0;
	constexpr uint64_t InventoryItems = 0x6A8;
	constexpr uint64_t InventoryItemCount = 0x6B0;
	constexpr uint64_t InventoryItemTagItemCount = 0x40;

	constexpr uint64_t Equipment = 0x430;
	constexpr uint64_t ItemsArray = 0x588;
	constexpr uint64_t Durability = 0x1E4;
	constexpr uint64_t Durabilitymax = 0x1E0;

	constexpr uint64_t VehicleCommonComponent = 0xB50;
	constexpr uint64_t FloatingVehicleCommonComponent = 0x4F8;
	constexpr uint64_t VehicleFuel = 0x2E0;
	constexpr uint64_t VehicleFuelMax = 0x2E4;
	constexpr uint64_t VehicleHealth = 0x2D8;
	constexpr uint64_t VehicleHealthMax = 0x2DC;

	constexpr uint64_t PhysxSDK = 0x11992F48;                  // 更新
	constexpr uint64_t PhysicsScene = 0x368;
	constexpr uint64_t mPhysXScene = 0xD0;
	constexpr uint64_t rigid_dynamics = 0x3B98;
	constexpr uint64_t Unreal_Engine = 0x11DF6510;

	constexpr uint64_t CurrentMinimapViewScale1D = 0x4A4;
	constexpr uint64_t LastMinimapPos = 0x4B8;
	constexpr uint64_t Minimap = 0x2D8;
	constexpr uint64_t GlobalAnimRateScale = 0xED8;
	constexpr uint64_t LastDamage = 0x1B60;
	constexpr uint64_t RecoilValueVector = 0x1154;
	constexpr uint64_t VerticalRecovery = 0x1168;

	// 以下为旧数据中未被覆盖的项，保留原值
	constexpr uint64_t Health = 0x970;
	constexpr uint64_t Offsets_ = 0x0;
	constexpr uint64_t Physx_sdk = 0x12270548;
	constexpr uint64_t MortarLocation = 0xB0;
	constexpr uint64_t MapMarker = 0xD0;
	constexpr uint64_t SPOOFCALL = 0x0;
	constexpr uint64_t bIsDead = 0x3538;
	constexpr uint64_t FloatingComponent = 0x4F8;
	constexpr uint64_t Ping = 0x448;
	

	inline void Sever_Init()
	{
		// --- 血量解密 ---
		GameData.Offset["Health"] = Health4;
		GameData.Offset["bEncryptedHealth"] = Health4 + 0x15;
		GameData.Offset["EncryptedHealthOffset"] = Health4 + 0x14;
		GameData.Offset["DecryptedHealthOffset"] = Health4 + 0x10;

		GameData.Offset["HeaFlag"] = HeaFlag;
		GameData.Offset["Health1"] = Health1;
		GameData.Offset["Health2"] = Health2;
		GameData.Offset["Health3"] = Health3;
		GameData.Offset["Health4"] = Health4;
		GameData.Offset["Health5"] = Health5;
		GameData.Offset["Health6"] = Health6;
		GameData.Offset["GroggyHealth"] = GroggyHealth;
		GameData.Offset["BlueBlockerGaugeTotalMax"] = BlueBlockerGaugeTotalMax;

		GameData.Offset["DecryptedHealthOffsets0"] = Health_keys0;
		GameData.Offset["DecryptedHealthOffsets1"] = Health_keys1;
		GameData.Offset["DecryptedHealthOffsets2"] = Health_keys2;
		GameData.Offset["DecryptedHealthOffsets3"] = Health_keys3;
		GameData.Offset["DecryptedHealthOffsets4"] = Health_keys4;
		GameData.Offset["DecryptedHealthOffsets5"] = Health_keys5;
		GameData.Offset["DecryptedHealthOffsets6"] = Health_keys6;
		GameData.Offset["DecryptedHealthOffsets7"] = Health_keys7;
		GameData.Offset["DecryptedHealthOffsets8"] = Health_keys8;
		GameData.Offset["DecryptedHealthOffsets9"] = Health_keys9;
		GameData.Offset["DecryptedHealthOffsets10"] = Health_keys10;
		GameData.Offset["DecryptedHealthOffsets11"] = Health_keys11;
		GameData.Offset["DecryptedHealthOffsets12"] = Health_keys12;
		GameData.Offset["DecryptedHealthOffsets13"] = Health_keys13;
		GameData.Offset["DecryptedHealthOffsets14"] = Health_keys14;
		GameData.Offset["DecryptedHealthOffsets15"] = Health_keys15;

		// --- 引擎核心 ---
		GameData.Offset["XenuineDecrypt"] = XenuineDecrypt;
		GameData.Offset["UWorld"] = UWorld;
		GameData.Offset["GNames"] = GNames;
		GameData.Offset["GNamesPtr"] = GNamesPtr;
		GameData.Offset["ChunkSize"] = ChunkSize;
		GameData.Offset["ObjID"] = ObjID;
		GameData.Offset["GameInstance"] = GameInstance;
		GameData.Offset["LocalPlayer"] = LocalPlayer;
		GameData.Offset["Actors"] = Actors;
		GameData.Offset["CurrentLevel"] = CurrentLevel;
		GameData.Offset["PlayerController"] = PlayerController;
		GameData.Offset["AcknowledgedPawn"] = AcknowledgedPawn;
		GameData.Offset["PlayerCameraManager"] = PlayerCameraManager;
		GameData.Offset["GameState"] = GameState;
		GameData.Offset["NumAliveTeams"] = NumAliveTeams;
		GameData.Offset["ActorsForGC"] = ActorsForGC;

		// --- GNames 解密参数 ---
		GameData.Offset["DecryptNameIndexRor"] = DecryptNameIndexRor;
		GameData.Offset["DecryptNameIndexXorKey1"] = DecryptNameIndexXorKey1;
		GameData.Offset["DecryptNameIndexXorKey2"] = DecryptNameIndexXorKey2;
		GameData.Offset["DecryptNameIndexXorKey3"] = DecryptNameIndexXorKey3;
		GameData.Offset["DecryptNameIndexRval"] = DecryptNameIndexRval;
		GameData.Offset["DecryptNameIndexSval"] = DecryptNameIndexSval;
		GameData.Offset["DecryptNameIndexDval"] = DecryptNameIndexDval;

		// --- 相机 ---
		GameData.Offset["ViewTarget"] = ViewTarget;
		GameData.Offset["CameraCacheLocation"] = CameraCacheLocation;
		GameData.Offset["CameraCacheRotation"] = CameraCacheRotation;
		GameData.Offset["CameraCacheFOV"] = CameraCacheFOV;

		// --- 队伍/玩家信息 ---
		GameData.Offset["LastTeamNum"] = LastTeamNum;
		GameData.Offset["TeamNumber"] = TeamNumber;
		GameData.Offset["PlayerArray"] = PlayerArray;
		GameData.Offset["AccountId"] = AccountId;
		GameData.Offset["PlayerName"] = PlayerName;
		GameData.Offset["PlayerStatusType"] = PlayerStatusType;
		GameData.Offset["SquadMemberIndex"] = SquadMemberIndex;
		GameData.Offset["PlayerState"] = PlayerState;
		GameData.Offset["PlayerStatistics"] = PlayerStatistics;
		GameData.Offset["DamageDealtOnEnemy"] = DamageDealtOnEnemy;
		GameData.Offset["SpectatedCount"] = SpectatedCount;
		GameData.Offset["PartnerLevel"] = PartnerLevel;
		GameData.Offset["SurvivalTier"] = SurvivalTier;
		GameData.Offset["SurvivalLevel"] = SurvivalLevel;
		GameData.Offset["CharacterClanInfo"] = CharacterClanInfo;
		GameData.Offset["CustomTimeDilation"] = CustomTimeDilation;

		// --- HUD / 组件 ---
		GameData.Offset["MyHUD"] = MyHUD;
		GameData.Offset["bShowMouseCursor"] = bShowMouseCursor;
		GameData.Offset["BlockInputWidgetList"] = BlockInputWidgetList;
		GameData.Offset["ComponentLocation"] = ComponentLocation;
		GameData.Offset["ComponentToWorld"] = ComponentToWorld;
		GameData.Offset["CharacterState"] = CharacterState;
		GameData.Offset["CharacterName"] = CharacterName;
		GameData.Offset["CharacterMovement"] = CharacterMovement;
		GameData.Offset["WorldToMap"] = WorldToMap;
		GameData.Offset["LayoutData"] = LayoutData;
		GameData.Offset["Offsets"] = Offsets_;
		GameData.Offset["Alignment"] = Alignment;
		GameData.Offset["Visibility"] = Visibility;
		GameData.Offset["SelectMinimapSizeIndex"] = SelectMinimapSizeIndex;
		GameData.Offset["Slot"] = Slot;
		GameData.Offset["WidgetStateMap"] = WidgetStateMap;

		// --- 安全区 ---
		GameData.Offset["FeatureRepObject"] = FeatureRepObject;
		GameData.Offset["SafetyZonePosition"] = SafetyZonePosition;
		GameData.Offset["SafetyZoneRadius"] = SafetyZoneRadius;
		GameData.Offset["BlueZoneRadius"] = BlueZoneRadius;
		GameData.Offset["BlueZonePosition"] = BlueZonePosition;

		// --- Mesh / 骨骼 ---
		GameData.Offset["Mesh"] = Mesh;
		GameData.Offset["StaticMesh"] = StaticMesh;
		GameData.Offset["RootComponent"] = RootComponent;
		GameData.Offset["LastUpdateVelocity"] = LastUpdateVelocity;
		GameData.Offset["ComponentVelocity"] = ComponentVelocity;
		GameData.Offset["Eyes"] = Eyes;
		GameData.Offset["bAlwaysCreatePhysicsState"] = bAlwaysCreatePhysicsState;

		// --- 输入 ---
		GameData.Offset["PlayerInput"] = PlayerInput;
		GameData.Offset["InputAxisProperties"] = InputAxisProperties;
		GameData.Offset["InputYawScale"] = InputYawScale;
		GameData.Offset["MouseX"] = MouseX;
		GameData.Offset["MouseY"] = MouseY;

		// --- 动画 ---
		GameData.Offset["AnimScriptInstance"] = AnimScriptInstance;
		GameData.Offset["PreEvalPawnState"] = PreEvalPawnState;

		// --- 自瞄 ---
		GameData.Offset["AimOffsets"] = AimOffsets;

		// --- 载具 ---
		GameData.Offset["VehicleMovement"] = VehicleMovement;
		GameData.Offset["VehicleRiderComponent"] = VehicleRiderComponent;
		GameData.Offset["ReplicatedMovement"] = ReplicatedMovement;
		GameData.Offset["LastVehiclePawn"] = LastVehiclePawn;
		GameData.Offset["SeatIndex"] = SeatIndex;
		GameData.Offset["Wheels"] = Wheels;
		GameData.Offset["WheelLocation"] = WheelLocation;
		GameData.Offset["DampingRate"] = DampingRate;
		GameData.Offset["ShapeRadius"] = ShapeRadius;
		GameData.Offset["VehicleCommonComponent"] = VehicleCommonComponent;
		GameData.Offset["FloatingComponent"] = FloatingComponent;
		GameData.Offset["VehicleFuel"] = VehicleFuel;
		GameData.Offset["VehicleFuelMax"] = VehicleFuelMax;
		GameData.Offset["VehicleHealth"] = VehicleHealth;
		GameData.Offset["VehicleHealthMax"] = VehicleHealthMax;
		GameData.Offset["FloatingVehicleCommonComponent"] = FloatingVehicleCommonComponent;

		// --- 物品 ---
		GameData.Offset["ItemID"] = ItemID;
		GameData.Offset["ItemTable"] = ItemTable;
		GameData.Offset["ItemPackageItems"] = ItemPackageItems;
		GameData.Offset["DroppedItemGroup"] = DroppedItemGroup;
		GameData.Offset["DroppedItem"] = DroppedItem;
		GameData.Offset["DroppedItemGroupUItem"] = DroppedItemGroupUItem;

		// --- 武器 ---
		GameData.Offset["WeaponProcessor"] = WeaponProcessor;
		GameData.Offset["CurrentAmmoData"] = CurrentAmmoData;
		GameData.Offset["CurrentWeaponIndex"] = CurrentWeaponIndex;
		GameData.Offset["EquippedWeapons"] = EquippedWeapons;
		GameData.Offset["WeaponTrajectoryData"] = WeaponTrajectoryData;
		GameData.Offset["TrajectoryGravityZ"] = TrajectoryGravityZ;
		GameData.Offset["TrajectoryConfig"] = TrajectoryConfig;
		GameData.Offset["BallisticCurve"] = BallisticCurve;
		GameData.Offset["FloatCurves"] = FloatCurves;
		GameData.Offset["Keys"] = Keys;
		GameData.Offset["WeaponConfig_WeaponClass"] = WeaponConfig_WeaponClass;
		GameData.Offset["Mesh3P"] = Mesh3P;
		GameData.Offset["FiringAttachPoint"] = FiringAttachPoint;
		GameData.Offset["AttachedStaticComponentMap"] = AttachedStaticComponentMap;
		GameData.Offset["AttachedItems"] = AttachedItems;
		GameData.Offset["WeaponAttachmentData"] = WeaponAttachmentData;
		GameData.Offset["ScopingAttachPoint"] = ScopingAttachPoint;
		GameData.Offset["ElapsedCookingTime"] = ElapsedCookingTime;
		GameData.Offset["RecoilValueVector"] = RecoilValueVector;
		GameData.Offset["VerticalRecovery"] = VerticalRecovery;

		// --- 物品栏 ---
		GameData.Offset["InventoryFacade"] = InventoryFacade;
		GameData.Offset["Inventory"] = Inventory;
		GameData.Offset["InventoryItems"] = InventoryItems;
		GameData.Offset["InventoryItemCount"] = InventoryItemCount;
		GameData.Offset["InventoryItemTagItemCount"] = InventoryItemTagItemCount;
		GameData.Offset["Equipment"] = Equipment;
		GameData.Offset["ItemsArray"] = ItemsArray;
		GameData.Offset["Durability"] = Durability;
		GameData.Offset["Durabilitymax"] = Durabilitymax;

		// --- CP 状态 ---
		GameData.Offset["ControlRotation_CP"] = ControlRotation_CP;
		GameData.Offset["RecoilADSRotation_CP"] = RecoilADSRotation_CP;
		GameData.Offset["LeanLeftAlpha_CP"] = LeanLeftAlpha_CP;
		GameData.Offset["LeanRightAlpha_CP"] = LeanRightAlpha_CP;
		GameData.Offset["bIsScoping_CP"] = bIsScoping_CP;
		GameData.Offset["bIsReloading_CP"] = bIsReloading_CP;
		GameData.Offset["bIsInVehicle_CP"] = bIsInVehicle_CP;
		GameData.Offset["bIsParachuting_CP"] = bIsParachuting_CP;
		GameData.Offset["bIsFreefalling_CP"] = bIsFreefalling_CP;
		GameData.Offset["bEmergencyPickup_Flying_CP"] = bEmergencyPickup_Flying_CP;
		GameData.Offset["bIsReviving_CP"] = bIsReviving_CP;
		GameData.Offset["bIsSwimming_CP"] = bIsSwimming_CP;
		GameData.Offset["bIsDBNO_CP"] = bIsDBNO_CP;
		GameData.Offset["bIsPreparingThrow_CP"] = bIsPreparingThrow_CP;
		GameData.Offset["bIsThrowing_CP"] = bIsThrowing_CP;
		GameData.Offset["bIsFlashed_CP"] = bIsFlashed_CP;
		GameData.Offset["VTable"] = VTable;
		GameData.Offset["bIsDBNO0"] = bIsDBNO0;
		GameData.Offset["bIsDBNO1"] = bIsDBNO1;
		GameData.Offset["bIsDBNO2"] = bIsDBNO2;

		// --- 静态插槽 ---
		GameData.Offset["StaticSockets"] = StaticSockets;
		GameData.Offset["StaticSocketName"] = StaticSocketName;
		GameData.Offset["StaticRelativeLocation"] = StaticRelativeLocation;
		GameData.Offset["StaticRelativeRotation"] = StaticRelativeRotation;
		GameData.Offset["StaticRelativeScale"] = StaticRelativeScale;

		// --- 反作弊 ---
		GameData.Offset["AntiCheatCharacterSyncManager"] = AntiCheatCharacterSyncManager;

		// --- 手雷/迫击炮 ---
		GameData.Offset["TimeSeconds"] = TimeSeconds;
		GameData.Offset["TimeTillExplosion"] = TimeTillExplosion;
		GameData.Offset["ExplodeState"] = ExplodeState;
		GameData.Offset["MortarLocation"] = MortarLocation;
		GameData.Offset["MortarRotation"] = MortarRotation;

		// --- 地图/训练场 ---
		GameData.Offset["TrainingMapGrid"] = TrainingMapGrid;
		GameData.Offset["MapMarker"] = MapMarker;
		GameData.Offset["MapGrid_Map"] = MapGrid_Map;
		GameData.Offset["Gender"] = Gender;
		GameData.Offset["CurrentMinimapViewScale1D"] = CurrentMinimapViewScale1D;
		GameData.Offset["LastMinimapPos"] = LastMinimapPos;
		GameData.Offset["Minimap"] = Minimap;

		// --- PhysX ---
		GameData.Offset["PhysxSDK"] = PhysxSDK;
		GameData.Offset["PhysicsScene"] = PhysicsScene;
		GameData.Offset["mPhysXScene"] = mPhysXScene;
		GameData.Offset["rigid_dynamics"] = rigid_dynamics;
		GameData.Offset["Unreal_Engine"] = Unreal_Engine;

		// --- 其他 ---
		GameData.Offset["LineTraceSingle"] = LineTraceSingle;
		GameData.Offset["HOOK"] = HOOK;
		GameData.Offset["HOOK_TWO"] = HOOK_TWO;
		GameData.Offset["RecentlyRendered"] = RecentlyRendered;
		GameData.Offset["LastSubmitTime"] = LastSubmitTime;
		GameData.Offset["LastRenderTimeOnScreen"] = LastRenderTimeOnScreen;
		GameData.Offset["CapsuleComponent"] = CapsuleComponent;
		GameData.Offset["Ping"] = Ping;
		GameData.Offset["MatchId"] = MatchId;

		return;
	}
}
