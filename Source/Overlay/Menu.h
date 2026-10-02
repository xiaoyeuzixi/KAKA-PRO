#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Common/Constant.h>
#include <DMALibrary/Memory/Memory.h>
#include <imgui.h>
#include "imgui_settings.h"
#include "imgui_internal.h"
#include "RenderHelper.h"
#include <string>
#include <Utils/KmBox.h>
#include <Utils/KmBoxNet.h>
#include <Utils/Lurker.h>
#include <Utils/MoBox.h>
#include <Utils/Throttler.h>
#include <Utils/Utils.h>
#include <item_image.h>
#include "Utils/Utils.h"
#include "Common/Config.h"
#include "font.h"
#include "imgui_elements.h"
#include "imgui_freetype.h"
#include <cstdlib>
#include <notify.h>
#include "MenuPlayerLists.h"
#include "MenuTheme.h"

std::string FormatRemainingTime(int seconds, const std::string& language)
{
	bool isEnglish = (language == "en" || language == "en-us" || language == "en_US");

	if (seconds <= 0)
		return isEnglish ? "Expired" : U8("已过期");

	int days = seconds / 86400;
	int hours = (seconds % 86400) / 3600;
	int minutes = (seconds % 3600) / 60;
	int secs = seconds % 60;

	std::ostringstream oss;

	if (isEnglish)
	{
		oss << days << U8("天 ")
			<< std::setw(2) << std::setfill('0') << hours << U8("时 ")
			<< std::setw(2) << std::setfill('0') << minutes << U8("分 ")
			<< std::setw(2) << std::setfill('0') << secs << U8("秒");

	}
	else
	{
		oss << days << "d "
			<< std::setw(2) << std::setfill('0') << hours << "h "
			<< std::setw(2) << std::setfill('0') << minutes << "m "
			<< std::setw(2) << std::setfill('0') << secs << "s";
	}

	return oss.str();
}

ImU32 GetExpireColor(int remainingSeconds)
{
	if (remainingSeconds <= 3600) // 小于1小时
		return GetColorU32(IM_COL32(255, 0, 0, 255)); // 红色
	else if (remainingSeconds <= 86400) // 小于1天
		return GetColorU32(IM_COL32(255, 255, 0, 255)); // 黄色
	else
		return GetColorU32(IM_COL32(0, 255, 0, 255)); // 绿色
}

static int lastUpdateTime = 0;       // 记录上次更新时间（秒级）
static int cachedRemainingTime = 0;  // 存储当前剩余时间
static bool isInitialized = false;   // 是否已初始化
using namespace ImGui;
RECT menuRect;
inline bool Languages = 1;
std::string formatTime(const std::string& input) {
	std::regex timePattern(R"((\d{4})年(\d{1,2})月(\d{1,2})日(\d{1,2})时(\d{1,2})分(\d{1,2})秒)");
	std::smatch match;

	if (std::regex_match(input, match, timePattern)) {
		if (match.size() == 7) {
			// 提取各个部分并格式化为目标格式
			std::string year = match[1];
			std::string month = match[2];
			std::string day = match[3];
			std::string hour = match[4];
			std::string minute = match[5];
			std::string second = match[6];

			return year + "-" + month + "-" + day + "-" + hour + ":" + minute + ":" + second;
		}
	}

	// 如果输入格式不符合预期，返回原始输入
	return input;
}

char filterPlayerName[256] = "";
enum class MenuStyle
{
	Dark,
	Light,
};

int menu_style;

MenuStyle currentMenuStyle = MenuStyle::Light;

int tabs = 0;
int item_tabs = 0;


int text_add = 0;
inline DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;

ImGuiStyle* ImStyle = &ImGui::GetStyle();

bool rememberme = true;
char field[46] = { "HELLO WORLD I AM ALEX(AN1XER)" };
char search_char[46] = {};
char field_[46] = { "HELLO WORLD I AM ALEX(AN1XER)" };
int select_combo = 0;
int select_combo_ = 0;
const char* items[3]{ "Item 0", "Item 1", "Item 2" };


// Pluto 主标签（含「其他配置」）
const char* GetMainTabTitle(int index) {
	static const char* tabs[2][4] = {
		{U8("主要透视"), U8("自瞄/无后座"), U8("物品透视"), U8("其他配置")},
		{"Main ESP", "Aimbot/Recoil", "Item ESP", "Other"}
	};
	if (index < 0 || index >= 4) return "";
	return tabs[Languages == 1 ? 0 : 1][index];
}
const char* GetItemsSubTabTitle(int index) {
	static const char* tabs[2][3] = {
		{U8("物品设置"), U8("物品分组"), U8("雷达设置")},
		{"Items", "Groups", "Radar"}
	};
	if (index < 0 || index >= 3) return "";
	return tabs[Languages == 1 ? 0 : 1][index];
}
// 兼容旧引用
const char* GetTabTitle(int index) {
	if (index < 4) return GetMainTabTitle(index);
	return GetItemsSubTabTitle(index - 2);
}
const char* GetTabTitle1(int index) {
	static const char* tabs[2][14] = {
		{

			U8("步枪"), U8("栓狙"), U8("连狙"), U8("机枪"), U8("霰弹枪"),
			U8("手枪"), U8("冲锋枪"),U8("配件"), U8("药品"), U8("防具"),
			U8("子弹"), U8("投掷物"), U8("钥匙"), U8("其他")


		}, // 中文
		{
			"Rifle", "Bolt Action Sniper", "Semi-Auto Sniper",
		 "Machine Gun", "Shotgun", "Pistol","SMG","Attachments","Medicals","Armor","Ammo","Throwables","Keys","Others"

		}  // 英文
	};
	return tabs[Languages == 1 ? 0 : 1][index];
}
// 图标定义（语言无关）
static const char* tabIcons[] = { "c", "o", "m", "d", "M", "f" };


bool multi_num[10] = { false, false, false, false, false , };
bool multi_num_[5] = { false, false, false, false, false };
bool esp_pre[12] = { true,false, false, false, false, false,true,false, false, false, false, false };


static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0; static float anim = 0.f; static float alpha = 0.f;
static int items_sub_tab = 0;

std::vector<std::string> words;

float tab_size = 0.f;
bool tab_opening = false;

ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

void OpenLink(const char* url) {
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

const char* itemsChinese[] = {
	U8("头皮"), U8("头部"), U8("脖子"), U8("胸部"), U8("裆部"),
	U8("左肩"), U8("左肘"), U8("右肩"), U8("右肘"),
	U8("左手"), U8("右手"), U8("左骨盆"), U8("左腿骨"),
	U8("右骨盆"), U8("右腿骨"), U8("左脚"), U8("右脚")
};


static std::string MenuLag(const char* strID, int Languages)
{
	std::map<std::string, std::string> languageMaps;

	// 菜单名称翻译
	languageMaps["Perspective Settings"] = Languages == 0 ? "Perspective Settings" : U8("透视设置");
	languageMaps["Aimbot Settings"] = Languages == 0 ? "Aimbot Settings" : U8("自瞄设置");
	languageMaps["Item Settings"] = Languages == 0 ? "Item Settings" : U8("物品设置");
	languageMaps["Item Groups"] = Languages == 0 ? "Item Groups" : U8("物品分组");
	languageMaps["Radar Settings"] = Languages == 0 ? "Radar Settings" : U8("雷达设置");
	languageMaps["Other Configurations"] = Languages == 0 ? "Other Configurations" : U8("其他配置");
	languageMaps["Basic Settings"] = Languages == 0 ? "Basic Settings" : U8("基础设置");
	languageMaps["Model Settings"] = Languages == 0 ? "Model Settings" : U8("模型设置");
	languageMaps["GrenadePredict Settings"] = Languages == 0 ? "GrenadePredict Settings" : U8("手雷/迫击炮/火箭筒");
	languageMaps["Value Settings"] = Languages == 0 ? "Value Settings" : U8("数值设置");
	languageMaps["Perspective Display"] = Languages == 0 ? "Perspective Display" : U8("透视显示");
	languageMaps["ESP Switch Config"] = Languages == 0 ? "ESP Switch" : U8("透视开关");
	languageMaps["Match Info Config"] = Languages == 0 ? "Match Info" : U8("战局信息");
	languageMaps["Warning Misc Config"] = Languages == 0 ? "Warning" : U8("预警杂项");
	languageMaps["Visual Misc Config"] = Languages == 0 ? "Visual Misc" : U8("杂项调节");
	languageMaps["Player ESP Config"] = Languages == 0 ? "Player ESP" : U8("人物透视");
	languageMaps["Match Color Config"] = Languages == 0 ? "Colors" : U8("战局颜色");
	languageMaps["Explosive Draw Config"] = Languages == 0 ? "Explosives" : U8("爆炸物");
	languageMaps["Big Map ESP Config"] = Languages == 0 ? "Big Map" : U8("大地图");
	languageMaps["Mini Map ESP Config"] = Languages == 0 ? "Mini Map" : U8("小地图");
	languageMaps["Grenade Related Config"] = Languages == 0 ? "Grenade" : U8("手雷");
	languageMaps["Drawing Colors"] = Languages == 0 ? "Drawing Colors" : U8("绘制颜色");
	languageMaps["Perspective Settings"] = Languages == 0 ? "Perspective Settings" : U8("透视调节");
	languageMaps["Other Displays"] = Languages == 0 ? "Other Displays" : U8("其它显示");
	languageMaps["Aiming Values"] = Languages == 0 ? "Aiming Values" : U8("瞄准数值");
	languageMaps["Tire Lock Settings"] = Languages == 0 ? "Tire Lock Settings" : U8("锁胎设置");
	languageMaps["Missed Shot Settings"] = Languages == 0 ? "Missed Shot Settings" : U8("漏打设置");
	languageMaps["Auto Trigger"] = (Languages == 0) ? "Auto Trigger 1" : U8("自动扳机");
	languageMaps["Auto Trigger 2"] = (Languages == 0) ? "Auto Trigger 2" : U8("自动扳机 2");
	languageMaps["Aimbot Body Parts"] = Languages == 0 ? "Aimbot Body Parts" : U8("自瞄部位");
	languageMaps["RecoilSettings"] = Languages == 0 ? "RecoilSettings" : U8("压枪设置");
	languageMaps["RecoilDelay"] = Languages == 0 ? "RecoilDelay" : U8("压枪延迟");
	languageMaps["Item Perspective"] = Languages == 0 ? "Item Perspective" : U8("物品透视");
	languageMaps["Airdrop Perspective"] = Languages == 0 ? "Airdrop Perspective" : U8("空投透视");
	languageMaps["Vehicle Perspective"] = Languages == 0 ? "Vehicle Perspective" : U8("载具透视");
	languageMaps["Box Perspective"] = Languages == 0 ? "Box Perspective" : U8("盒子透视");
	languageMaps["Color Settings"] = Languages == 0 ? "Color Settings" : U8("颜色设置");
	languageMaps["Smart Display"] = Languages == 0 ? "Smart Display" : U8("智能显示");
	languageMaps["Container Filter"] = Languages == 0 ? "Container Filter" : U8("容器筛选");
	languageMaps["Item List"] = Languages == 0 ? "Item List" : U8("物品列表");
	languageMaps["Big Map Radar"] = Languages == 0 ? "Big Map Radar" : U8("大地图雷达");
	languageMaps["Mini Map Radar"] = Languages == 0 ? "Mini Map Radar" : U8("小地图雷达");
	languageMaps["Web Shared Radar"] = Languages == 0 ? "Web Shared Radar" : U8("网页共享雷达");
	languageMaps["Other Settings"] = Languages == 0 ? "Other Settings" : U8("其它设置");
	languageMaps["Grenade Settings"] = Languages == 0 ? "Grenade Settings" : U8("手雷设置");
	languageMaps["Radar Warning"] = Languages == 0 ? "Radar Warning" : U8("雷达预警");
	languageMaps["AimBot Config"] = Languages == 0 ? "AimBot Config" : U8("自瞄配置");
	languageMaps["Grenade Tip"] = Languages == 0 ? "Grenade Tip" : U8("掐雷提示");


	languageMaps["[ Welcome to Use PUBG ]"] = Languages == 0 ? "[ Welcome to Use Jax Kane ]" : U8("[ 欢迎使用 PUBG ]");

	auto it = languageMaps.find(strID);
	if (it != languageMaps.end())
	{
		return it->second;
	}

	return "";
}

ImVec2 GetTextureSize(ID3D11ShaderResourceView* texture) {
	if (!texture) {
		return ImVec2(0, 0);
	}

	ID3D11Resource* resource = nullptr;
	texture->GetResource(&resource);

	if (!resource) {
		return ImVec2(0, 0);
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	((ID3D11Texture2D*)resource)->GetDesc(&desc);

	resource->Release();

	return ImVec2(static_cast<float>(desc.Width), static_cast<float>(desc.Height));
}
class newMenu
{
public:
	static void AutoConnectBox() {
		static bool autoConnectInitialized = false;
		if (GameData.Config.AimBot.AutoConnect && !autoConnectInitialized) {
			autoConnectInitialized = true;
			std::thread autoConnectThread([&]() {
				std::this_thread::sleep_for(std::chrono::seconds(0));
				if (!GameData.Config.AimBot.Connected) {
					bool Connected = false;
					std::string extractedStr;
					auto ports = Utils::GetCOMPorts();
					if (ports.size() > 0) {
						extractedStr = Utils::ExtractSubstring(ports[GameData.Config.AimBot.COM], R"(COM(\d+))");
					}
					int COM = 0;
					if (extractedStr != "") {
						COM = std::stoi(extractedStr);
					}

					switch (GameData.Config.AimBot.Controller) {
					case 0:
						if (!GameData.Config.AimBot.Connected) {
							Connected = KmBox::Init(COM);
						}
						break;
					case 1:
						if (!GameData.Config.AimBot.Connected) {
							Connected = KmBoxNet::Init(GameData.Config.AimBot.IP, GameData.Config.AimBot.Port, GameData.Config.AimBot.UUID);
						}
						break;
					default:
						break;
					}

					GameData.Config.AimBot.Connected = Connected;
				}
				});
			autoConnectThread.detach();
		}
	}

	static void Setting(std::string type)
	{
		const ImVec2& region = ImGui::GetContentRegionMax();
		const ImVec2 aimbotContent = MenuTheme::AimbotContentSize(region);
		const ImVec2 col3 = MenuTheme::ColumnSizeFromContent(aimbotContent, 3);
		const ImGuiWindowFlags pf = MenuTheme::ScrollPanelFlags();
		ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, false, MenuTheme::ContentStartY(false) + MenuTheme::WeaponTabRowHeight));

		ImGui::BeginChild(false, "Child", "o", aimbotContent, false, MenuTheme::ScrollPanelFlags());
		{
			auto& Config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[type];

			ImGui::BeginGroup();
			{

				ImGui::BeginChild(true, MenuLag("Aimbot Settings", Languages).c_str(), "o", MenuTheme::StyledPanel(col3.x, 18, 4), false, MenuTheme::ScrollPanelFlags());
				{
					ImGui::CheckboxWishTips(Languages == 1 ? U8("开启自瞄") : "Enable Aim Assist", &Config.enable, "Enable Aim Assist");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("开启漏打") : "Enable Missed Shot", &Config.LineTraceSingle, "Enable Missed Shot");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("空弹不锁") : "No Lock on Empty Ammo", &Config.NoBulletNotAim, "No Lock on Empty Ammo");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("随机自瞄") : "Random Self-Aiming", &Config.RandomAim, "Random Self-Aiming");
					if (Config.RandomAim)
					{
						ImGui::SliderFloat1(Languages == 1 ? U8("随机因子") : "Random Factor", &Config.RandomFactor, 0.1f, 1.0f);
						ImGui::SliderInt1(Languages == 1 ? U8("变化间隔") : "Change Interval", &Config.RandomInterval, 100, 2000);

						// 添加随机自瞄身体部位选项
						ImGui::CheckboxWishTips(Languages == 1 ? U8("随机部位参数") : "Random Site Parameters", &Config.RandomBodyParts, "Random Site Parameters");
						if (Config.RandomBodyParts)
						{
							int randomBodyPartCount = Config.RandomBodyPartCount;
							ImGui::SliderInt1(Languages == 1 ? U8("随机部位数量") : "Random Number Of Parts", &randomBodyPartCount, 1, 5);
							Config.RandomBodyPartCount = randomBodyPartCount;

							int randomSpeed = Config.RandomSpeed;
							ImGui::SliderInt1(Languages == 1 ? U8("切换速度(毫秒)") : "Switching Speed (ms)", &randomSpeed, 100, 2000);
							Config.RandomSpeed = randomSpeed;

							/*ImGui::Text(U8("随机自瞄位置选择"));
							ImGui::Separator();*/

							/*const char* BodyPartNames[17] = {
								U8("头顶"),
								U8("头部"),
								U8("脖子"),
								U8("胸部"),
								U8("裆部"),
								U8("左肩"),
								U8("左肘"),
								U8("右肩"),
								U8("右肘"),
								U8("左手"),
								U8("右手"),
								U8("左骨盆"),
								U8("左腿骨"),
								U8("右骨盆"),
								U8("右腿骨"),
								U8("左脚"),
								U8("右脚")
							};*/

							//// 显示骨骼选择列表，使用多选框
							//for (int i = 0; i < 17; i++) {
							//	 ImGui::CheckboxWishTips(BodyPartNames[i], &Config.RandomBodyPartsList[i]);
							//}
						}
					}


					ImGui::CheckboxWishTips(Languages == 1 ? U8("曲线瞄准") : "Aim on Scope", &Config.UseBezierMovement, "Aim on Scope");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("开镜自瞄") : "Aim on Scope", &Config.IsScopeandAim, "Aim on Scope");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("动态范围") : "Dynamic Range", &Config.DynamicFov, "Dynamic Range");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("锁定切换") : "Lock Toggle", &Config.FOVenable, "Lock Toggle");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("掩体不锁") : "No Lock through Cover", &Config.VisibleCheck, "No Lock through Cover");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("弹道预判") : "Bullet Prediction", &Config.Prediction, "Bullet Prediction");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("不锁倒地") : "No Lock on Downed", &Config.IgnoreGroggy, "No Lock on Downed");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("热键合并") : "Hotkey Merge", &Config.HotkeyMerge, "Hotkey Merge");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("自瞄范围") : "Aim Assist Range", &Config.ShowFOV, "Aim Assist Range");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("击杀切换") : "Kill Switch", &Config.AutoSwitch, "Kill Switch");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("后座抑制") : "Auto Recoil Control", &Config.NoRecoil, "Auto Recoil Control");
					if (!Config.NoRecoil) {
						ImGui::CheckboxWishTips(Languages == 1 ? U8("原始弹道") : "Original Ballistics", &Config.OriginalRecoil, "Original Ballistics");
					}

					ImGui::CheckboxWishTips(Languages == 1 ? U8("显示预瞄") : "Show Pre-Aim", &GameData.Config.AimBot.ShowPoint, "Show Pre-Aim");

					MenuTheme::BeginFullWidthRow();
					ImGui::ColorEdit5(Languages == 1 ? U8("范围颜色") : "Range Color", GameData.Config.AimBot.FOVColor, picker_flags);
					ImGui::ColorEdit5(Languages == 1 ? U8("预瞄颜色") : "Pre-Aim Color", GameData.Config.AimBot.PointColor, picker_flags);

				}
				ImGui::EndChild(true);

			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			{

				ImGui::BeginChild(true, MenuLag("Aiming Values", Languages).c_str(), "b", MenuTheme::StyledPanel(col3.x, 3, 10), false, pf);
				{
					MenuTheme::BeginFullWidthRow();
					ImGui::Keybind(Languages == 1 ? U8("主要自瞄按键") : "Primary Aim Hotkey", &Config.First.Key);

					ImGui::Keybind(Languages == 1 ? U8("强制自瞄按键") : "Force Aim Hotkey", &Config.Second.Key);

					ImGui::Keybind(Languages == 1 ? U8("倒地自瞄按键") : "Downed Aim Hotkey", &Config.Groggy.Key);

					const char* Prioritizeselection[] = { Languages == 1 ? U8("准星距离优先") : "Screen Distance Priority",Languages == 1 ? U8("人物距离优先") : "Player Distance Priority" };
					ImGui::SetNextItemWidth(-1);
					ImGui::Combo_popup(Languages == 1 ? U8("优先锁定选择") : "Priority Lock Selection", &GameData.Config.AimBot.LockMode, Prioritizeselection, IM_ARRAYSIZE(Prioritizeselection));


					ImGui::SliderInt1(Languages == 1 ? U8("预瞄大小") : "Pre-Aim Size", &GameData.Config.AimBot.PointSize, 1, 20, "%d");

					ImGui::SliderInt1(Languages == 1 ? U8("最大距离") : "Max Distance", &Config.AimDistance, 0, 1000, "%d M");

					ImGui::SliderInt1(Languages == 1 ? U8("锁定范围") : "Lock Range", &Config.FOV, 1, 360, "%d");

					ImGui::SliderFloat1(Languages == 1 ? U8("Y轴平滑") : "Y-Axis Smoothing", &Config.YSpeed, 0.0f, 10.0f, "%.1f")
						;
					ImGui::SliderFloat1(Languages == 1 ? U8("X轴平滑") : "X-Axis Smoothing", &Config.XSpeed, 0.0f, 10.0f, "%.1f");

					ImGui::SliderFloat1(Languages == 1 ? U8("吸附强度") : "Snap Strength", &Config.AimSpeedMaxFactor, 0.1f, 2.0f, "%.1f");

					ImGui::SliderInt1(Languages == 1 ? U8("平滑比例") : "Smoothing Ratio", &Config.InitialValue, 100, 1500, "%d");

					ImGui::SliderInt1(Languages == 1 ? U8("最大帧数") : "Max FPS", &Config.FPS, 1, 1000, "%d FPS");

					ImGui::SliderInt1(Languages == 1 ? U8("延迟切换") : "Delay Switch", &Config.SwitchingDelay, 1, 10, "%d");
				}
				ImGui::EndChild(true);


			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			{
				ImGui::BeginChild(true, MenuLag("Tire Lock Settings", Languages).c_str(), "o", MenuTheme::StyledPanel(col3.x, 2, 3), false, pf);
				{
					MenuTheme::BeginFullWidthRow();
					ImGui::CheckboxWishTips(Languages == 1 ? U8("开启锁胎") : "Enable Tire Lock", &Config.AimWheel, "Enable Tire Lock");

					ImGui::CheckboxWishTips(Languages == 1 ? U8("显示范围") : "Show Range", &GameData.Config.AimBot.ShowWheelFOV, "Show Range");

					ImGui::SliderInt1(Languages == 1 ? U8("锁胎范围") : "Tire Lock Range", &Config.WheelFOV, 1, 360, "%d");

					ImGui::ColorEdit5(Languages == 1 ? U8("范围颜色") : "Range Color", GameData.Config.AimBot.WheelFOVColor, picker_flags);

					ImGui::Keybind(Languages == 1 ? U8("锁胎热键") : "Tire Lock Hotkey", &Config.Wheel.Key);

					ImGui::SliderFloat1(Languages == 1 ? U8("锁胎平滑") : "Tire Lock Smoothing", &Config.AimWheelSpeed, 0.1f, 10.f, "%.1f");
				}
				ImGui::EndChild(true);

				// 添加静态变量来跟踪当前选择的触发器类型
				static int selectedTrigger = 0; // 0 = Auto Trigger 1, 1 = Auto Trigger 2
				const char* triggerTypes[] = {
					Languages == 1 ? U8("自动扳机（无需按键）") : "Auto Trigger 1",
					Languages == 1 ? U8("自动扳机 （需要按键）") : "Auto Trigger 2"
				};

				// 创建 Combo 选择器
				ImGui::BeginChild(true, MenuLag("Auto Trigger", Languages).c_str(), "o", MenuTheme::StyledPanel(col3.x, 4, 2), false, pf);
				{
					MenuTheme::BeginFullWidthRow();
					// Combo 下拉菜单
					ImGui::SetNextItemWidth(-1);
					ImGui::Combo_popup(Languages == 1 ? U8("触发器类型") : "Trigger Type", &selectedTrigger, triggerTypes, IM_ARRAYSIZE(triggerTypes));

					ImGui::Separator(); // 添加分隔线

					// 根据选择显示不同的设置
					if (selectedTrigger == 0) // Auto Trigger 1
					{
						ImGui::CheckboxWishTips(Languages == 1 ? U8("启用自动扳机 1") : "Enable Auto Trigger 1", &Config.AutomaticShooting, "Enable Auto Trigger 1");
						//	ImGui::Keybind(Languages == 1 ? U8("扳机按键") : "Trigger Key", &Config.AutomaticShootingKey);
						ImGui::SliderInt1(Languages == 1 ? U8("扳机延迟") : "Trigger Delay", &Config.AutomaticShootingTime, 10, 1500, "%d");
						ImGui::SliderInt1(Languages == 1 ? U8("扳机范围") : "Trigger FOV", &Config.AutomaticShootingFOV, 1, 5, "%d");
						ImGui::SliderInt1(Languages == 1 ? U8("扳机距离") : "Trigger Distance", &Config.banjiAimDistance, 1, 500, "%d");
					}
					else if (selectedTrigger == 1) // Auto Trigger 2
					{
						ImGui::CheckboxWishTips(Languages == 1 ? U8("启用自动扳机 2") : "Enable Auto Trigger 2", &Config.AimAndShot, "Enable Auto Trigger 2");
						ImGui::SliderInt1(Languages == 1 ? U8("扳机延迟") : "Trigger Delay", &Config.Delay1, 10, 1500, "%d");
						ImGui::SliderFloat1(Languages == 1 ? U8("扳机阈值") : "Trigger threshold", &Config.Threshold, 1.0f, 15.0f, "%.1f");
						ImGui::SliderInt1(Languages == 1 ? U8("扳机距离") : "Trigger Distance", &Config.banjiAimDistance, 1, 500, "%d");
					}
				}
				ImGui::EndChild(true);

			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();

			ImGui::BeginChild(true, MenuLag("Aimbot Body Parts", Languages).c_str(), "o", MenuTheme::StyledPanel(col3.x, 51, 0), false, MenuTheme::ScrollPanelFlags());
			{
				MenuTheme::BeginFullWidthRow();
				static int combo;
				const char* comboItems[] = { Languages == 1 ? U8("主要部位") : "Main Body",Languages == 1 ? U8("强制部位") : "Force Body",Languages == 1 ? U8("倒地部位") : "Downed Body" };
				ImGui::SetNextItemWidth(-1);
				ImGui::SetCursorPosY(-10);
				ImGui::Combo_popup("##0", &combo, comboItems, IM_ARRAYSIZE(comboItems));

				ImGui::SetCursorPos({ 26,140 });
				ImGui::Image(texture::playermoder, ImVec2(220, 392));
				const int y_offset = 80;

				if (combo == 0)//主要部位
				{
					ImGui::SetCursorPos(ImVec2(123, 65 + y_offset));
					ImGui::Checkbox_hitbox("Scalp", &Config.First.Bones[0]);// 选择框：头皮
					ImGui::SetCursorPos(ImVec2(123, 90 + y_offset));
					ImGui::Checkbox_hitbox("Head", &Config.First.Bones[1]); // 选择框：头部
					ImGui::SetCursorPos(ImVec2(123, 120 + y_offset));
					ImGui::Checkbox_hitbox("Neck", &Config.First.Bones[2]);// 选择框：脖部
					ImGui::SetCursorPos(ImVec2(122, 165 + y_offset));
					ImGui::Checkbox_hitbox("Body", &Config.First.Bones[3]);// 选择框：胸部
					ImGui::SetCursorPos(ImVec2(122, 235 + y_offset));
					ImGui::Checkbox_hitbox("Groin", &Config.First.Bones[4]);// 选择框：裆部
					ImGui::SetCursorPos(ImVec2(80, 155 + y_offset));
					ImGui::Checkbox_hitbox("Left_Shoulder", &Config.First.Bones[7]);// 选择框：左肩膀
					ImGui::SetCursorPos(ImVec2(55, 190 + y_offset));
					ImGui::Checkbox_hitbox("Left_Elbow", &Config.First.Bones[8]);// 选择框：左肘部
					ImGui::SetCursorPos(ImVec2(170, 155 + y_offset));
					ImGui::Checkbox_hitbox("Right_Shoulder", &Config.First.Bones[5]);// 选择框：右肩膀
					ImGui::SetCursorPos(ImVec2(190, 190 + y_offset));
					ImGui::Checkbox_hitbox("Right_Elbow", &Config.First.Bones[6]);// 选择框：右肘部
					ImGui::SetCursorPos(ImVec2(45, 235 + y_offset));
					ImGui::Checkbox_hitbox("Left_Hand", &Config.First.Bones[10]);// 选择框：左手
					ImGui::SetCursorPos(ImVec2(201, 235 + y_offset));
					ImGui::Checkbox_hitbox("Right_Hand", &Config.First.Bones[9]);// 选择框：右手
					ImGui::SetCursorPos(ImVec2(100, 260 + y_offset));
					ImGui::Checkbox_hitbox("Left_Pelvis", &Config.First.Bones[13]);// 选择框：左骨盆
					ImGui::SetCursorPos(ImVec2(100, 335 + y_offset));
					ImGui::Checkbox_hitbox("Left_Leg_Bone", &Config.First.Bones[14]);// 选择框：左腿部
					ImGui::SetCursorPos(ImVec2(145, 260 + y_offset));
					ImGui::Checkbox_hitbox("Right_Pelvis", &Config.First.Bones[11]);// 选择框：右骨盆
					ImGui::SetCursorPos(ImVec2(150, 335 + y_offset));
					ImGui::Checkbox_hitbox("Right_Leg_Bone", &Config.First.Bones[12]);// 选择框：右腿部
					ImGui::SetCursorPos(ImVec2(90, 430 + y_offset));
					ImGui::Checkbox_hitbox("Left_Foot", &Config.First.Bones[16]);// 选择框：左脚
					ImGui::SetCursorPos(ImVec2(160, 430 + y_offset));
					ImGui::Checkbox_hitbox("Right_Foot", &Config.First.Bones[15]);// 选择框：右脚
				}
				else if (combo == 1)//强制部位
				{
					ImGui::SetCursorPos(ImVec2(123, 65 + y_offset));
					ImGui::Checkbox_hitbox("Scalp", &Config.Second.Bones[0]);// 选择框：头皮
					ImGui::SetCursorPos(ImVec2(123, 90 + y_offset));
					ImGui::Checkbox_hitbox("Head", &Config.Second.Bones[1]);// 选择框：头部
					ImGui::SetCursorPos(ImVec2(123, 120 + y_offset));
					ImGui::Checkbox_hitbox("Neck", &Config.Second.Bones[2]);// 选择框：脖部
					ImGui::SetCursorPos(ImVec2(122, 165 + y_offset));
					ImGui::Checkbox_hitbox("Body", &Config.Second.Bones[3]);// 选择框：胸部
					ImGui::SetCursorPos(ImVec2(122, 235 + y_offset));
					ImGui::Checkbox_hitbox("Groin", &Config.Second.Bones[4]);// 选择框：裆部
					ImGui::SetCursorPos(ImVec2(80, 155 + y_offset));
					ImGui::Checkbox_hitbox("Left_Shoulder", &Config.Second.Bones[7]);// 选择框：左肩膀
					ImGui::SetCursorPos(ImVec2(55, 190 + y_offset));
					ImGui::Checkbox_hitbox("Left_Elbow", &Config.Second.Bones[8]);// 选择框：左肘部
					ImGui::SetCursorPos(ImVec2(170, 155 + y_offset));
					ImGui::Checkbox_hitbox("Right_Shoulder", &Config.Second.Bones[5]);// 选择框：右肩膀
					ImGui::SetCursorPos(ImVec2(190, 190 + y_offset));
					ImGui::Checkbox_hitbox("Right_Elbow", &Config.Second.Bones[6]);// 选择框：右肘部
					ImGui::SetCursorPos(ImVec2(45, 235 + y_offset));
					ImGui::Checkbox_hitbox("Left_Hand", &Config.Second.Bones[10]);// 选择框：左手
					ImGui::SetCursorPos(ImVec2(201, 235 + y_offset));
					ImGui::Checkbox_hitbox("Right_Hand", &Config.Second.Bones[9]);// 选择框：右手
					ImGui::SetCursorPos(ImVec2(100, 260 + y_offset));
					ImGui::Checkbox_hitbox("Left_Pelvis", &Config.Second.Bones[13]);// 选择框：左骨盆
					ImGui::SetCursorPos(ImVec2(100, 335 + y_offset));
					ImGui::Checkbox_hitbox("Left_Leg_Bone", &Config.Second.Bones[14]);// 选择框：左腿部
					ImGui::SetCursorPos(ImVec2(145, 260 + y_offset));
					ImGui::Checkbox_hitbox("Right_Pelvis", &Config.Second.Bones[11]);// 选择框：右骨盆
					ImGui::SetCursorPos(ImVec2(150, 335 + y_offset));
					ImGui::Checkbox_hitbox("Right_Leg_Bone", &Config.Second.Bones[12]);// 选择框：右腿部
					ImGui::SetCursorPos(ImVec2(90, 430 + y_offset));
					ImGui::Checkbox_hitbox("Left_Foot", &Config.Second.Bones[16]);// 选择框：左脚
					ImGui::SetCursorPos(ImVec2(160, 430 + y_offset));
					ImGui::Checkbox_hitbox("Right_Foot", &Config.Second.Bones[15]);// 选择框：右脚
				}
				else if (combo == 2)//倒地部位
				{
					ImGui::SetCursorPos(ImVec2(123, 65 + y_offset));
					ImGui::Checkbox_hitbox("Scalp", &Config.Groggy.Bones[0]);// 选择框：头皮
					ImGui::SetCursorPos(ImVec2(123, 90 + y_offset));
					ImGui::Checkbox_hitbox("Head", &Config.Groggy.Bones[1]);// 选择框：头部
					ImGui::SetCursorPos(ImVec2(123, 120 + y_offset));
					ImGui::Checkbox_hitbox("Neck", &Config.Groggy.Bones[2]);// 选择框：脖部
					ImGui::SetCursorPos(ImVec2(122, 165 + y_offset));
					ImGui::Checkbox_hitbox("Body", &Config.Groggy.Bones[3]);// 选择框：胸部
					ImGui::SetCursorPos(ImVec2(122, 235 + y_offset));
					ImGui::Checkbox_hitbox("Groin", &Config.Groggy.Bones[4]); // 选择框：裆部
					ImGui::SetCursorPos(ImVec2(80, 155 + y_offset));
					ImGui::Checkbox_hitbox("Left_Shoulder", &Config.Groggy.Bones[7]);// 选择框：左肩膀
					ImGui::SetCursorPos(ImVec2(55, 190 + y_offset));
					ImGui::Checkbox_hitbox("Left_Elbow", &Config.Groggy.Bones[8]);// 选择框：左肘部
					ImGui::SetCursorPos(ImVec2(170, 155 + y_offset));
					ImGui::Checkbox_hitbox("Right_Shoulder", &Config.Groggy.Bones[5]);// 选择框：右肩膀
					ImGui::SetCursorPos(ImVec2(190, 190 + y_offset));
					ImGui::Checkbox_hitbox("Right_Elbow", &Config.Groggy.Bones[6]);// 选择框：右肘部
					ImGui::SetCursorPos(ImVec2(45, 235 + y_offset));
					ImGui::Checkbox_hitbox("Left_Hand", &Config.Groggy.Bones[10]);// 选择框：左手
					ImGui::SetCursorPos(ImVec2(201, 235 + y_offset));
					ImGui::Checkbox_hitbox("Right_Hand", &Config.Groggy.Bones[9]);// 选择框：右手
					ImGui::SetCursorPos(ImVec2(100, 260 + y_offset));
					ImGui::Checkbox_hitbox("Left_Pelvis", &Config.Groggy.Bones[13]);// 选择框：左骨盆
					ImGui::SetCursorPos(ImVec2(100, 335 + y_offset));
					ImGui::Checkbox_hitbox("Left_Leg_Bone", &Config.Groggy.Bones[14]);// 选择框：左腿部
					ImGui::SetCursorPos(ImVec2(145, 260 + y_offset));
					ImGui::Checkbox_hitbox("Right_Pelvis", &Config.Groggy.Bones[11]);// 选择框：右骨盆
					ImGui::SetCursorPos(ImVec2(150, 335 + y_offset));
					ImGui::Checkbox_hitbox("Right_Leg_Bone", &Config.Groggy.Bones[12]);// 选择框：右腿部
					ImGui::SetCursorPos(ImVec2(90, 430 + y_offset));
					ImGui::Checkbox_hitbox("Left_Foot", &Config.Groggy.Bones[16]);// 选择框：左脚
					ImGui::SetCursorPos(ImVec2(160, 430 + y_offset));
					ImGui::Checkbox_hitbox("Right_Foot", &Config.Groggy.Bones[15]);// 选择框：右脚
				}
			}
			ImGui::EndChild(true);

			ImGui::EndGroup();

		}
	}


	static void SortItemsByGamePlayerInfo(std::vector<GamePlayerInfo>& items, int column, bool ascending, std::unordered_map<std::string, PlayerRankList> PlayerRankLists = {}) {
		auto compare = [&column, &ascending, &PlayerRankLists](const GamePlayerInfo& a, const GamePlayerInfo& b) {
			PlayerRankList APlayerRank, BPlayerRank;
			PlayerRankInfo APlayerRankData, BPlayerRankData;

			if (PlayerRankLists.count(a.PlayerName) > 0)
			{
				APlayerRank = PlayerRankLists[a.PlayerName];
			}

			if (PlayerRankLists.count(b.PlayerName) > 0)
			{
				BPlayerRank = PlayerRankLists[b.PlayerName];
			}

			if (GameData.Config.PlayerList.RankMode == 1)
			{
				APlayerRankData = APlayerRank.TPP;
				BPlayerRankData = BPlayerRank.TPP;
			}

			if (GameData.Config.PlayerList.RankMode == 2)
			{
				APlayerRankData = APlayerRank.SquadTPP;
				BPlayerRankData = BPlayerRank.SquadTPP;
			}

			if (GameData.Config.PlayerList.RankMode == 3)
			{
				APlayerRankData = APlayerRank.FPP;
				BPlayerRankData = BPlayerRank.FPP;
			}

			if (GameData.Config.PlayerList.RankMode == 4)
			{
				APlayerRankData = APlayerRank.SquadFPP;
				BPlayerRankData = BPlayerRank.SquadFPP;
			}

			switch (column) {
			case 0: return ascending ? a.TeamID < b.TeamID : a.TeamID > b.TeamID;
			case 1: return ascending ? a.PlayerName < b.PlayerName : a.PlayerName > b.PlayerName;
			case 2: return ascending ? APlayerRankData.Tier  < BPlayerRankData.Tier : APlayerRankData.Tier > BPlayerRankData.Tier;
			case 3: return ascending ? APlayerRankData.KDA < BPlayerRankData.KDA : APlayerRankData.KDA > BPlayerRankData.KDA;
			case 4: return ascending ? APlayerRankData.RankPoint < BPlayerRankData.RankPoint : APlayerRankData.RankPoint > BPlayerRankData.RankPoint;
			case 5: return ascending ? APlayerRankData.WinRatio < BPlayerRankData.WinRatio : APlayerRankData.WinRatio > BPlayerRankData.WinRatio;
			case 6: return ascending ? a.KillCount < b.KillCount : a.KillCount > b.KillCount;
			case 7: return ascending ? a.DamageDealtOnEnemy < b.DamageDealtOnEnemy : a.DamageDealtOnEnemy > b.DamageDealtOnEnemy;
			case 8: return ascending ? a.PubgIdData.SurvivalLevel < b.PubgIdData.SurvivalLevel : a.PubgIdData.SurvivalLevel > b.PubgIdData.SurvivalLevel;
			case 9: return ascending ? a.PartnerLevel < b.PartnerLevel : a.PartnerLevel > b.PartnerLevel;
			case 10: return ascending ? a.ListType < b.ListType : a.ListType > b.ListType;
			default: return false;
			}
			};
		std::sort(items.begin(), items.end(), compare);
	}

	//https://gitee.com/tyddyuan/shenxian/raw/master/offset
	static void Render(HWND hwnd)
	{
		const ImVec2 Spacing = ImGui::GetStyle().ItemSpacing;

		ImGui::SetNextWindowSize({ 920 + Spacing.x, 670 + Spacing.y });

		if (GameData.Config.Window.Players && ImGui::Begin(Languages == 1 ? U8("玩家列表") : "Player List", &GameData.Config.Window.Players, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImVec2 Pos = ImGui::GetWindowPos();
			Pos.x += Spacing.x / 2;
			Pos.y += Spacing.y / 2;

			ImGui::GetWindowDrawList()->AddShadowRect(Pos, ImVec2(Pos.x + 920, Pos.y + 670), ImGui::GetColorU32(c::shadow, 0.35f), 10, ImVec2(0, 0));
			ImGui::GetWindowDrawList()->AddRectFilled(Pos, ImVec2(Pos.x + 920, Pos.y + 670), ImGui::GetColorU32(c::bg::background), 10);
			//ImGui::SetCursorPos(ImVec2(18 + Spacing.x / 2, 18));

			ImGui::SetCursorPos(ImVec2(18 + Spacing.x / 2, 18));  // 你可以根据需求调整这行位置
			// 玩家名单过滤选项
			const char* filterPlayerList[] = { Languages == 1 ? U8("全部名单") : "All Lists",Languages == 1 ? U8("黑名单") : "Blacklist",Languages == 1 ? U8("白名单") : "Whitelist" };
			static int filterPlayerListIndex = 0;

			ImGui::SetNextItemWidth(100);
			if (ImGui::BeginCombo(U8("##名单类型"), filterPlayerList[filterPlayerListIndex], filterPlayerListIndex)) {
				for (int i = 0; i < IM_ARRAYSIZE(filterPlayerList); i++) {
					bool isSelected = (filterPlayerListIndex == i);
					if (ImGui::Selectable(filterPlayerList[i], isSelected))
						filterPlayerListIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();  // 确保下面的下拉框在同一行

			// 调整玩家类型下拉框位置
			ImGui::SetNextItemWidth(100);

			const char* filterPlayerType[] = { Languages == 1 ? U8("全部类型") : "All Types",Languages == 1 ? U8("玩家") : "Player",Languages == 1 ? U8("合作者") : "Collaborator",Languages == 1 ? U8("队友") : "Teammate",Languages == 1 ? U8("本人") : "Myself",Languages == 1 ? U8("人机") : "Bot",Languages == 1 ? U8("菜逼") : "Noob",Languages == 1 ? U8("挂逼") : "Cheater",Languages == 1 ? U8("老挂逼") : "Perma Cheat" };
			static int filterPlayerTypeIndex = 0;
			if (ImGui::BeginCombo(U8("##玩家类型"), filterPlayerType[filterPlayerTypeIndex], filterPlayerTypeIndex)) {
				for (int i = 0; i < IM_ARRAYSIZE(filterPlayerType); i++) {
					bool isSelected = (filterPlayerTypeIndex == i);
					if (ImGui::Selectable(filterPlayerType[i], isSelected))
						filterPlayerTypeIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);


			const char* rankModeOptions[] = { Languages == 1 ? U8("不查战绩") : "Hide Rank",Languages == 1 ? U8("TPP单人") : "TPP Solo",Languages == 1 ? U8("TPP小队") : "TPP Squad",Languages == 1 ? U8("FPP单人") : "FPP Solo",Languages == 1 ? U8("FPP小队") : "FPP Squad" };
			const char* currentRankMode = rankModeOptions[GameData.Config.PlayerList.RankMode];

			if (ImGui::BeginCombo(U8("##战绩数据"), rankModeOptions[GameData.Config.PlayerList.RankMode], GameData.Config.PlayerList.RankMode)) {
				for (int i = 0; i < IM_ARRAYSIZE(rankModeOptions); i++) {
					bool isSelected = (GameData.Config.PlayerList.RankMode == i);
					if (ImGui::Selectable(rankModeOptions[i], isSelected))
						GameData.Config.PlayerList.RankMode = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputTextEx(U8("   "), Languages == 1 ? U8("搜索玩家") : "Search Player", filterPlayerName, sizeof(filterPlayerName), ImVec2(130, 30), NULL);

			static ImGuiTableFlags flags = ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

			std::unordered_map<std::string, GamePlayerInfo> PlayerLists;
			PlayerLists = Data::GetPlayerLists();

			bool NeedUpdatePlayerLists = false;
			std::vector<GamePlayerInfo> players = {};
			for (auto player : PlayerLists) {
				players.push_back(player.second);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 8.0f));

			// 调整表格位置
			ImGui::SetCursorPos(ImVec2(18 + Spacing.x / 2, 18 + 70));  // 你可以根据需求调整这行位置
			// 玩家列表表格设置
			if (ImGui::BeginTable(Languages == 1 ? U8("玩家列表") : "Player List", 12, flags, ImVec2(900.f, 580.0f)))
			{
				ImGui::TableSetupScrollFreeze(0, 1);  // 冻结表头

				ImGui::TableSetupColumn(Languages == 1 ? U8("队伍") : "Team", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("玩家昵称") : "Player", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("段位") : "Rank", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("KD") : "KD", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("分数") : "Score", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("胜率") : "Win Rate", ImGuiTableColumnFlags_WidthFixed, 70.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("击杀") : "Kills", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("伤害") : "Damage", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("等级") : "Level", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("类型") : "Type", ImGuiTableColumnFlags_WidthFixed, 70.0f);
				ImGui::TableSetupColumn(Languages == 1 ? U8("名单") : "List", ImGuiTableColumnFlags_WidthFixed, 80.0f);

				// **自定义表头**
				ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
				for (int column = 0; column < 11; column++)
				{
					ImGui::TableSetColumnIndex(column);
					const char* header = ImGui::TableGetColumnName(column);  // 获取列名

					if (column == 2 || column == 10)  // 仅对 "段位" (索引 2) 和 "名单" (索引 10) 居中对齐
					{
						float width = ImGui::GetColumnWidth(column);  // 获取列宽
						float text_width = ImGui::CalcTextSize(header).x;  // 计算文本宽度
						float offset = (width - text_width) * 0.5f;  // 计算水平居中偏移
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);  // 设置居中位置
					}

					ImGui::TextUnformatted(header);  // 绘制表头文本
				}

				std::unordered_map<std::string, PlayerRankList> PlayerRankLists = Data::GetPlayerRankLists();

				if (ImGuiTableSortSpecs* SortsSpecs = ImGui::TableGetSortSpecs())
				{
					if (SortsSpecs->SpecsDirty)
					{
						SortItemsByGamePlayerInfo(players, SortsSpecs->Specs->ColumnIndex, SortsSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending, PlayerRankLists);
					}
				}

				for (auto& player : players)
				{
					if (player.PlayerName == "" || player.PlayerName == "Bear" || player.PlayerName == "Bird") {
						continue;
					}
					if (player.StatusType == 8 || player.StatusType == 12) {
						auto PlayerName = player.PlayerName;
						if (player.ClanName != "") {
							PlayerName = "[" + player.ClanName + "] " + player.PlayerName;
						}

						PlayerRankInfo PlayerRankData;
						PlayerRankList temp = Data::GetPlayerSegmentListsItem(player.PlayerName);

						switch (GameData.Config.PlayerList.RankMode) {
						case 1:
							PlayerRankData = temp.TPP;
							break;
						case 2:
							PlayerRankData = temp.SquadTPP;
							break;
						case 3:
							PlayerRankData = temp.FPP;
							break;
						case 4:
							PlayerRankData = temp.SquadFPP;
							break;
						default:
							break;
						}

						// 获取本地化段位文本（PUBG）
						std::string localizedTier;
						if (Languages == 1) { // 中文
							if (PlayerRankData.TierToString == U8("青铜1")) localizedTier = U8("青铜 1");
							else if (PlayerRankData.TierToString == U8("青铜2")) localizedTier = U8("青铜 2");
							else if (PlayerRankData.TierToString == U8("青铜3")) localizedTier = U8("青铜 3");
							else if (PlayerRankData.TierToString == U8("青铜4")) localizedTier = U8("青铜 4");
							else if (PlayerRankData.TierToString == U8("青铜5")) localizedTier = U8("青铜 5");
							else if (PlayerRankData.TierToString == U8("白银1")) localizedTier = U8("白银 1");
							else if (PlayerRankData.TierToString == U8("白银2")) localizedTier = U8("白银 2");
							else if (PlayerRankData.TierToString == U8("白银3")) localizedTier = U8("白银 3");
							else if (PlayerRankData.TierToString == U8("白银4")) localizedTier = U8("白银 4");
							else if (PlayerRankData.TierToString == U8("白银5")) localizedTier = U8("白银 5");
							else if (PlayerRankData.TierToString == U8("黄金1")) localizedTier = U8("黄金 1");
							else if (PlayerRankData.TierToString == U8("黄金2")) localizedTier = U8("黄金 2");
							else if (PlayerRankData.TierToString == U8("黄金3")) localizedTier = U8("黄金 3");
							else if (PlayerRankData.TierToString == U8("黄金4")) localizedTier = U8("黄金 4");
							else if (PlayerRankData.TierToString == U8("黄金5")) localizedTier = U8("黄金 5");
							else if (PlayerRankData.TierToString == U8("白金1")) localizedTier = U8("白金 1");
							else if (PlayerRankData.TierToString == U8("白金2")) localizedTier = U8("白金 2");
							else if (PlayerRankData.TierToString == U8("白金3")) localizedTier = U8("白金 3");
							else if (PlayerRankData.TierToString == U8("白金4")) localizedTier = U8("白金 4");
							else if (PlayerRankData.TierToString == U8("白金5")) localizedTier = U8("白金 5");
							else if (PlayerRankData.TierToString == U8("水晶1")) localizedTier = U8("水晶 1");
							else if (PlayerRankData.TierToString == U8("水晶2")) localizedTier = U8("水晶 2");
							else if (PlayerRankData.TierToString == U8("水晶3")) localizedTier = U8("水晶 3");
							else if (PlayerRankData.TierToString == U8("水晶4")) localizedTier = U8("水晶 4");
							else if (PlayerRankData.TierToString == U8("钻石1")) localizedTier = U8("钻石 1");
							else if (PlayerRankData.TierToString == U8("钻石2")) localizedTier = U8("钻石 2");
							else if (PlayerRankData.TierToString == U8("钻石3")) localizedTier = U8("钻石 3");
							else if (PlayerRankData.TierToString == U8("钻石4")) localizedTier = U8("钻石 4");
							else if (PlayerRankData.TierToString == U8("钻石5")) localizedTier = U8("钻石 5");
							else if (PlayerRankData.TierToString == U8("大师1")) localizedTier = U8("大师");
							else if (PlayerRankData.TierToString == U8("生存者1")) localizedTier = U8("生存者");
							else if (PlayerRankData.TierToString == U8("未定级")) localizedTier = U8("没有KD");
							else if (PlayerRankData.TierToString == U8("没有KD")) localizedTier = U8("没有KD");
						}
						else { // 英文
							if (PlayerRankData.TierToString == U8("青铜1")) localizedTier = U8("Bronze 1");
							else if (PlayerRankData.TierToString == U8("青铜2")) localizedTier = U8("Bronze 2");
							else if (PlayerRankData.TierToString == U8("青铜3")) localizedTier = U8("Bronze 3");
							else if (PlayerRankData.TierToString == U8("青铜4")) localizedTier = U8("Bronze 4");
							else if (PlayerRankData.TierToString == U8("青铜5")) localizedTier = U8("Bronze 5");
							else if (PlayerRankData.TierToString == U8("白银1")) localizedTier = U8("Silver 1");
							else if (PlayerRankData.TierToString == U8("白银2")) localizedTier = U8("Silver 2");
							else if (PlayerRankData.TierToString == U8("白银3")) localizedTier = U8("Silver 3");
							else if (PlayerRankData.TierToString == U8("白银4")) localizedTier = U8("Silver 4");
							else if (PlayerRankData.TierToString == U8("白银5")) localizedTier = U8("Silver 5");
							else if (PlayerRankData.TierToString == U8("黄金1")) localizedTier = U8("Gold 1");
							else if (PlayerRankData.TierToString == U8("黄金2")) localizedTier = U8("Gold 2");
							else if (PlayerRankData.TierToString == U8("黄金3")) localizedTier = U8("Gold 3");
							else if (PlayerRankData.TierToString == U8("黄金4")) localizedTier = U8("Gold 4");
							else if (PlayerRankData.TierToString == U8("黄金5")) localizedTier = U8("Gold 5");
							else if (PlayerRankData.TierToString == U8("白金1")) localizedTier = U8("Platinum 1");
							else if (PlayerRankData.TierToString == U8("白金2")) localizedTier = U8("Platinum 2");
							else if (PlayerRankData.TierToString == U8("白金3")) localizedTier = U8("Platinum 3");
							else if (PlayerRankData.TierToString == U8("白金4")) localizedTier = U8("Platinum 4");
							else if (PlayerRankData.TierToString == U8("白金5")) localizedTier = U8("Platinum 5");
							else if (PlayerRankData.TierToString == U8("水晶1")) localizedTier = U8("Crystal 1");
							else if (PlayerRankData.TierToString == U8("水晶2")) localizedTier = U8("Crystal 2");
							else if (PlayerRankData.TierToString == U8("水晶3")) localizedTier = U8("Crystal 3");
							else if (PlayerRankData.TierToString == U8("水晶4")) localizedTier = U8("Crystal 4");
							else if (PlayerRankData.TierToString == U8("钻石1")) localizedTier = U8("Diamond 1");
							else if (PlayerRankData.TierToString == U8("钻石2")) localizedTier = U8("Diamond 2");
							else if (PlayerRankData.TierToString == U8("钻石3")) localizedTier = U8("Diamond 3");
							else if (PlayerRankData.TierToString == U8("钻石4")) localizedTier = U8("Diamond 4");
							else if (PlayerRankData.TierToString == U8("钻石5")) localizedTier = U8("Diamond 5");
							else if (PlayerRankData.TierToString == U8("大师1")) localizedTier = U8("Master");
							else if (PlayerRankData.TierToString == U8("生存者1")) localizedTier = U8("Survivor");
							else if (PlayerRankData.TierToString == U8("未定级")) localizedTier = U8("Unranked");
							else if (PlayerRankData.TierToString == U8("没有KD")) localizedTier = U8("Unranked");

						}


						std::string RankTier = localizedTier;
						std::string RankKDA;
						if (PlayerRankData.KDAToString.empty())  // 如果 KDAToString 为空
						{
							if (PlayerRankData.KDA == 0.0) {
								RankKDA = "";  // 如果 KDA 为 0，显示为 "0.00"
							}
							else {
								std::ostringstream oss;
								oss << std::fixed << std::setprecision(2) << PlayerRankData.KDA;
								RankKDA = oss.str();  // 否则使用 KDA 生成字符串
							}
						}
						else {
							RankKDA = PlayerRankData.KDAToString;  // 如果 KDAToString 已经有值，直接使用
						}

						std::string RankPoint = std::to_string(static_cast<int>(PlayerRankData.RankPoint));
						if (PlayerRankData.RankPoint == 0)
						{
							RankPoint = "";
						}
						else
						{
							RankPoint = std::to_string(static_cast<int>(PlayerRankData.RankPoint));
						}

						std::string WinRatio = PlayerRankData.WinRatioToString;
						if (PlayerRankData.WinRatioToString.empty()) {
							if (PlayerRankData.WinRatio > 0.0f) {
								WinRatio = std::to_string(PlayerRankData.WinRatio);
							}
							else {
								WinRatio = "";  // 如果没有数据，则留空
							}
						}
						else {
							WinRatio = PlayerRankData.WinRatioToString;  // 确保格式正确
						}

						// 玩家类型和名单类型显示（支持中英文切换）
						std::string PlayerType = Languages == 1 ? U8("玩家") : "Player";
						std::string PlayerListType = Languages == 1 ? U8("默认") : "Default";
						ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 默认白色

						if (player.PartnerLevel > 0) {
							PlayerType = Languages == 1 ?
								U8("合作者Lv.") + std::to_string(player.PartnerLevel) :
								"Partner Lv." + std::to_string(player.PartnerLevel);
						}
						else if (player.IsSelf) {
							PlayerType = Languages == 1 ? U8("本人") : "Self";
							textColor = ImVec4(0.8f, 0.3f, 1.0f, 1.0f);
						}
						else if (player.IsMyTeam) {
							PlayerType = Languages == 1 ? U8("队友") : "Teammate";
							textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
						}

						if (player.StatusType == 12) {
							PlayerType = Languages == 1 ? U8("人机") : "Bot";
							textColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
						}

						if (player.ListType == 1) {
							PlayerListType = Languages == 1 ? U8("黑名单") : "Blacklist";
							textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						}
						else if (player.ListType == 2) {
							PlayerListType = Languages == 1 ? U8("白名单") : "Whitelist";
							textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
						}

						double KDAValue = 0.0;
						try {
							KDAValue = std::stod(PlayerRankData.KDAToString);  // 字符串转 double
						}
						catch (...) {
							KDAValue = 0.0; // 如果转换失败，默认当作 0
						}

						// 仅当是“普通玩家”时判断危险等级
						bool isDefaultPlayer = (Languages == 1 && PlayerType == U8("玩家")) ||
							(Languages != 1 && PlayerType == "Player");

						if (isDefaultPlayer) {
							if (PlayerRankData.TierToString == U8("大师1") && KDAValue > 2.9) {
								PlayerType = Languages == 1 ? U8("老挂逼") : "Perma Cheat";
								textColor = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
							}
							else if (PlayerRankData.TierToString == U8("大师1") || PlayerRankData.TierToString == U8("钻石1") || PlayerRankData.TierToString == U8("钻石2") || PlayerRankData.TierToString == U8("钻石3")) {
								PlayerType = Languages == 1 ? U8("挂逼") : "Cheater";
								textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
							}
							else if (KDAValue > 2.00) {
								PlayerType = Languages == 1 ? U8("挂逼") : "Cheater";
								textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
							}
							else if (KDAValue < 1.00 && KDAValue > 0.0) {
								PlayerType = Languages == 1 ? U8("菜逼") : "Noob";
								textColor = ImVec4(0.3f, 0.8f, 1.0f, 1.0f);
							}
						}

						if (player.ListType == 1) {
							textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						}
						else if (player.ListType == 2) {
							textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
						}


						if ((!filterPlayerListIndex && !filterPlayerTypeIndex && !filterPlayerName[0]) || (filterPlayerListIndex > 0 && strstr(filterPlayerList[filterPlayerListIndex], PlayerListType.c_str())) || (filterPlayerTypeIndex > 0 && strstr(PlayerType.c_str(), filterPlayerType[filterPlayerTypeIndex])) || (filterPlayerName[0] && strstr(player.PlayerName.c_str(), filterPlayerName) != nullptr)) {
							ImGui::TableNextRow(0);
							std::string Space = "  ";
							ImGui::TableSetColumnIndex(0);

							// 定义颜色数组（按顺序对应TeamID）
							ImColor teamColors[100] = {
								ImColor(247, 248, 19),    ImColor(250, 127, 73),    ImColor(90, 198, 227),
								ImColor(90, 189, 77),     ImColor(225, 99, 120),    ImColor(115, 129, 168),
								ImColor(159, 126, 105),   ImColor(255, 134, 200),   ImColor(210, 224, 191),
								ImColor(154, 52, 142),    ImColor(98, 146, 158),    ImColor(226, 214, 239),
								ImColor(4, 167, 119),     ImColor(115, 113, 252),   ImColor(255, 0, 255),
								ImColor(93, 46, 140),     ImColor(0, 255, 0),       ImColor(0, 0, 255),
								ImColor(255, 165, 0),     ImColor(128, 0, 128),     ImColor(255, 192, 203),
								ImColor(128, 128, 0),     ImColor(255, 215, 0),     ImColor(75, 0, 130),
								ImColor(0, 191, 255),     ImColor(255, 105, 180),   ImColor(139, 69, 19),
								ImColor(220, 20, 60),     ImColor(0, 255, 127),     ImColor(0, 250, 154),
								ImColor(72, 61, 139),     ImColor(143, 188, 143),   ImColor(178, 34, 34),
								ImColor(153, 50, 204),    ImColor(233, 150, 122),   ImColor(148, 0, 211),
								ImColor(95, 158, 160),    ImColor(127, 255, 212),   ImColor(218, 112, 214),
								ImColor(244, 164, 96),    ImColor(210, 105, 30),    ImColor(222, 184, 135),
								ImColor(255, 228, 181),   ImColor(255, 239, 213),   ImColor(175, 238, 238),
								ImColor(100, 149, 237),   ImColor(219, 112, 147),   ImColor(173, 216, 230),
								ImColor(240, 128, 128),   ImColor(255, 248, 220),   ImColor(255, 99, 71),
								ImColor(255, 140, 0),     ImColor(154, 205, 50),    ImColor(34, 139, 34),
								ImColor(70, 130, 180),    ImColor(123, 104, 238),   ImColor(255, 20, 147),
								ImColor(65, 105, 225),    ImColor(240, 230, 140),   ImColor(0, 128, 128),
								ImColor(216, 191, 216),   ImColor(176, 224, 230),   ImColor(0, 100, 0),
								ImColor(255, 69, 0),      ImColor(255, 215, 0),     ImColor(75, 0, 130),
								ImColor(199, 21, 133),    ImColor(0, 206, 209),     ImColor(128, 0, 0),
								ImColor(135, 206, 235),   ImColor(255, 228, 225),   ImColor(210, 180, 140),
								ImColor(32, 178, 170),    ImColor(95, 158, 160),    ImColor(255, 105, 180),
								ImColor(244, 164, 96),    ImColor(0, 255, 255),     ImColor(255, 250, 205),
								ImColor(138, 43, 226),    ImColor(124, 252, 0),     ImColor(255, 192, 203),
								ImColor(176, 196, 222),   ImColor(210, 105, 30),    ImColor(135, 206, 250),
								ImColor(152, 251, 152),   ImColor(221, 160, 221),   ImColor(255, 69, 0),
								ImColor(50, 205, 50),     ImColor(0, 0, 139),       ImColor(255, 20, 147),
								ImColor(72, 209, 204),    ImColor(139, 0, 139),     ImColor(255, 215, 0),
								ImColor(0, 191, 255),     ImColor(34, 139, 34),     ImColor(218, 112, 214),
								ImColor(255, 99, 71),     ImColor(135, 206, 250),   ImColor(255, 182, 193),
							};

							ImColor circleColor = player.TeamID < IM_ARRAYSIZE(teamColors) ? teamColors[player.TeamID] : ImColor(255, 255, 255);

							// 获取绘制列表和位置
							ImDrawList* draw_list = ImGui::GetWindowDrawList();
							ImVec2 pos = ImGui::GetCursorScreenPos();
							float radius = 10.0f; // 圆圈半径

							// 显示的文本内容，超过100显示"AI"
							std::string text;
							if (player.TeamID <= 100)
								text = std::to_string(player.TeamID);
							else
								text = "AI";

							// 绘制彩色圆圈
							draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius), radius, circleColor);

							ImFont* font = ImGui::GetFont();
							float ascent = font->Ascent;
							float descent = font->Descent;

							ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
							ImVec2 circle_center(pos.x + radius, pos.y + radius);
							float y_pos = circle_center.y - (ascent - descent) * 0.5f;
							ImVec2 text_pos(circle_center.x - text_size.x * 0.5f, y_pos);

							// 黑色描边
							for (int dx = -1; dx <= 1; ++dx)
							{
								for (int dy = -1; dy <= 1; ++dy)
								{
									if (dx == 0 && dy == 0) continue;
									draw_list->AddText(ImVec2(text_pos.x + dx, text_pos.y + dy), IM_COL32(0, 0, 0, 255), text.c_str());
								}
							}
							// 白色文字
							draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), text.c_str());

							ImGui::TableSetColumnIndex(1);
							ImGui::TextUnformatted(PlayerName.c_str());
							ImGui::TableSetColumnIndex(2);

							if (!PlayerRankData.TierToString.empty()) {
								// 构造段位图标路径
								std::string IconUrl = "Assets/image/RankImage/" + PlayerRankData.Tier + "-" + PlayerRankData.SubTier + ".png";
								if (PlayerRankData.Tier == "Master")
									IconUrl = "Assets/image/RankImage/" + PlayerRankData.Tier + ".png";
								if (PlayerRankData.Tier.empty())
									IconUrl = "Assets/image/RankImage/Unranked.png";

								// 确保图标存在并加载成功
								if (GImGuiTextureMap[IconUrl].Width > 0) {
									ImGui::Image((void*)GImGuiTextureMap[IconUrl].Texture, ImVec2(20, 20)); // 调整图标大小
									ImGui::SameLine(); // 让图标和文字保持在同一行
								}
							}

							ImGui::TextUnformatted(RankTier.c_str());
							ImGui::TableSetColumnIndex(3);
							ImGui::TextUnformatted(RankKDA.c_str());
							ImGui::TableSetColumnIndex(4);
							ImGui::TextUnformatted(RankPoint.c_str());
							ImGui::TableSetColumnIndex(5);
							ImGui::TextUnformatted(WinRatio.c_str());
							ImGui::TableSetColumnIndex(6);
							ImGui::TextUnformatted(std::to_string(player.KillCount).c_str());
							ImGui::TableSetColumnIndex(7);
							ImGui::TextUnformatted(std::to_string((int)player.DamageDealtOnEnemy).c_str());
							ImGui::TableSetColumnIndex(8);
							ImGui::TextUnformatted(std::to_string(player.PubgIdData.SurvivalLevel).c_str());
							ImGui::TableSetColumnIndex(9);
							ImGui::TextColored(textColor, "%s", PlayerType.c_str());
							if (ImGui::TableSetColumnIndex(10)) {
								auto NeedPopStyleColor = false;
								if (player.ListType == 1) {
									ImGui::PushStyleColor(ImGuiCol_Button, Utils::FloatToImColor(GameData.Config.ESP.Color.Blacklist.Info));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Utils::FloatToImColor(GameData.Config.ESP.Color.Blacklist.Info));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, Utils::FloatToImColor(GameData.Config.ESP.Color.Blacklist.Info));
									NeedPopStyleColor = true;
								}
								else if (player.ListType == 2) {
									ImGui::PushStyleColor(ImGuiCol_Button, Utils::FloatToImColor(GameData.Config.ESP.Color.Whitelist.Info));
									ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Utils::FloatToImColor(GameData.Config.ESP.Color.Whitelist.Info));
									ImGui::PushStyleColor(ImGuiCol_ButtonActive, Utils::FloatToImColor(GameData.Config.ESP.Color.Whitelist.Info));
									NeedPopStyleColor = true;
								}

								ImGui::PushID((int)player.pPlayerInfo);
								ImVec2 buttonSize(80.0f, 20.0f); // 自定义按钮大小
								if (ImGui::Button(PlayerListType.c_str(), buttonSize)) {
									if (player.ListType >= 2) {
										player.ListType = 0;
									}
									else {
										player.ListType++;
									}
									if (player.ListType == 1) {

										Players::AddToBlackList(player.AccountId);
									}
									else if (player.ListType == 2) {
										Players::AddToWhiteList(player.AccountId);
									}
									else {
										Players::RemoveList(player.AccountId);
									}

									if (player.ListType != PlayerLists[player.PlayerName].ListType)
									{
										PlayerLists[player.PlayerName].ListType = player.ListType;
										Data::SetPlayerListsItem(player.PlayerName, PlayerLists[player.PlayerName]);
									}

								}

								if (NeedPopStyleColor)
								{
									ImGui::PopStyleColor(3);
								}

								ImGui::PopID();
							}
						}
					}
				}

				ImGui::EndTable();
			}

			ImGui::End();
		}
	}
	static void UpdateModelSelection(int selectedModel) {
		GameData.Config.ESP.LowModel = (selectedModel == 0);
		GameData.Config.ESP.MediumModel = (selectedModel == 1);
		GameData.Config.ESP.HighModel = (selectedModel == 2);
	}

#include "MenuPlutoEsp.inl"

	static void newmenu(HWND hwnd) {

		ImGuiStyle* style = &ImGui::GetStyle();
		MenuTheme::ApplyPluto();

		GameData.Config.Menu.UiFontSize = ImClamp(GameData.Config.Menu.UiFontSize, 16.f, 22.f);
		MenuTheme::ApplyUiScale(style, GameData.Config.Menu.UiFontSize);
		Languages = GameData.Config.Project.CurrentLanguage ? 1 : 0;
		if (font::menu_ui) ImGui::PushFont(font::menu_ui);
		else if (default_font) ImGui::PushFont(default_font);

		style->WindowPadding = ImVec2(0, 0);
		style->WindowBorderSize = 0;
		style->ScrollbarSize = 8.f;
		style->FrameRounding = 2.f;
		style->GrabRounding = 2.f;

		ImGui::GetStyle().Colors[ImGuiCol_Text] = ImColor(235, 240, 250, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TableRowBg] = ImColor(255, 255, 255, 255);
		ImGui::GetStyle().Colors[ImGuiCol_TableRowBgAlt] = ImColor(245, 245, 245, 255);

		ImGui::SetNextWindowSize(MenuTheme::WindowSize, ImGuiCond_Always);

		ImGui::Begin("Menu", nullptr, window_flags);
		{
			const ImVec2& pos = ImGui::GetWindowPos();
			const ImVec2& region = ImGui::GetContentRegionMax();
			const ImVec2& spacing = style->ItemSpacing;
			const bool hasSubTabs = (tabs >= 2 && tabs <= 4);
			const ImVec2 contentSize = MenuTheme::ContentSize(region, hasSubTabs);
			const ImVec2 colSize = MenuTheme::ColumnSize(region, 3, hasSubTabs);
			const ImVec2 col4 = MenuTheme::ColumnSizeFromContent(contentSize, 4);
			const ImVec2 colMainEsp = MenuTheme::ColumnSize(region, 3, false);
			const ImVec2 col2w = ImVec2((contentSize.x - MenuTheme::ColumnGap) * 0.5f, contentSize.y);
			const ImVec2 col2a = ImVec2(contentSize.x * 0.40f, contentSize.y);
			const ImVec2 col2b = ImVec2(contentSize.x - col2a.x - MenuTheme::ColumnGap, contentSize.y);
			menuRect.left = (LONG)pos.x;
			menuRect.right = (LONG)(pos.x + MenuTheme::WindowSize.x);
			menuRect.top = (LONG)pos.y;
			menuRect.bottom = (LONG)(pos.y + MenuTheme::WindowSize.y);

			MenuTheme::DrawChrome(GetBackgroundDrawList(), pos, region);

			// 窗口控制按钮（顶栏右侧）
			PushFont(font::myth_bold);
			SetCursorPos(ImVec2(region.x - 60.f, 4.f));
			BeginGroup();
			if (ImGui::Selectable(U8("—"), false, 0, ImVec2(18, 18))) {
				GameData.Config.Menu.Show = !GameData.Config.Menu.Show;
				SetForegroundWindow(GameData.Config.Menu.Show ? GameData.Config.Overlay.hWnd : GetDesktopWindow());
			}
			ImGui::SameLine();
			if (ImGui::Selectable(U8("X"), false, 0, ImVec2(18, 18))) {
				HWND Progman = FindWindowA("Progman", NULL);
				HWND TrayWnd = FindWindowA("Shell_TrayWnd", NULL);
				if (Progman) ShowWindow(Progman, SW_SHOW);
				if (TrayWnd) ShowWindow(TrayWnd, SW_SHOW);
				TerminateProcess(GetCurrentProcess(), 1);
			}
			EndGroup();
			PopFont();

			// 顶部导航栏
			ImGui::SetCursorPos(ImVec2(8.f, MenuTheme::TitleHeight + 1.f));
			
			// 主标签按钮
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.f, 8.f));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.f, 0.f));
			const int mainTab = (tabs <= 1) ? tabs : (tabs == 5 ? 3 : 2);
			for (int i = 0; i < 4; ++i) {
				if (ImGui::Tabs(mainTab == i, "", GetMainTabTitle(i))) {
					if (i == 0) tabs = 0;
					else if (i == 1) tabs = 1;
					else if (i == 2) { tabs = 2; items_sub_tab = 0; }
					else tabs = 5;
				}
				if (i < 3) ImGui::SameLine(0, 2.f);
			}
			ImGui::PopStyleVar(2);

			// ============ 右侧永久设置面板 ============
			{
				const float sidebarX = pos.x + region.x - MenuTheme::SidebarWidth - 4.f;
				const float sidebarY = pos.y + MenuTheme::TitleHeight + MenuTheme::TopNavHeight + 4.f;
				const float sidebarH = region.y - MenuTheme::TitleHeight - MenuTheme::TopNavHeight - MenuTheme::FooterHeight - 12.f;
				ImGui::SetNextWindowPos(ImVec2(sidebarX, sidebarY));
				ImGui::SetNextWindowSize(ImVec2(MenuTheme::SidebarWidth, sidebarH));
			ImGui::Begin(Languages == 1 ? U8("##设置面板") : "##SettingsPanel", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
				{
					ImGui::BeginChild(false, "##sidebar", "o", ImVec2(0.f, 0.f), true, MenuTheme::ScrollPanelFlags());
					// 语言切换
					ImGui::TextColoredWishFont(font::calibri_bold_hint, ImVec4(0.00f, 0.82f, 0.94f, 1.00f), Languages == 1 ? U8("语言切换") : "Language");
					const char* langItems[] = { "English (EN)", U8("中文 (CN)") };
					int langIdx = Languages ? 1 : 0;
					ImGui::SetNextItemWidth(-1);
					if (ImGui::Combo_popup("##langSide", &langIdx, langItems, IM_ARRAYSIZE(langItems))) {
						Languages = langIdx ? 1 : 0;
						GameData.Config.Project.CurrentLanguage = Languages;
					}
					ImGui::Spacing();

				// 快捷开关
				ImGui::TextColoredWishFont(font::calibri_bold_hint, ImVec4(0.00f, 0.82f, 0.94f, 1.00f), Languages == 1 ? U8("快捷开关") : "Quick Toggles");
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 4.f));
				ImGui::CheckboxWishTips(Languages == 1 ? U8("数据") : "Data", &GameData.Config.ESP.DataSwitch, "Draw Data");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("FPS") : "FPS", &GameData.Config.Overlay.ShowFPS, "Show FPS");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("主机鼠标") : "Host Mouse", &GameData.Config.ESP.Mouse, "Host Mouse");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("打开列表窗") : "Player Window", &GameData.Config.Window.Players, "Player Window");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("垂直同步") : "VSync", &GameData.Config.Overlay.VSync, "VSync");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("独立线程") : "Thread", &GameData.Config.Overlay.UseThread, "Thread");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("备用相机") : "Backup Cam", &GameData.Config.Overlay.UseLastFrameCameraCache, "Backup Cam");
				ImGui::PopStyleVar();
				ImGui::Separator();

					// 热键设置
					ImGui::TextColoredWishFont(font::calibri_bold_hint, ImVec4(0.00f, 0.82f, 0.94f, 1.00f), Languages == 1 ? U8("热键设置") : "Hotkeys");
					ImGui::Keybind(Languages == 1 ? U8("显隐菜单") : "Toggle Menu", &GameData.Config.Menu.ShowKey);
					ImGui::Keybind(Languages == 1 ? U8("安全退出") : "Safe Exit", &GameData.Config.Overlay.Quit_key);
					ImGui::Keybind(Languages == 1 ? U8("融合模式") : "Fusion", &GameData.Config.Overlay.ModeKey);
					ImGui::Keybind(Languages == 1 ? U8("自瞄开关") : "Aimbot Toggle", &GameData.Config.ESP.AimbotHotkey);
					ImGui::Keybind(Languages == 1 ? U8("玩家列表") : "Player List", &GameData.Config.ESP.Playerskey);
					ImGui::Keybind(Languages == 1 ? U8("刷新缓存") : "Refresh Cache", &GameData.Config.Function.ClearKey);
					ImGui::Keybind(Languages == 1 ? U8("战斗模式") : "Combat Mode", &GameData.Config.ESP.FocusModeKey);
					ImGui::Keybind(Languages == 1 ? U8("获取模型") : "Obtain Model", &GameData.Config.Overlay.ObtainModel);
					ImGui::Keybind(Languages == 1 ? U8("重载模型") : "Reload Model", &GameData.Config.Overlay.ReloadModel);
					ImGui::Separator();

					// 菜单设置
					ImGui::TextColoredWishFont(font::calibri_bold_hint, ImVec4(0.00f, 0.82f, 0.94f, 1.00f), Languages == 1 ? U8("菜单设置") : "Menu Settings");
					ImGui::SliderFloat1(Languages == 1 ? U8("菜单字号") : "Menu Font", &GameData.Config.Menu.UiFontSize, 16.f, 22.f, "%.0f px");
					ImGui::Text("%s: %dx%d", Languages == 1 ? U8("分辨率") : "Resolution",
						GameData.Config.Overlay.ScreenWidth, GameData.Config.Overlay.ScreenHeight);
					const char* configItems[] = { Languages == 1 ? U8("配置1") : "Profile1", Languages == 1 ? U8("配置2") : "Profile2" };
					ImGui::SetNextItemWidth(-1);
					ImGui::Combo_popup(Languages == 1 ? U8("自瞄配置") : "AimBot Config", &GameData.Config.AimBot.ConfigIndex, configItems, IM_ARRAYSIZE(configItems));
					if (ImGui::Button(Languages == 1 ? U8("读取配置 F2") : "Load F2", ImVec2(-1, 0.f))) Config::Load();
					if (ImGui::Button(Languages == 1 ? U8("保存配置") : "Save", ImVec2(-1, 0.f))) Config::Save();
					ImGui::EndChild();
				}
				ImGui::End();
			}
			if (tabs >= 2 && tabs <= 4) {
				const float subTabY = MenuTheme::TitleHeight + MenuTheme::TopNavHeight + 2.f;
				ImGui::SetCursorPos(ImVec2(12.f, subTabY));
				BeginGroup();
				for (int j = 0; j < 3; ++j) {
					if (ImGui::SubTabs(font::calibri_bold, items_sub_tab == j, GetItemsSubTabTitle(j), ImVec2(96.f, 26.f))) {
						items_sub_tab = j;
						tabs = 2 + j;
					}
					if (j < 2) ImGui::SameLine(0, 4.f);
				}
				EndGroup();
			}
			tab_alpha = ImLerp(tab_alpha, (tabs == active_tab) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
			if (tab_alpha < 0.01f && tab_add < 0.01f) {
				active_tab = tabs;
				if (active_tab >= 2 && active_tab <= 4) items_sub_tab = active_tab - 2;
			}

			anim = ImLerp(anim, (tabs == active_tab) ? 1.f : 0.f, 1);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);
			{

				if (active_tab == 0)
				{

					ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, hasSubTabs));
					ImGui::BeginChild(false, "Child", "o", contentSize, false, MenuTheme::ScrollPanelFlags());
					{
						RenderPlutoMainEsp(contentSize, colMainEsp, spacing);
					}
					ImGui::EndChild();
				}
				else  if (active_tab == 1)
				{
					const ImVec2 aimbotContent = MenuTheme::AimbotContentSize(region);
					const ImVec2 aimbotCol2 = ImVec2((aimbotContent.x - MenuTheme::ColumnGap) * 0.5f, aimbotContent.y);

					ImGui::SetCursorPos(ImVec2(12.f, MenuTheme::ContentStartY(false) + 4.f));
					ImGui::BeginGroup();
					{
						std::vector<std::vector<std::string>> tabs_section_text = {
							{"D", "F", "E", "G", "H", "C", "B", "M", "A"},
						};
						static int tabs_sect;

						// 计算所有 SubTabs 选项的总宽度
						float total_width = 0.0f;
						float button_width = 100.0f; // 每个按钮宽度
						float button_spacing = 10.0f; // 按钮间距
						int num_buttons = tabs_section_text[0].size();

						total_width = num_buttons * button_width + (num_buttons - 1) * button_spacing;

						// 计算起始 X 坐标，使整个组居中
						float available_width = contentSize.x;
						float start_x = (available_width - total_width) * 0.5f;

						// 确保不会超出左侧边界
						ImGui::SetCursorPosX(12.f + (start_x > 0 ? start_x : 0.f));

						// 绘制 SubTabs
						for (int i = 0; i < num_buttons; i++)
						{
							if (ImGui::SubTabs(font::weapon_pubg, i == tabs_sect, tabs_section_text[0][i].c_str(), ImVec2(button_width, MenuTheme::WeaponTabRowHeight - 4.f)))
								tabs_sect = i;

							if (i < num_buttons - 1); // 避免最后一个元素多余的 SameLine
							ImGui::SameLine(0.0f, button_spacing);
						}

						if (tabs_sect < 8) {

							const std::string WeapType[8] = { "AR", "SR", "DMR", "LMG","SG",  "SMG" , "HG", "MELEE" };
							Setting(WeapType[tabs_sect]);

						}
						else if (tabs_sect == 8)
						{

							ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, false, MenuTheme::ContentStartY(false) + MenuTheme::WeaponTabRowHeight));

							ImGui::BeginChild(false, "ChildRecoil", "o", aimbotContent, false, MenuTheme::ScrollPanelFlags());
							ImGui::BeginGroup();

							ImGui::BeginChild(true, MenuLag("RecoilSettings", Languages).c_str(), "o", MenuTheme::StyledPanel(aimbotCol2.x, 9, 0), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("开启压枪") : "Enable Recoil Control", &GameData.Config.AimBot.Recoilenanlek, "Enable Recoil Control");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("增强压枪") : "Enhanced Recoil", &GameData.Config.AimBot.EnableEnhancedRecoil, "Enhanced Recoil");
								ImGui::CheckboxWishTips(Languages == 1 ? U8("独立Y轴") : "Y-Axis Only", &GameData.Config.AimBot.YDownEnable, "Y-Axis Only");
								ImGui::CheckboxWishTips(Languages == 1 ? U8("红点自适应") : "RedDot Adaptive", &GameData.Config.AimBot.RedDownEnable, "RedDot Adaptive");

								ImGui::SliderInt1(Languages == 1 ? U8("红点幅度") : "Red Dot Amplitude", &GameData.Config.AimBot.yRecoil[0], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("二倍幅度") : "2x Amplitude", &GameData.Config.AimBot.yRecoil[1], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("三倍幅度") : "3x Amplitude", &GameData.Config.AimBot.yRecoil[2], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("四倍幅度") : "4x Amplitude", &GameData.Config.AimBot.yRecoil[3], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("六倍幅度") : "6x Amplitude", &GameData.Config.AimBot.yRecoil[4], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("八倍幅度") : "8x Amplitude", &GameData.Config.AimBot.yRecoil[5], 1, 50, "%d");

							}
							ImGui::EndChild(true);

							ImGui::EndGroup();

							ImGui::SameLine();

							ImGui::BeginGroup();

							ImGui::BeginChild(true, MenuLag("RecoilDelay", Languages).c_str(), "o", MenuTheme::StyledPanel(aimbotCol2.x, 6, 0), false, MenuTheme::ScrollPanelFlags());
							{
								// ImGui::CheckboxWishTips(Languages == 1 ? U8("开启压枪") : "Enable Recoil Control", &GameData.Config.AimBot.Recoilenanlek);

								ImGui::SliderInt1(Languages == 1 ? U8("红点延迟") : "Red Dot Delay", &GameData.Config.AimBot.interval[0], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("二倍延迟") : "2x Delay", &GameData.Config.AimBot.interval[1], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("三倍延迟") : "3x Delay", &GameData.Config.AimBot.interval[2], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("四倍延迟") : "4x Delay", &GameData.Config.AimBot.interval[3], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("六倍延迟") : "6x Delay", &GameData.Config.AimBot.interval[4], 1, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("八倍延迟") : "8x Delay", &GameData.Config.AimBot.interval[5], 1, 50, "%d");

							}
							ImGui::EndChild(true);

							ImGui::EndGroup();
							ImGui::EndChild();

						}

					}
					ImGui::EndGroup();

				}
				else  if (active_tab == 2)
				{

					ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, true));
					ImGui::BeginChild(false, "Child", "o", contentSize, false, MenuTheme::ScrollPanelFlags());
					{

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Item Perspective", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 10, 3), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("物透开关") : "Item ESP Toggle", &GameData.Config.Item.Enable, "Item ESP Toggle");

								ImGui::Keybind(Languages == 1 ? U8("切换热键") : "Toggle Hotkey", &GameData.Config.Item.GroupKey);

								ImGui::Keybind(Languages == 1 ? U8("A组热键") : "Group A Hotkey", &GameData.Config.Item.GroupAKey);

								ImGui::Keybind(Languages == 1 ? U8("B组热键") : "Group B Hotkey", &GameData.Config.Item.GroupBKey);

								ImGui::Keybind(Languages == 1 ? U8("C组热键") : "Group C Hotkey", &GameData.Config.Item.GroupCKey);

								ImGui::Keybind(Languages == 1 ? U8("D组热键") : "Group D Hotkey", &GameData.Config.Item.GroupDKey);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示图标和文字") : "Show Icons And Text", &GameData.Config.Item.ShowIconAndText, "Show Icons And Text");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示图标") : "Show Icons", &GameData.Config.Item.ShowIcon, "Show Icons");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("叠加显示") : "Stack Display", &GameData.Config.Item.Combination, "Stack Display");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示射线") : "Show Ray", &GameData.Config.Item.ShowRay, "Show Ray");

								ImGui::SliderInt1(Languages == 1 ? U8("物透距离") : "Item ESP Distance", &GameData.Config.Item.DistanceMax, 0, 150, "%d M");

								ImGui::SliderInt1(Languages == 1 ? U8("字体大小") : "Font Size", &GameData.Config.Item.FontSize, 10, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("射线粗细") : "Ray Width", &GameData.Config.Item.RayWidth, 1, 10, "%d");

								//ImGui::Keybind(Languages == 1 ? U8("切换热键") : "Switch Hotkey", & GameData.Config.Vehicle.EnableKey);


							}
							ImGui::EndChild(true);

						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Airdrop Perspective", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 3, 3), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示空投") : "Show AirDrop", &GameData.Config.AirDrop.Enable, "Show AirDrop");

								ImGui::Keybind(Languages == 1 ? U8("空投热键") : "AirDrop Hotkey", &GameData.Config.AirDrop.EnableKey);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示物品") : "Show Items", &GameData.Config.AirDrop.ShowItems, "Show Items");

								ImGui::SliderInt1(Languages == 1 ? U8("显示距离") : "Show Distance", &GameData.Config.AirDrop.DistanceMax, 0, 1000, "%d M");

								ImGui::SliderInt1(Languages == 1 ? U8("字体大小") : "Font Size", &GameData.Config.AirDrop.FontSize, 10, 50, "%d");
							}
							ImGui::EndChild(true);

							ImGui::BeginChild(true, MenuLag("Vehicle Perspective", Languages).c_str(), "b", MenuTheme::StyledPanel(colSize.x, 5, 2), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示载具") : "Show Vehicle", &GameData.Config.Vehicle.Enable, "Show Vehicle");

								ImGui::Keybind(Languages == 1 ? U8("载具热键") : "Vehicle Hotkey", &GameData.Config.Vehicle.EnableKey);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("载具油量") : "Vehicle Fuel", &GameData.Config.Vehicle.Durability, "Vehicle Fuel");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("载具血量") : "Vehicle Health", &GameData.Config.Vehicle.Health, "Vehicle Health");

								ImGui::SliderInt1(Languages == 1 ? U8("显示距离") : "Show Distance", &GameData.Config.Vehicle.DistanceMax, 100, 1000, "%d M");

								ImGui::SliderInt1(Languages == 1 ? U8("字体大小") : "Font Size", &GameData.Config.Vehicle.FontSize, 10, 50, "%d");

							}
							ImGui::EndChild(true);
						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Box Perspective", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 3, 2), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示盒子") : "Show Loot Box", &GameData.Config.DeadBox.Enable, "Show Loot Box");

								ImGui::Keybind(Languages == 1 ? U8("盒子热键") : "Loot Box Hotkey", &GameData.Config.DeadBox.EnableKey);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("显示物品") : "Show Items", &GameData.Config.DeadBox.ShowItems, "Show Items");

								ImGui::SliderInt1(Languages == 1 ? U8("显示距离") : "Show Distance", &GameData.Config.DeadBox.DistanceMax, 10, 200, "%d M");

								ImGui::SliderInt1(Languages == 1 ? U8("字体大小") : "Font Size", &GameData.Config.DeadBox.FontSize, 10, 50, "%d");



							}
							ImGui::EndChild(true);

							ImGui::BeginChild(true, MenuLag("Color Settings", Languages).c_str(), "b", MenuTheme::StyledPanel(colSize.x, 11, 0), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::ColorEdit5(Languages == 1 ? U8("A组颜色") : "Group A Color", GameData.Config.Item.GroupAColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("B组颜色") : "Group B Color", GameData.Config.Item.GroupBColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("C组颜色") : "Group C Color", GameData.Config.Item.GroupCColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("D组颜色") : "Group D Color", GameData.Config.Item.GroupDColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("射线颜色") : "Ray Color", GameData.Config.Item.RayColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("空投颜色") : "AirDrop Color", GameData.Config.AirDrop.Color, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("载具颜色") : "Vehicle Color", GameData.Config.Vehicle.Color, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("盒子颜色") : "Loot Box Color", GameData.Config.DeadBox.Color, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("载具油量") : "Vehicle Fuel", GameData.Config.Vehicle.Fuelbarcolor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("载具血量") : "Vehicle Health", GameData.Config.Vehicle.Healthbarcolor, picker_flags);
							}
							ImGui::EndChild(true);
						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{
							ImGui::BeginChild(true, MenuLag("Smart Display", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 3, 14), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("装备配件屏蔽相同配件") : "Hide Duplicate Attachments", &GameData.Config.Item.AccessoriesFilter, "Hide Duplicate Attachments");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("屏蔽手上持有武器显示") : "Hide Equipped Weapons", &GameData.Config.Item.HandheldWeaponFilter, "Hide Equipped Weapons");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("满耐久头甲屏蔽同头甲") : "Hide Full Durability Gear", &GameData.Config.Item.FilterHelmets, "Hide Full Durability Gear");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("背包物品达到上限不显") : "Hide Full Backpack Items", &GameData.Config.Item.ItemLimit, "Hide Full Backpack Items");

								ImGui::SliderInt1(Languages == 1 ? U8("绷带") : "Bandage", &GameData.Config.ItemFiltering.Bandage, 0, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("急救包") : "First Aid Kit", &GameData.Config.ItemFiltering.FirstAidKit, 0, 30, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("医疗箱") : "Med Kit", &GameData.Config.ItemFiltering.MedicalKit, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("止痛药") : "Painkiller", &GameData.Config.ItemFiltering.Painkiller, 0, 30, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("能量饮料") : "Energy Drink", &GameData.Config.ItemFiltering.EnergyDrink, 0, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("肾上腺素") : "Adrenaline Syringe", &GameData.Config.ItemFiltering.Adrenaline, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("C4炸药") : "C4", &GameData.Config.ItemFiltering.C4, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("破片手雷") : "Grenade", &GameData.Config.ItemFiltering.Grenade, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("烟雾弹") : "Smoke Grenade", &GameData.Config.ItemFiltering.SmokeGrenade, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("闪光弹") : "Flashbang", &GameData.Config.ItemFiltering.Flashbang, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("燃烧瓶") : "Molotov", &GameData.Config.ItemFiltering.Molotov, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("蓝圈手雷") : "Blue Zone Grenade", &GameData.Config.ItemFiltering.BlueZoneGrenade, 0, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("粘性炸弹") : "Sticky Bomb", &GameData.Config.ItemFiltering.StickyBomb, 0, 20, "%d");

							}
							ImGui::EndChild(true);
						}
						ImGui::EndGroup();

					}
					ImGui::EndChild();
				}
				else  if (active_tab == 3)
				{
					ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, true));
					ImGui::BeginChild(false, "Child", "o", contentSize, false, MenuTheme::ScrollPanelFlags());
					{
						//static int checkbox;
						// 添加一个变量来跟踪当前选中的索引
						static int selectedIndex = 0; // -1 表示没有选中任何项
						static bool checkbox[14] = {
							true,   // 默认选中第一个（索引0）
							false,  // 其他初始化为未选中
							false,
							false,
							false,
							false,
							false,
							false,
							false,
							false,
							false,
							false,
							false,
							false
						};
						ImGui::BeginGroup();
						{
							ImGui::BeginChild(true, MenuLag("Container Filter", Languages).c_str(), "e", col2a, false, MenuTheme::ScrollPanelFlags());
							{


								ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 10.f));

								const int cols = 2;
								const float cellGap = 10.f;
								const float cellW = ImFloor((ImGui::GetContentRegionAvail().x - cellGap) / (float)cols);
								const ImVec2 cellSize(cellW, 104.f);
								const ImVec2 texSize(72.f, 72.f);
								/*static float sp = 0.0f;

								ImVec2 mousePos = ImGui::GetMousePos();

								ImGui::SetCursorPos(ImVec2(145, 120));
								ImGui::Hive_Checkbox_Radio(texture::bq, Languages == 1 ? U8("步枪") : "Rifle", &checkbox, 0, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("步枪") : "Rifle").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::sj, Languages == 1 ? U8("栓狙") : "Bolt Action Sniper", &checkbox, 1, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("栓狙") : "Bolt").x, 105));

								ImGui::SetCursorPosX(105);
								ImGui::Hive_Checkbox_Radio(texture::lj, Languages == 1 ? U8("连狙") : "Semi-Auto Sniper", &checkbox, 2, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("连狙") : "Semi").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::jq, Languages == 1 ? U8("机枪") : "Machine Gun", &checkbox, 3, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("机枪") : "MG").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::sdq, Languages == 1 ? U8("霰弹枪") : "Shotgun", &checkbox, 4, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("霰弹枪") : "Shotgun").x, 105));

								ImGui::SetCursorPosX(65);
								ImGui::Hive_Checkbox_Radio(texture::sq, Languages == 1 ? U8("手枪") : "Pistol", &checkbox, 5, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("手枪") : "Pistol").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::cfq, Languages == 1 ? U8("冲锋枪") : "SMG", &checkbox, 6, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("冲锋枪") : "SMG").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::pj, Languages == 1 ? U8("配件") : "Attachments", &checkbox, 7, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("配件") : "Attach").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::yp, Languages == 1 ? U8("药品") : "Medicals", &checkbox, 8, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("药品") : "Med").x, 105));

								ImGui::SetCursorPosX(105);
								ImGui::Hive_Checkbox_Radio(texture::fj, Languages == 1 ? U8("防具") : "Armor", &checkbox, 9, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("防具") : "Armor").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::zd, Languages == 1 ? U8("子弹") : "Ammo", &checkbox, 10, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("子弹") : "Ammo").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::tzw, Languages == 1 ? U8("投掷物") : "Throwables", &checkbox, 11, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("投掷物") : "Throw").x, 105));

								ImGui::SetCursorPosX(145);
								ImGui::Hive_Checkbox_Radio(texture::ys, Languages == 1 ? U8("钥匙") : "Keys", &checkbox, 12, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("钥匙") : "Keys").x, 105)); ImGui::SameLine(0, sp);
								ImGui::Hive_Checkbox_Radio(texture::item, Languages == 1 ? U8("其他") : "Others", &checkbox, 13, ImVec2(80, 80), 40, mousePos, pos + ImVec2(475 - CalcTextSize(Languages == 1 ? U8("其他") : "Other").x, 105));
								ImGui::PopStyleVar();*/
								//bool weapon_check_item[14];
								char weaponnamesearch[120] = { "" };
								std::string searchTerm = weaponnamesearch;


								std::vector<std::string> itemNames = {
									Languages == 1 ? U8("步枪") : "Rifle", Languages == 1 ? U8("栓狙") : "Bolt Action Sniper", Languages == 1 ? U8("连狙") : "Semi-Auto Sniper", Languages == 1 ? U8("机枪") : "Machine Gun", Languages == 1 ? U8("霰弹枪") : "Shotgun",
									Languages == 1 ? U8("手枪") : "Pistol", Languages == 1 ? U8("冲锋枪") : "SMG", Languages == 1 ? U8("配件") : "Attachments",
									Languages == 1 ? U8("药品") : "Medicals", Languages == 1 ? U8("防具") : "Armor",
									Languages == 1 ? U8("子弹") : "Ammo", Languages == 1 ? U8("投掷物") : "Throwables",
									Languages == 1 ? U8("钥匙") : "Keys", Languages == 1 ? U8("其他") : "Others"
								};

								ID3D11ShaderResourceView* pageShortcuts[14] = {
									texture::bq, texture::sj, texture::lj, texture::jq, texture::sdq,
									texture::sq, texture::cfq, texture::pj, texture::yp, texture::fj,
									texture::zd,texture::tzw,texture::ys,texture::item
								};

								int row = cols;
								int value[14] = { 5,5,4,4,3,3,2,2,1,1,1,1, 0,0 };
								for (int i = 0; i < itemNames.size(); ++i)
								{
									if (!searchTerm.empty() && itemNames[i].find(searchTerm) == std::string::npos)
									{
										continue;
									}

									// 临时保存当前项的选中状态
									bool wasSelected = checkbox[i];


									// 传递单元格尺寸给 Item_checkbox
									ImGui::Item_checkbox(
										texSize,
										pageShortcuts[i],
										itemNames[i].c_str(),
										&checkbox[i],
										cellSize,
										value[i]
									);

									// 检查选中状态是否变化
									if (checkbox[i] && !wasSelected)
									{
										// 如果当前项被选中，取消其他所有项的选中状态
										for (int j = 0; j < itemNames.size(); j++)
										{
											if (j != i) checkbox[j] = false;
										}

										// 更新选中的索引
										selectedIndex = i;
									}
									else if (!checkbox[i] && wasSelected)
									{
										// 如果当前项被取消选中，清除选中索引
										selectedIndex = -1;
									}

									if ((i + 1) % row != 0 && i < itemNames.size() - 1)
									{
										ImGui::SameLine(0.f, cellGap);
									}
								}

								ImGui::PopStyleVar();
							}
							ImGui::EndChild(true);

						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{
							ImGui::BeginChild(true, MenuLag("Item List", Languages).c_str(), "f", col2b, false, MenuTheme::ScrollPanelFlags());
							{

								const float tableH = ImGui::GetContentRegionAvail().y;
								const float catColW = ImMax(220.f, ImGui::GetContentRegionAvail().x - 52.f - 140.f);
								const ImGuiTableFlags tableFlags =
									ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersH
									| ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit;
								if (ImGui::BeginTable("SortableTable", 3, tableFlags, ImVec2(0.f, tableH)))
								{

									// 设置表格列（中英文双语）
									ImGui::TableSetupColumn(Languages == 1 ? U8("图像") : "Graphics", ImGuiTableColumnFlags_WidthFixed, 52.0f);
									ImGui::TableSetupColumn(Languages == 1 ? U8("物品名称") : "Item Name", ImGuiTableColumnFlags_WidthFixed, 140.0f);
									ImGui::TableSetupColumn(Languages == 1 ? U8("分类") : "Category", ImGuiTableColumnFlags_WidthFixed, catColW);
									ImGui::TableSetupScrollFreeze(0, 1);

									ImGui::TableHeadersRow();

									if (checkbox[0]) {
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::AR) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);

												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";
												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[1])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::SR) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[2])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::DMR) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[3])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::LMG) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[4])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::SG) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[5])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::HG) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[6])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::SMG) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[7])
									{
										const WeaponType typeOrder[] = {
											WeaponType::Muzzle, WeaponType::Sight, WeaponType::GunButt,
											WeaponType::Grip, WeaponType::Magazine
										};

										const char* typeNames[] = {
											Languages == 1 ? U8("枪口") : "Muzzle",
											Languages == 1 ? U8("瞄具") : "Sight",
											Languages == 1 ? U8("枪托") : "GunButt",
											Languages == 1 ? U8("握把") : "Grip",
											Languages == 1 ? U8("弹匣") : "Magazine"
										};

										size_t index = 0;

										for (int typeIdx = 0; typeIdx < 5; typeIdx++)
										{
											WeaponType currentType = typeOrder[typeIdx];
											bool hasItems = false;

											for (auto& pair : GameData.Config.Item.Lists) {
												ItemDetail& detail = pair.second;
												if (detail.Type == currentType) {
													hasItems = true;
													break;
												}
											}

											if (hasItems) {
												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 2);
												ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.1f, 1.0f), typeNames[typeIdx]);
												ImGui::TableNextColumn();
												ImGui::Text("");
												ImGui::TableNextColumn();
												ImGui::Text("");

												for (auto& pair : GameData.Config.Item.Lists) {
													const std::string& ItemName = pair.first;
													ItemDetail& detail = pair.second;
													if (detail.Type == currentType) {
														std::string displayName = Utils::StringToUTF8(detail.DisplayName);
														std::string name = displayName;

														ImGui::PushID(index);

														std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

														ImGui::TableNextRow();
														ImGui::TableNextColumn();
														ImGui::SetCursorPosY(GetCursorPosY());
														ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
														ImGui::TableNextColumn();
														ImGui::SetCursorPosY(GetCursorPosY() + 5);
														ImGui::Text(name.c_str());
														ImGui::TableNextColumn();

														const char* groupItems[] = {
															Languages == 1 ? U8("未选择") : "Unselected",
															Languages == 1 ? U8("分组A") : "Group A",
															Languages == 1 ? U8("分组B") : "Group B",
															Languages == 1 ? U8("分组C") : "Group C",
															Languages == 1 ? U8("分组D") : "Group D"
														};

														ImGui::SetCursorPosY(GetCursorPosY() - 25);
														ImGui::SetNextItemWidth(200);
														ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

														ImGui::SameLine();
														ImGui::SetCursorPosY(GetCursorPosY());
														ImGui::SetNextItemWidth(150);

														const char* rayItems[] = {
															Languages == 1 ? U8("不显射线") : "Hide Ray",
															Languages == 1 ? U8("显示射线") : "Show Ray"
														};

														int raySelection = detail.ShowRay ? 1 : 0;

														if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
															detail.ShowRay = (raySelection == 1);
														}

														ImGui::PopID();
														++index;
													}
												}
											}
										}
									}
									else if (checkbox[8])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::Drug) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[9])
									{
										size_t index = 0;

										std::vector<std::pair<std::string, ItemDetail*>> sortedArmor;

										for (auto& pair : GameData.Config.Item.Lists) {
											if (pair.second.Type == WeaponType::Armor) {
												sortedArmor.push_back({ pair.first, &pair.second });
											}
										}

										std::sort(sortedArmor.begin(), sortedArmor.end(),
											[](const auto& a, const auto& b) {
												return Utils::StringToUTF8(a.second->DisplayName) < Utils::StringToUTF8(b.second->DisplayName);
											});

										for (auto& item : sortedArmor) {
											const std::string& ItemName = item.first;
											ItemDetail& detail = *item.second;
											std::string displayName = Utils::StringToUTF8(detail.DisplayName);
											std::string name = displayName;

											ImGui::PushID(index);

											std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

											ImGui::TableNextRow();
											ImGui::TableNextColumn();
											ImGui::SetCursorPosY(GetCursorPosY());
											ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
											ImGui::TableNextColumn();
											ImGui::SetCursorPosY(GetCursorPosY() + 5);
											ImGui::Text(name.c_str());
											ImGui::TableNextColumn();

											const char* groupItems[] = {
												Languages == 1 ? U8("未选择") : "Unselected",
												Languages == 1 ? U8("分组A") : "Group A",
												Languages == 1 ? U8("分组B") : "Group B",
												Languages == 1 ? U8("分组C") : "Group C",
												Languages == 1 ? U8("分组D") : "Group D"
											};

											ImGui::SetCursorPosY(GetCursorPosY() - 25);
											ImGui::SetNextItemWidth(200);
											ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

											ImGui::SameLine();
											ImGui::SetCursorPosY(GetCursorPosY());
											ImGui::SetNextItemWidth(150);

											const char* rayItems[] = {
												Languages == 1 ? U8("不显射线") : "Hide Ray",
												Languages == 1 ? U8("显示射线") : "Show Ray"
											};

											int raySelection = detail.ShowRay ? 1 : 0;

											if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
												detail.ShowRay = (raySelection == 1);
											}

											ImGui::PopID();
											++index;
										}
									}
									else if (checkbox[10])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::Bullet) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[11])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::Grenade) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[12])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::key) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}
									else if (checkbox[13])
									{
										size_t index = 0;
										for (auto& pair : GameData.Config.Item.Lists) {
											const std::string& ItemName = pair.first;
											ItemDetail& detail = pair.second;
											if (detail.Type == WeaponType::Other) {
												std::string displayName = Utils::StringToUTF8(detail.DisplayName);
												std::string name = displayName;
												ImGui::PushID(index);
												// 图标路径
												std::string IconUrl = "Assets/image/All/" + ItemName + ".png";

												ImGui::TableNextRow();
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::Image(GImGuiTextureMap[IconUrl].Texture, ImVec2(45, 45));
												ImGui::TableNextColumn();
												ImGui::SetCursorPosY(GetCursorPosY() + 5);
												ImGui::Text(name.c_str());
												ImGui::TableNextColumn();

												// 第一行：分组选择
												const char* groupItems[] = {
													Languages == 1 ? U8("未选择") : "Unselected",
													Languages == 1 ? U8("分组A") : "Group A",
													Languages == 1 ? U8("分组B") : "Group B",
													Languages == 1 ? U8("分组C") : "Group C",
													Languages == 1 ? U8("分组D") : "Group D"
												};
												ImGui::SetCursorPosY(GetCursorPosY() - 25);
												ImGui::SetNextItemWidth(200);  // 减小宽度为两个控件留空间
												ImGui::Combo_popup(("##Group" + std::to_string(index)).c_str(), &detail.Group, groupItems, IM_ARRAYSIZE(groupItems));

												// 第二行：射线显示选择
												ImGui::SameLine();
												ImGui::SetCursorPosY(GetCursorPosY());
												ImGui::SetNextItemWidth(150);

												// 射线显示选项
												const char* rayItems[] = {
													Languages == 1 ? U8("不显射线") : "Hide Ray",
													Languages == 1 ? U8("显示射线") : "Show Ray"
												};

												// 将bool转换为int (0 = 不显示, 1 = 显示)
												int raySelection = detail.ShowRay ? 1 : 0;

												if (ImGui::Combo_popup(("##Ray" + std::to_string(index)).c_str(), &raySelection, rayItems, IM_ARRAYSIZE(rayItems))) {
													detail.ShowRay = (raySelection == 1);
												}

												ImGui::PopID();
												++index;
											}
										}
									}

									ImGui::EndTable();
								}

							}
							ImGui::EndChild(true);


						}
						ImGui::EndGroup();



					}
					ImGui::EndChild();
				}
				else  if (active_tab == 4)
				{

					ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, true));
					ImGui::BeginChild(false, "Child", "o", contentSize, false, MenuTheme::ScrollPanelFlags());
					{
						ImGui::BeginGroup();
						{
							ImGui::BeginChild(true, MenuLag("Big Map Radar", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 5, 1), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("大地图玩家") : "Map Player", &GameData.Config.Radar.Main.ShowPlayer, "Map Player");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("大地图载具") : "Map Vehicle", &GameData.Config.Radar.Main.ShowVehicle, "Map Vehicle");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("大地图空投") : "Map Airdrop", &GameData.Config.Radar.Main.ShowAirDrop, "Map Airdrop");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("大地图死亡") : "Map Death", &GameData.Config.Radar.Main.ShowDeadBox, "Map Death");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("大地图密室") : "Secret Room", &GameData.Config.Radar.Main.MapRoom, "Secret Room");

								ImGui::SliderInt1(Languages == 1 ? U8("大地图图标比例") : "Map Icon Scale", &GameData.Config.Radar.Main.FontSize, 10, 50, "%d");

							}
							ImGui::EndChild(true);

						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Mini Map Radar", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 5, 1), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("小地图玩家") : "MiniMap Player", &GameData.Config.Radar.Mini.ShowPlayer, "MiniMap Player");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("小地图载具") : "MiniMap Vehicle", &GameData.Config.Radar.Mini.ShowVehicle, "MiniMap Vehicle");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("小地图空投") : "MiniMap Airdrop", &GameData.Config.Radar.Mini.ShowAirDrop, "MiniMap Airdrop");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("小地图死亡") : "MiniMap Death", &GameData.Config.Radar.Mini.ShowDeadBox, "MiniMap Death");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("小地图密室") : "MiniMap Secret Room", &GameData.Config.Radar.Mini.MapRoom, "MiniMap Secret Room");

								ImGui::SliderInt1(Languages == 1 ? U8("小地图图标比例") : "MiniMap Icon Scale", &GameData.Config.Radar.Mini.FontSize, 10, 50, "%d");
							}
							ImGui::EndChild(true);


						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Web Shared Radar", Languages).c_str(), "o", MenuTheme::StyledPanel(contentSize.x - colSize.x * 2.f - MenuTheme::ColumnGap * 2.f, 6, 4), false, MenuTheme::ScrollPanelFlags());
							{
								char address[512];
								snprintf(address, sizeof(address), "http://%s:%s", GameData.Config.WebRadar.IP, GameData.Config.WebRadar.Port);

								ImGui::InputTextEx(Languages == 1 ? U8("服务器IP") : "Server IP", Languages == 1 ? U8("IP") : "IP", GameData.Config.WebRadar.IP, IM_ARRAYSIZE(GameData.Config.WebRadar.IP), ImVec2(280, 30), NULL);

								ImGui::InputTextEx(Languages == 1 ? U8("服务器Port") : "Server Port", Languages == 1 ? U8("Port") : "Port", GameData.Config.WebRadar.Port, IM_ARRAYSIZE(GameData.Config.WebRadar.Port), ImVec2(280, 30), NULL);

								ImGui::InputTextEx(Languages == 1 ? U8("服务器PIN") : "Server PIN", Languages == 1 ? U8("PIN") : "PIN", GameData.Config.WebRadar.PIN, IM_ARRAYSIZE(GameData.Config.WebRadar.PIN), ImVec2(280, 30), NULL);


								ImGui::InputTextEx(Languages == 1 ? U8("观看地址") : "Watch URL", Languages == 1 ? U8("URL") : "URL", address, IM_ARRAYSIZE(address), ImVec2(280, 30), NULL);

								// 添加链接按钮
								if (ImGui::Button(GameData.Config.WebRadar.isWebRadarConnect ? U8("断开连接") : U8("连接"), ImVec2(200, 30))) {
									if (GameData.Config.WebRadar.isWebRadarConnect) {
										// 断开连接
										GameData.Config.WebRadar.isWebRadarEnable = false;
										GameData.Config.WebRadar.isWebRadarConnect = false;
									}
									else {
										// 建立连接
										GameData.Config.WebRadar.isWebRadarEnable = true;
										GameData.Config.WebRadar.isWebRadarConnect = true;
									}
								}


								if (ImGui::Button(Languages == 1 ? U8("打开浏览器") : "Open Browser", ImVec2(200, 30)))
								{
									OpenLink(address);
								}

							}
							ImGui::EndChild(true);


						}
						ImGui::EndGroup();


					}
					ImGui::EndChild();
				}
				else  if (active_tab == 5)
				{

					ImGui::SetCursorPos(MenuTheme::AnimatedContentPos(region, tab_alpha, anim, false));
					ImGui::BeginChild(false, "Child", "o", contentSize, false, MenuTheme::ScrollPanelFlags());
					{
						ImGui::BeginGroup();
						{
							ImGui::BeginChild(true, MenuLag("Grenade Settings", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 10, 5), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("爆炸提示") : "Grenade Alert", &GameData.Config.Project.Enable, "Grenade Alert");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("瞬爆手雷自瞄") : "Instant Grenade Self-Aiming", &GameData.Config.Project.GrenadeEnable, "Instant Grenade Self-Aiming");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("迫击炮自瞄") : "Mortar Self-Aiming", &GameData.Config.Project.MortarShooting, "Mortar Self-Aiming");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("爆炸范围") : "Explosion Range", &GameData.Config.Project.explosionrange, "Explosion Range");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("高抛预判") : "High Throw Prediction", &GameData.Config.Project.GrenadePrediction, "High Throw Prediction");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("手雷轨迹") : "Grenade Trajectory", &GameData.Config.Project.GrenadeTrajectory, "Grenade Trajectory");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("掐雷倒数") : "Grenade Countdown", &GameData.Config.Project.ShowChareTime, "Grenade Countdown");

								ImGui::ColorEdit5(Languages == 1 ? U8("掐雷颜色") : "Grenade Color", GameData.Config.Project.ChareColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("提示颜色") : "Alert Color", GameData.Config.Project.Color, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("范围颜色") : "Range Color", GameData.Config.Project.explosionrangeColor, picker_flags);

								ImGui::ColorEdit5(Languages == 1 ? U8("轨迹颜色") : "Trajectory Color", GameData.Config.Project.TrajectoryColor, picker_flags);

								ImGui::SliderInt1(Languages == 1 ? U8("提示大小") : "Alert Size", &GameData.Config.Project.FontSize, 10, 50, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("轨迹线条") : "Trajectory Line", &GameData.Config.Project.TrajectorySize, 1, 20, "%d");

								ImGui::SliderInt1(Languages == 1 ? U8("提示距离") : "Alert Distance", &GameData.Config.Project.DistanceMax, 10, 300, "%d M");

								ImGui::SliderInt1(Languages == 1 ? U8("掐雷大小") : "Grenade Size", &GameData.Config.Project.ChareFontSize, 10, 50, "%d");
							}
							ImGui::EndChild(true);


						}
						ImGui::EndGroup();

						ImGui::SameLine();

						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("AimBot Config", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 6, 8), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("盒子自动连接") : "Automatic Connection Of The Box", &GameData.Config.AimBot.AutoConnect, "Automatic Connection Of The Box");

								const char* configItems[] = { Languages == 1 ? U8("配置1") : "Profile1",Languages == 1 ? U8("配置2") : "Profile2" };

								ImGui::SetNextItemWidth(230);
								ImGui::Combo_popup(Languages == 1 ? U8("自瞄配置") : "AimBot Config", &GameData.Config.AimBot.ConfigIndex, configItems, IM_ARRAYSIZE(configItems));

								ImGui::Keybind(Languages == 1 ? U8("配置热键") : "Config Hotkey", &GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Key);

								const char* controllerItems[] = { Languages == 1 ? U8("Kmbox B Pro") : "Kmbox B Pro",Languages == 1 ? U8("Kmbox Net") : "Kmbox Net",Languages == 1 ? U8("Lurker") : "Lurker",Languages == 1 ? U8("键鼠魔盒") : "Magic Box" };

								ImGui::SetNextItemWidth(230);
								ImGui::Combo_popup(Languages == 1 ? U8("自瞄盒子") : "AimBot Box", &GameData.Config.AimBot.Controller, controllerItems, IM_ARRAYSIZE(controllerItems));

								// 获取 COM 端口
								static std::vector<std::string> ports = Utils::GetCOMPorts();

								// Kmbox Net 特殊设置
								if (GameData.Config.AimBot.Controller == 1) {
									ImGui::InputTextEx(Languages == 1 ? U8("IP地址") : "Your Ip",
										"*****",
										GameData.Config.AimBot.IP,
										IM_ARRAYSIZE(GameData.Config.AimBot.IP),
										ImVec2(180, 30),
										NULL);
									ImGui::InputTextEx(Languages == 1 ? U8("端口") : "Your Port",
										"*****",
										GameData.Config.AimBot.Port,
										IM_ARRAYSIZE(GameData.Config.AimBot.Port),
										ImVec2(180, 30),
										NULL);
									ImGui::InputTextEx(Languages == 1 ? U8("UID") : "Your Uid",
										"*****",
										GameData.Config.AimBot.UUID,
										IM_ARRAYSIZE(GameData.Config.AimBot.UUID),
										ImVec2(180, 30),
										NULL);
								}
								else {
									// COM 端口选择
									static std::vector<std::string> utf8Ports;
									utf8Ports.clear();
									std::vector<const char*> items;

									for (const auto& port : Utils::GetCOMPorts()) {
										utf8Ports.push_back(Utils::StringToUTF8(port));
										items.push_back(utf8Ports.back().c_str());
									}

									if (!items.empty()) {
										ImGui::SetNextItemWidth(230);
										ImGui::Combo_popup(Languages == 1 ? U8("COM端口") : "COM Port",
											&GameData.Config.AimBot.COM,
											items.data(),
											items.size());
									}
									else {
										ImGui::SetNextItemWidth(230);
										ImGui::Text(Languages == 1 ? U8("无可用COM端口") : "No Available COM Port");
									}
								}

								// 连接/断开连接按钮
								if (ImGui::Button(
									GameData.Config.AimBot.Connected ?
									(Languages == 1 ? U8("断开连接") : "Disconnect") :
									(Languages == 1 ? U8("连接") : "Connect"),
									ImVec2(GetContentRegionMax().x - spacing.x, 35)))
								{
									bool Connected = false;
									std::string extractedStr;

									if (ports.size() > 0) {
										extractedStr = Utils::ExtractSubstring(ports[GameData.Config.AimBot.COM], R"(COM(\d+))");
									}

									int COM = extractedStr.empty() ? 0 : std::stoi(extractedStr);

									switch (GameData.Config.AimBot.Controller) {
									case 0: Connected = GameData.Config.AimBot.Connected ? (KmBox::Close(), false) : KmBox::Init(COM); break;
									case 1: Connected = GameData.Config.AimBot.Connected ?
										(KmBoxNet::Close(), false) :
										KmBoxNet::Init(GameData.Config.AimBot.IP, GameData.Config.AimBot.Port, GameData.Config.AimBot.UUID);
										break;
									case 2: Connected = GameData.Config.AimBot.Connected ? (Lurker::Close(), false) : Lurker::Init(COM); break;
									case 3: Connected = GameData.Config.AimBot.Connected ? (MoBox::Close(), false) : MoBox::Init(COM); break;
									}

									GameData.Config.AimBot.Connected = Connected;
								}

								// 测试移动按钮
								if (ImGui::Button(Languages == 1 ? U8("测试移动") : "Test Movement",
									ImVec2(GetContentRegionMax().x - spacing.x, 35)))
								{
									switch (GameData.Config.AimBot.Controller) {
									case 0: KmBox::Move(0, 100); break;
									case 1: KmBoxNet::Move(0, 100); break;
									case 2: Lurker::Move(0, 100); break;
									case 3: MoBox::Move(0, 100); break;
									}
								}

								// 保存配置按钮
								if (ImGui::Button(Languages == 1 ? U8("保存配置") : "Save Config",
									ImVec2(GetContentRegionMax().x - spacing.x, 35)))
								{
									if (Config::Save()) {
										ImGuiToast toast(ImGuiToastType_Success,
											Languages == 1 ? U8("保存成功") : "Save Successful");
										ImGui::InsertNotification(toast);
									}
									else {
										ImGuiToast toast(ImGuiToastType_Error,
											Languages == 1 ? U8("保存失败") : "Save Failed");
										ImGui::InsertNotification(toast);
									}
									ImGui::RenderNotifications();  // 实际渲染所有通知
								}

							}
							ImGui::EndChild(true);
						}
						ImGui::EndGroup();

						ImGui::SameLine();
						ImGui::BeginGroup();
						{

							ImGui::BeginChild(true, MenuLag("Model Settings", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 5, 1), false, MenuTheme::ScrollPanelFlags());
							{
								if (ImGui::Checkbox1(Languages == 1 ? U8("低配模型") : "Set Low Model", &GameData.Config.ESP.LowModel)) {
									if (GameData.Config.ESP.LowModel) {
										UpdateModelSelection(0); // 0 for LowModel
									}
								}

								if (ImGui::Checkbox1(Languages == 1 ? U8("中配模型") : "Set Medium Model", &GameData.Config.ESP.MediumModel)) {
									if (GameData.Config.ESP.MediumModel) {
										UpdateModelSelection(1); // 1 for MediumModel
									}
								}

								if (ImGui::Checkbox1(Languages == 1 ? U8("高配模型") : "Set High Model", &GameData.Config.ESP.HighModel)) {
									if (GameData.Config.ESP.HighModel) {
										UpdateModelSelection(2); // 2 for HighModel
									}
								}

								ImGui::CheckboxWishTips(Languages == 1 ? U8("模型展示") : "Model Preview", &GameData.Config.ESP.PhysXDebug, "Model Preview");

								if (ImGui::Button(Languages == 1 ? U8("重载模型") : "Reload Model",
									ImVec2(GetContentRegionMax().x - spacing.x, 35)))
								{
									GameData.ReloadModelRequested.store(true);
								}

								//ImGui::CheckboxWishTips(Languages == 1 ? U8("加载本地模型") : "Load Static Model", &GameData.Config.ESP.LoadStaticModel, "Load Static Model");
							}
							ImGui::EndChild(true);



							ImGui::BeginChild(true, MenuLag("GrenadePredict Settings", Languages).c_str(), "o", MenuTheme::StyledPanel(colSize.x, 6, 0), false, MenuTheme::ScrollPanelFlags());
							{
								ImGui::CheckboxWishTips(Languages == 1 ? U8("投掷/火箭自动锁定") : "Auto Lock Projectiles/Rockets", &GameData.Config.AimBot.ProjectAutoLock, "Auto Lock Projectiles/Rockets");

								ImGui::CheckboxWishTips(Languages == 1 ? U8("自瞄手雷") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.GrenadePredict, "Instant Grenade Self-Aiming");

								ImGui::Keybind(Languages == 1 ? U8("手雷") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.Grenade, true);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("迫击炮自瞄") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.MortarPredict, "Instant Grenade Self-Aiming");

								ImGui::Keybind(Languages == 1 ? U8("迫击炮") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.Mortar2, true);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("火箭筒自瞄") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.PanzerFaust, "Instant Grenade Self-Aiming");

								ImGui::Keybind(Languages == 1 ? U8("火箭") : "Instant Grenade Self-Aiming", &GameData.Config.AimBot.Mortar, true);

								ImGui::CheckboxWishTips(Languages == 1 ? U8("瞬爆手雷自瞄") : "Instant Grenade Self-Aiming", &GameData.Config.Project.GrenadeEnable, "Instant Grenade Self-Aiming");



							}
							ImGui::EndChild(true);


						}
						ImGui::EndGroup();


					}
					ImGui::EndChild();
				}
			}
			ImGui::PopStyleVar();
		}
		MenuTheme::ResetUiScale(style);
		if (font::menu_ui || default_font) ImGui::PopFont();
		ImGui::End();

		ImGui::RenderNotifications();
		AutoConnectBox();
	}
};
