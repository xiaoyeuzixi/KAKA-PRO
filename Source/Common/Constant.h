#pragma once

#include <iostream>
#include <map>
#include <utils/utils.h>
#include <Utils/FNVHash.h>
#include <Utils/ue4math/rotator.h>
enum class EntityType
{
	Unknown = 0,// 未知
	Player = 1, // 玩家
	AI = 2, //  AI
	DroppedItem, // 掉落物品
	DroppedItemGroup, // 刷新物品
	Item, // 普通物品
	Vehicle, // 汽车
	Drug, // 药物
	Armor, // 防具
	Accessories, // 配件
	Attached, //装备配件
	Weapon, // 手持武器
	AirDrop,// 空投
	DeadBox, // 死亡盒子
	Grenade, // 投掷物品
	Project, // 投掷物品
	Widget,
	Map,
	Other,
	Wheel,
	key
};

enum class WeaponType
{
	None = -1,
	AR = 0, // 步枪
	DMR = 1, // 连狙
	SR = 2, // 栓狙
	SG = 3, // 散弹枪
	SMG = 4, // 冲锋枪
	LMG = 5, // 机枪
	HG = 6, // 手枪
	Drug = 7, // 药品
	Armor = 8, // 防具
	Muzzle = 9, // 枪口
	Grip = 10, // 握把
	Magazine = 11, //弹匣
	GunButt = 12, // 枪托
	Sight = 13, // 瞄具
	Bullet = 14, // 子弹
	Grenade = 15, // 投掷物
	Other = 16, // 其它
	key = 17, // 钥匙
	PanzerFaust100M1 = 18//火箭筒
};

inline std::unordered_map<WeaponType, std::string> WeaponTypeToString = {
	{WeaponType::AR, "AR"},
	{WeaponType::DMR, "DMR"},
	{WeaponType::SR, "SR"},
	{WeaponType::SG, "SG"},
	{WeaponType::SMG, "SMG"},
	{WeaponType::LMG, "LMG"},
	{WeaponType::HG, "HG"},
    {WeaponType::Grenade, "Grenade"},
	{WeaponType::PanzerFaust100M1, "PanzerFaust100M1"},
	{WeaponType::Other, "Other"},
};

struct FMapSize
{
	float Size;
	float ImageSize;
};

inline std::unordered_map<std::string, std::vector<FVector>> MapKey = {

	{ "Tiger_Main", {
		FVector(101973.66f, 340445.72f, 632.69f),
		FVector(125754.41f, 270115.06f, 1375.06f),
		FVector(139785.30f, 118843.00f, 2193.73f),
		FVector(257335.91f, 134945.42f, 7588.22f),
		FVector(357957.31f, 197454.56f, 12842.96f),
		FVector(483691.16f, 172000.22f, 6046.09f),
		FVector(691742.12f, 207942.67f, 5930.20f),
		FVector(710549.50f, 336822.28f, 2787.74f),
		FVector(604548.25f, 387233.12f, 1731.61f),
		FVector(442769.53f, 497417.28f, 3557.28f),
		FVector(641906.00f, 557295.00f, 610.79f),
		FVector(636142.25f, 719227.94f, 1173.13f),
		FVector(492981.12f, 642849.69f, 258.13f),
		FVector(242397.16f, 645694.81f, 2014.24f),
		FVector(95526.52f, 526490.44f, 570.50f)
	}},

	{ "Baltic_Main", {
		FVector(512839.16f, 66736.42f, 2853.12f),
		FVector(412836.00f, 197079.80f, 1906.42f),
		FVector(259117.12f, 221380.48f, 2820.92f),
		FVector(138088.56f, 181589.17f, 3906.12f),
		FVector(148159.66f, 354476.50f, 1476.44f),
		FVector(301332.44f, 375624.38f, 1643.95f),
		FVector(273834.91f, 515857.69f, 1093.19f),
		FVector(126498.68f, 553201.06f, 3765.29f),
		FVector(653177.19f, 207790.53f, 3562.41f),
		FVector(546677.81f, 342650.94f, 636.37f),
		FVector(466260.88f, 442908.50f, 1027.77f),
		FVector(676097.88f, 489827.44f, 836.81f),
		FVector(567731.38f, 673219.00f, 8523.79f),
		FVector(440321.16f, 592397.56f, 3821.66f),
		FVector(330258.22f, 670551.69f, 2795.70f)
	}},

	{ "DihorOtok_Main", {
		FVector(686222.44f, 389310.41f, 3885.85f),
		FVector(472003.00f, 496287.78f, 4948.68f),
		FVector(410856.69f, 322186.09f, 7560.97f),
		FVector(626797.75f, 246338.53f, 4339.92f),
		FVector(542327.94f, 131988.73f, 6713.17f),
		FVector(275364.84f, 158244.02f, 4749.27f),
		FVector(139500.31f, 385237.31f, 6130.00f),
		FVector(238347.02f, 564223.94f, 6143.36f),
		FVector(395834.47f, 655351.50f, 6353.28f),
		FVector(608683.06f, 588817.56f, 4875.27f)
	}},
	//荣都密室点位
	{ "Neon_Main", {
        FVector(147856.91f, 136199.58f, 2562.46f),
		FVector(303368.06f, 93759.38f, 1368.85f),
		FVector(581072.94f, 90835.25f, 294.05f),
		FVector(705487.88f, 274381.09f, 1638.13f),
		FVector(504355.31f, 206517.41f, 1481.62f),
		FVector(379163.31f, 266859.31f, 1035.7f),
		FVector(142563.17f, 324367.75f, 770.01f),
		FVector(144515.38f, 481002.03f, 197.8f),
		FVector(301945.5f, 515715.22f, 621.29f),
		FVector(468088.0f, 440685.69f, 2138.07f),
		FVector(663910.06f, 428932.0f, 1958.37f),
		FVector(569874.5f, 609975.56f, 1160.09f),
		FVector(494797.41f, 739082.38f, 12895.77f),
		FVector(339558.12f, 701422.44f, 6777.57f),
		FVector(125475.88f, 651855.56f, 7003.19f)
	}},

	{ "Kiki_Main", {
		FVector(195240.59f, 181194.22f, 7742.40f),
		FVector(186896.30f, 143690.02f, 7509.95f),
		FVector(192065.77f, 148015.80f, 1183.90f),
		FVector(313007.19f, 270254.97f, 6763.64f),
		FVector(452233.56f, 273215.12f, 6180.42f),
		FVector(508888.59f, 197256.69f, 2468.47f),
		FVector(670375.31f, 188572.55f, 129.96f),
		FVector(519426.09f, 89724.55f, 1279.93f),
		FVector(277182.50f, 53172.27f, 2063.55f),
		FVector(160921.06f, 348761.97f, 3843.44f),
		FVector(328560.91f, 359823.59f, 3209.80f),
		FVector(381283.88f, 456040.94f, 4544.19f),
		FVector(457305.81f, 396963.91f, 3462.24f),
		FVector(230048.75f, 471447.28f, 715.19f),
		FVector(163635.11f, 455909.59f, 3105.53f),
		FVector(114330.39f, 630890.50f, 1108.30f),
		FVector(377483.69f, 568906.62f, 5615.79f),
		FVector(552621.12f, 634857.06f, 1739.51f),
		FVector(735044.94f, 315836.62f, 1067.34f),
		FVector(651279.75f, 434457.81f, 6007.83f),
		FVector(667396.31f, 420749.91f, 6063.61f),
		FVector(657334.44f, 414980.81f, 7722.01f),
		FVector(647890.75f, 407093.22f, 6275.92f),
		FVector(650196.94f, 407438.66f, 6323.54f),
		FVector(629552.62f, 412809.00f, 6132.85f),
		FVector(619823.31f, 419636.47f, 714.13f),
		FVector(619084.31f, 422907.59f, 731.54f),
		FVector(611108.31f, 423122.06f, 7886.60f),
		FVector(617462.69f, 424144.31f, 7297.66f),
		FVector(601572.44f,470410.50f,85.32f),
		FVector(594108.62f,475971.19f,6040.06f),
		FVector(603232.19f,469204.31f,6678.24f),
		FVector(613640.88f,471480.19f,7081.88f),
		FVector(637714.31f,461010.47f,5407.09f),
		FVector(638041.88f,454399.84f,6005.38f),
		FVector(646998.19f,472588.00f,6618.99f),
		FVector(653284.56f,466584.53f,6369.20f),
		FVector(654785.06f,464996.75f,6368.75f),
		FVector(661112.75f,458064.09f,685.69f),
		FVector(673433.00f,434075.53f,708.82f),
		FVector(679323.25f,454011.12f,2840.80f),
		FVector(677276.06f,454236.94f,9420.32f),
		FVector(669538.62f,459954.97f,2718.07f),
		FVector(671585.75f,459919.84f,9302.42f),
		FVector(662554.81f,493295.03f,8372.78f)
	}},

	{ "Chimera_Main", {
		FVector(249550.31f, 175667.17f, -6765.92f),
		FVector(184227.09f, 184489.97f, -9143.24f),
		FVector(190443.31f, 150526.83f, -8090.38f),
		FVector(132740.34f, 193721.80f, -11513.64f),
		FVector(39279.35f, 193822.42f, -11993.07f),
		FVector(159027.97f, 246050.58f, -15356.85f),
		FVector(121649.93f, 97666.56f, -11283.83f),
		FVector(84628.89f, 109181.44f, -11086.60f)
	}},
};

inline std::unordered_map<std::string, FMapSize, FnvHash> MapsSize = {
	{"Tiger_Main", {408000.f, 816000.f}},
	{"Kiki_Main", {408000.f, 816000.f}},
	{"Desert_Main", {408000.f, 816000.f}},
	{"Range_Main", {101175.f, 204000.f}},
	{"Summerland_Main", {101175.f, 204000.f}},
	{"Italy_Main", {101175.f, 102000.f}},
	{"Baltic_Main", {406387.5f, 816000.f}},
	{"Neon_Main", {408000.f, 816000.f}},
	{"Heaven_Main", {51420.f, 102000.f}},
	{"Savage_Main", {202387.5f, 408000.f}},
	{"DihorOtok_Main", {408000.f, 816000.f}},
	{"Chimera_Main", {152950.f, 306000.f}}
};

inline std::unordered_map<std::string, std::string, FnvHash> RankTierToChinese = {
	{"Unranked", U8("未定级")},
	{"Bronze", U8("青铜")},
	{"Silver", U8("白银")},
	{"Gold", U8("黄金")},
	{"Platinum", U8("铂金")},
	{"Crystal", U8("水晶")},
	{"Diamond", U8("钻石")},
	{"Master", U8("大师")},
	{"Survivor", U8("生存者")},
	{"-", U8("-")},
};

inline std::unordered_map<std::string, std::string, FnvHash> RankTierToLChinese = {
	{"Unranked", "未定级"},
	{"Bronze", "青铜"},
	{"Silver", "白银"},
	{"Gold", "黄金"},
	{"Platinum", "铂金"},
	{"Crystal", "水晶"},
	{"Diamond", "钻石"},
	{"Master", "大师"},
	{"Survivor", "生存者"},
	{"-", "-"},
};

enum class Scene
{
	FindProcess = 0,
	Lobby = 1,
	Gaming = 2,
};

enum class TextAlignment {
	Left,
	Center,
	Right
};

