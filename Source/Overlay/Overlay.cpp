#include "Overlay.h"
#include "ESP.h"
#include "Texture.h"
#include <Common/Data.h>
#pragma comment(lib, "D3DX11.lib")
#include <Uxtheme.h>
#include <dwmapi.h>
#include <Utils/Throttler.h>
#include "Style.h"
#include "Map.h"
#include "RenderHelper.h"
#include "MThreadRenderer.h"
#include "Menu.h"
#include "imgui_freetype.h"
#include "font.h"
#include "imgui_elements.h"
#include "texturepng.h"
#include "MenuPlayerLists.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "weapon.h"

#include <d3d11.h>
#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#include <SProtect/SPCloud64.h>
#include "imgui/Notify.h"
#include "Utils/image.h"
#pragma comment(lib, "urlmon.lib")
#include <filesystem>
#define LOGIN_WIDTH 800
#define LOGIN_HEIGHT 550
Wap deWap;
int tabs1 = 0;
float WIDTH = 800; // Loader Size X
float HEIGHT = 550; // Loader Size Y

// SP Cloud 全局变量
static void* g_Ctx_Cloud = NULL;
static std::string g_szUser;
static std::string g_szPassword;
static std::string g_NoticeText = "正在获取最新系统公告...";

#include <windows.h>
// GBK转UTF8工具函数
std::string GbkToUtf8(const char* src)
{
	int wlen = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
	if (wlen <= 0) return "";
	std::wstring wstr(wlen, 0);
	MultiByteToWideChar(CP_ACP, 0, src, -1, &wstr[0], wlen);

	int u8len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (u8len <= 0) return "";
	std::string u8str(u8len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &u8str[0], u8len, NULL, NULL);
	if (!u8str.empty() && u8str.back() == '\0') u8str.pop_back();
	return u8str;
}

// 公告文本（本地静态）
#define SP_EPOCH_DIFF 116444736000000000i64
#define SP_RATE_DIFF 10000000i64
#define SP_TIME_DIFF (8*60*60)
SYSTEMTIME SPTimeStamp2SystemTime(__int64 ts) {
	__int64 tmpTs = (ts + SP_TIME_DIFF) * SP_RATE_DIFF + SP_EPOCH_DIFF;
	FILETIME ft;
	SYSTEMTIME st;
	ft.dwLowDateTime = (DWORD)tmpTs;
	ft.dwHighDateTime = (DWORD)(tmpTs >> 32);
	FileTimeToSystemTime(&ft, &st);
	return st;
}

std::string SPFormatTimestamp(__int64 ts) {
	if (ts == 0) return "";
	SYSTEMTIME st = SPTimeStamp2SystemTime(ts);
	char buf[64];
	sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return std::string(buf);
}

// SP Cloud 心跳线程函数
void SPHeartbeat()
{
	SPErrorCode iError = SPCODE_NOERROR;
	while (true)
	{
		if (!SP_Cloud_Beat(g_Ctx_Cloud, &iError))
		{
			for (int i = 0; i < 5; i++) {
				Sleep(10 * 1000);
				SPErrorCode iError2 = SPCODE_NOERROR;
				if (SP_CloudUserLogin(g_Ctx_Cloud, g_szUser.c_str(), g_szPassword.c_str(), &iError2)) {
					break;
				}
				if (i == 4) {
					char szErrMsg[256];
					SP_Cloud_GetErrorMsg(iError, szErrMsg);
					MessageBoxA(0, GbkToUtf8(szErrMsg).c_str(), "验证错误", MB_ICONERROR);
					ExitProcess(0);
				}
			}
		}
		Sleep(60 * 1000);
	}
}


namespace notifications
{
	static std::string notif_name;
	const char* notif_icon;
	static int notif_state;
	static float notif_rotate;
	static float notif_offset;
	static float notif_timer;
	static float notif_width;
	static float min_width = 150.0f;

	void Message(const char* name, const char* icon)
	{
		notif_name = name;
		notif_icon = icon;
		notif_state = 0;
		notif_rotate = 0;
		notif_offset = 150;
		notif_timer = 0;

		ImGui::PushFont(big_icon);
		float icon_width = ImGui::CalcTextSize(icon).x;
		ImGui::PopFont();

		float text_width = ImGui::CalcTextSize(name).x;
		notif_width = text_width / 2 + icon_width / 2 + 30.f;

		notif_width = (notif_width < min_width) ? min_width : notif_width;
	}

	void NotifyUpdate(ImVec2 p)
	{
		if (CalcTextSize(notif_name.c_str()).x < 1.f)
			return;

		notif_offset = ImLerp(notif_offset, notif_state == 0 ? 0.f : 250.f, anim_speed);
		notif_rotate = ImLerp(notif_rotate, notif_state == 0 ? IM_PI / 2 : notif_state == 1 ? IM_PI / 2 - 0.6f : IM_PI / 2 + 0.6f, anim_speed * 2);
		notif_timer += anim_speed;

		if (notif_timer > 20.f)
			notif_state = 1;

		ImGui::PushFont(big_icon);
		float icon_width = CalcTextSize(notif_icon).x;
		ImGui::PopFont();

		float text_width = CalcTextSize(notif_name.c_str()).x;
		float required_width = text_width / 2 + icon_width / 2 + 30.f;

		notif_width = ImLerp(notif_width, required_width, anim_speed * 3);

		ImRect notif_bb(
			p + ImVec2(WIDTH / 2 - notif_width, HEIGHT - 95 + notif_offset),
			ImVec2(WIDTH / 2 + notif_width, HEIGHT - 35 + notif_offset)
		);

		ImRotateStart();
		rect_glow(ImGui::GetWindowDrawList(), notif_bb.Min, notif_bb.Max,
			ImColor(theme_a::main_color.Value.x, theme_a::main_color.Value.y, theme_a::main_color.Value.z, 0.15f),
			ImGui::GetStyle().FrameRounding, 2.f);

		ImGui::GetWindowDrawList()->AddRectFilled(notif_bb.Min, notif_bb.Max,
			theme_a::background_color,
			ImGui::GetStyle().FrameRounding);

		ImGui::PushFont(big_icon);
		if (notif_icon == "w")
		{
			ImGui::GetWindowDrawList()->AddShadowCircle(notif_bb.Min + ImVec2(30, 30), 10.f,
				ImColor(201, 80, 80, 200), 45.f, ImVec2(0, 0));

			ImGui::GetWindowDrawList()->AddText(notif_bb.Min + ImVec2(30, 30) - CalcTextSize(notif_icon) / 2,
				ImColor(201, 80, 80, 255), notif_icon);
		}
		if (notif_icon == "s")
		{
			ImGui::GetWindowDrawList()->AddShadowCircle(notif_bb.Min + ImVec2(30, 30), 10.f,
				ImColor(82, 201, 80, 200), 45.f, ImVec2(0, 0));

			ImGui::GetWindowDrawList()->AddText(notif_bb.Min + ImVec2(30, 30) - CalcTextSize(notif_icon) / 2,
				ImColor(82, 201, 80, 255), notif_icon);
		}
		ImGui::PopFont();

		ImGui::GetWindowDrawList()->AddText(
			ImVec2(notif_bb.Min.x + 55, center_text(notif_bb.Min, notif_bb.Max, notif_name.c_str()).y),
			theme_a::text_color[0],
			notif_name.c_str()
		);

		ImRotateEnd(notif_rotate);
	}
}

void Trinage_background()
{

	ImVec2 screen_size = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

	static ImVec2 partile_pos[100];
	static ImVec2 partile_target_pos[100];
	static float partile_speed[100];
	static float partile_size[100];
	static float partile_radius[100];
	static float partile_rotate[100];

	for (int i = 1; i < 100; i++)
	{
		if (partile_pos[i].x == 0 || partile_pos[i].y == 0)
		{
			partile_pos[i].x = rand() % (int)screen_size.x + 1;
			partile_pos[i].y = 15.f;
			partile_speed[i] = 1 + rand() % 25;
			partile_radius[i] = rand() % 4;
			partile_size[i] = rand() % 8;

			partile_target_pos[i].x = rand() % (int)screen_size.x;
			partile_target_pos[i].y = screen_size.y * 2;
		}

		partile_pos[i] = ImLerp(partile_pos[i], partile_target_pos[i], ImGui::GetIO().DeltaTime * (partile_speed[i] / 60));
		partile_rotate[i] += ImGui::GetIO().DeltaTime;

		if (partile_pos[i].y > screen_size.y)
		{
			partile_pos[i].x = 0;
			partile_pos[i].y = 0;
			partile_rotate[i] = 0;
		}

		ImRotateStart();
		ImGui::GetWindowDrawList()->AddShadowCircle(partile_pos[i], 1.f, theme_a::main_color, partile_size[i] * 2, ImVec2(0, 0), 0, 1);
		ImGui::GetWindowDrawList()->AddCircleFilled(partile_pos[i], partile_size[i], theme_a::main_color, 1);
		ImRotateEnd(partile_rotate[i]);
	}
}



static bool shouldExit = false;
static int exitDelay = 0;

static bool checkbox = false;
static int iProduct = 0;
static int current_image = 0;
static float images_alpha[4];
static bool g_shouldStartMainProgram = false;
ID3D11ShaderResourceView* bg = nullptr;

ID3D11ShaderResourceView* google_icon = nullptr;
ID3D11ShaderResourceView* twitter_icon = nullptr;

ID3D11ShaderResourceView* Logo = nullptr;

ID3D11ShaderResourceView* gp = nullptr;

ID3D11ShaderResourceView* hd = nullptr;

static bool images_loaded = false;

ID3D11ShaderResourceView* Avatar = nullptr;
ID3D11ShaderResourceView* back_images[4];
ID3D11ShaderResourceView* product_images[4];

// 新增的窗口相关变量
static HWND login_hwnd = nullptr;
static RECT login_rc;


// 在文件开头添加全局变量
static bool g_isLoginUIActive = false;
static RECT g_loginUIRect = { 0, 0, 0, 0 }; // 登录UI区域的位置和大小
static ImVec2 g_loginUIPos = ImVec2(0, 0); // 登录UI的当前位置

int page = 0; int page_aim = 0; int page_item = 0;


namespace font
{
	ImFont* calibri_bold = nullptr;
	ImFont* calibri_bold_hint = nullptr;
	ImFont* calibri_bold_hint2 = nullptr;
	ImFont* myth_bold = nullptr;
	ImFont* myth_bold_b = nullptr;
	ImFont* calibri_regular = nullptr;
	ImFont* icomoon_default = nullptr;
	ImFont* icomoon_menu = nullptr;
	ImFont* pixel_7_small = nullptr;
	ImFont* weapon_val = nullptr;
	ImFont* icomoon = nullptr;
	ImFont* weapon_pubg = nullptr;
	ImFont* menu_ui = nullptr;
	ImFont* menu_ui_bold = nullptr;
	ImFont* menu_ui_sizes[font::kMenuFontCount] = {};
	ImFont* menu_header_sizes[font::kMenuFontCount] = {};
}

namespace texture
{
	ID3D11ShaderResourceView* background = nullptr;
	ID3D11ShaderResourceView* logo = nullptr;
	ID3D11ShaderResourceView* playermoder = nullptr;
	ID3D11ShaderResourceView* weapon_image = nullptr;
	ID3D11ShaderResourceView* rank = nullptr;
	ID3D11ShaderResourceView* bq = nullptr;
	ID3D11ShaderResourceView* sj = nullptr;
	ID3D11ShaderResourceView* lj = nullptr;
	ID3D11ShaderResourceView* jq = nullptr;
	ID3D11ShaderResourceView* sdq = nullptr;
	ID3D11ShaderResourceView* sq = nullptr;
	ID3D11ShaderResourceView* cfq = nullptr;
	ID3D11ShaderResourceView* pj = nullptr;
	ID3D11ShaderResourceView* yp = nullptr;
	ID3D11ShaderResourceView* fj = nullptr;
	ID3D11ShaderResourceView* zd = nullptr;
	ID3D11ShaderResourceView* tzw = nullptr;
	ID3D11ShaderResourceView* ys = nullptr;
	ID3D11ShaderResourceView* item = nullptr;

}

LONG nv_default = WS_POPUP | WS_CLIPSIBLINGS;
LONG nv_default_in_game = nv_default | WS_DISABLED;
LONG nv_edit = nv_default_in_game | WS_VISIBLE;

LONG nv_ex_default = WS_EX_TOOLWINDOW;
LONG nv_ex_edit = nv_ex_default | WS_EX_LAYERED | WS_EX_TRANSPARENT;
LONG nv_ex_edit_menu = nv_ex_default | WS_EX_TRANSPARENT;

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void EnableMouseTransparency()
{
	HWND hWnd = (HWND)ImGui::GetCurrentWindow()->Viewport->PlatformHandleRaw;
	LONG_PTR exStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	exStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED;
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
}

void ShaderResource()
{
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Aimbot_, sizeof(Aimbot_), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Aimbot_id), nullptr);
	RtlZeroMemory(&Aimbot_, sizeof(Aimbot_));

	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, player, sizeof(player), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.player_id), nullptr);
	RtlZeroMemory(&player, sizeof(player));

	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, AK47, sizeof(AK47), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.AK47_id), nullptr);
	RtlZeroMemory(&AK47, sizeof(AK47));

	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, AUG, sizeof(AUG), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.AUG_id), nullptr);
	RtlZeroMemory(&AUG, sizeof(AUG));

	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, AWM, sizeof(AWM), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.AWM_id), nullptr);
	RtlZeroMemory(&AWM, sizeof(AWM));

	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Berreta686, sizeof(Berreta686), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Berreta686_id), nullptr);
	RtlZeroMemory(&Berreta686, sizeof(Berreta686));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, BerylM762, sizeof(BerylM762), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.BerylM762_id), nullptr);
	RtlZeroMemory(&BerylM762, sizeof(BerylM762));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, BizonPP19, sizeof(BizonPP19), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.BizonPP19_id), nullptr);
	RtlZeroMemory(&BizonPP19, sizeof(BizonPP19));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Crossbow, sizeof(Crossbow), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Crossbow_id), nullptr);
	RtlZeroMemory(&Crossbow, sizeof(Crossbow));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, DP12, sizeof(DP12), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.DP12_id), nullptr);
	RtlZeroMemory(&DP12, sizeof(DP12));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, DP28, sizeof(DP28), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.DP28_id), nullptr);
	RtlZeroMemory(&DP28, sizeof(DP28));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, FNFal, sizeof(FNFal), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.FNFal_id), nullptr);
	RtlZeroMemory(&FNFal, sizeof(FNFal));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, G36C, sizeof(G36C), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.G36C_id), nullptr);
	RtlZeroMemory(&G36C, sizeof(G36C));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Groza, sizeof(Groza), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Groza_id), nullptr);
	RtlZeroMemory(&Groza, sizeof(Groza));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, HK416, sizeof(HK416), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.HK416_id), nullptr);
	RtlZeroMemory(&HK416, sizeof(HK416));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, K2, sizeof(K2), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.K2_id), nullptr);
	RtlZeroMemory(&K2, sizeof(K2));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Kar98k, sizeof(Kar98k), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Kar98k_id), nullptr);
	RtlZeroMemory(&Kar98k, sizeof(Kar98k));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, L6, sizeof(L6), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.L6_id), nullptr);
	RtlZeroMemory(&L6, sizeof(L6));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, M16A4, sizeof(M16A4), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.M16A4_id), nullptr);
	RtlZeroMemory(&M16A4, sizeof(M16A4));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, M24, sizeof(M24), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.M24_id), nullptr);
	RtlZeroMemory(&M24, sizeof(M24));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, M249, sizeof(M249), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.M249_id), nullptr);
	RtlZeroMemory(&M249, sizeof(M249));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, MG3, sizeof(MG3), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.MG3_id), nullptr);
	RtlZeroMemory(&MG3, sizeof(MG3));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Mini14, sizeof(Mini14), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Mini14_id), nullptr);
	RtlZeroMemory(&Mini14, sizeof(Mini14));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, MK12, sizeof(MK12), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Mk12_id), nullptr);
	RtlZeroMemory(&MK12, sizeof(MK12));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, MK14, sizeof(MK14), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Mk14_id), nullptr);
	RtlZeroMemory(&MK14, sizeof(MK14));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Mk47Mutant, sizeof(Mk47Mutant), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Mk47Mutant_id), nullptr);
	RtlZeroMemory(&Mk47Mutant, sizeof(Mk47Mutant));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Mosin, sizeof(Mosin), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Mosin_id), nullptr);
	RtlZeroMemory(&Mosin, sizeof(Mosin));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, MP5K, sizeof(MP5K), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.MP5K_id), nullptr);
	RtlZeroMemory(&MP5K, sizeof(MP5K));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, P90, sizeof(P90), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.P90_id), nullptr);
	RtlZeroMemory(&P90, sizeof(P90));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, QBU88, sizeof(QBU88), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.QBU88_id), nullptr);
	RtlZeroMemory(&QBU88, sizeof(QBU88));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, QBZ95, sizeof(QBZ95), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.QBZ95_id), nullptr);
	RtlZeroMemory(&QBZ95, sizeof(QBZ95));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Saiga12, sizeof(Saiga12), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Saiga12_id), nullptr);
	RtlZeroMemory(&Saiga12, sizeof(Saiga12));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, SCAR_L, sizeof(SCAR_L), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.SCAR_L_id), nullptr);
	RtlZeroMemory(&SCAR_L, sizeof(SCAR_L));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, SKS, sizeof(SKS), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.SKS_id), nullptr);
	RtlZeroMemory(&SKS, sizeof(SKS));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Thompson, sizeof(Thompson), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Thompson_id), nullptr);
	RtlZeroMemory(&Thompson, sizeof(Thompson));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, UMP, sizeof(UMP), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.UMP_id), nullptr);
	RtlZeroMemory(&UMP, sizeof(UMP));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, UZI, sizeof(UZI), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.UZI_id), nullptr);
	RtlZeroMemory(&UZI, sizeof(UZI));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Vector, sizeof(Vector), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Vector_id), nullptr);
	RtlZeroMemory(&Vector, sizeof(Vector));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, VSS, sizeof(VSS), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.VSS_id), nullptr);
	RtlZeroMemory(&VSS, sizeof(VSS));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Win1894, sizeof(Win1894), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Win1894_id), nullptr);
	RtlZeroMemory(&Win1894, sizeof(Win1894));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Winchester, sizeof(Winchester), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Winchester_id), nullptr);
	RtlZeroMemory(&Winchester, sizeof(Winchester));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, DesertEagle, sizeof(DesertEagle), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.DesertEagle_id), nullptr);
	RtlZeroMemory(&DesertEagle, sizeof(DesertEagle));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, FlareGun, sizeof(FlareGun), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.FlareGun_id), nullptr);
	RtlZeroMemory(&FlareGun, sizeof(FlareGun));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, G18, sizeof(G18), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.G18_id), nullptr);
	RtlZeroMemory(&G18, sizeof(G18));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, M9, sizeof(M9), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.M9_id), nullptr);
	RtlZeroMemory(&M9, sizeof(M9));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, M1911, sizeof(M1911), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.M1911_id), nullptr);
	RtlZeroMemory(&M1911, sizeof(M1911));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, NagantM1895, sizeof(NagantM1895), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.NagantM1895_id), nullptr);
	RtlZeroMemory(&NagantM1895, sizeof(NagantM1895));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Rhino, sizeof(Rhino), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Rhino_id), nullptr);
	RtlZeroMemory(&Rhino, sizeof(Rhino));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, Sawnoff, sizeof(Sawnoff), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.Sawnoff_id), nullptr);
	RtlZeroMemory(&Sawnoff, sizeof(Sawnoff));
	D3DX11CreateShaderResourceViewFromMemory(
		g_pd3dDevice, vz61Skorpion, sizeof(vz61Skorpion), nullptr, nullptr,
		reinterpret_cast<ID3D11ShaderResourceView**>(&deWap.vz61Skorpion_id), nullptr);
	RtlZeroMemory(&vz61Skorpion, sizeof(vz61Skorpion));
}


void PNSImG()
{
	// 预加载武器与通用图标资源，确保 ESP.h 使用的路径可用
	Texture::LoadTextures(g_pd3dDevice, "Assets/image/Weapon");
	Texture::LoadTextures(g_pd3dDevice, "Assets/image/All");

    {
        std::string koiPath = "Assets/theme/koi.png";
        if (std::filesystem::exists(koiPath))
        {
            GImGuiTextureMap[koiPath] = Texture::LoadTexture(g_pd3dDevice, koiPath);
        }
    }
	std::string IconUrl = "Assets/image/Map/car_land.png";// 地图载具

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/Carapackage_RedBox_C.png";   //地图空投

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/dead.png";  //地图死亡

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/back_room.png";  //地图密室

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/indicator_onscreen_status_disconnect.png";  //地图掉线

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/indicator_onscreen_status_parachute.png";  //地图跳伞

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/indicator_onscreen_status_DBNO.png";  //地图倒地

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/indicator_onscreen_status_vehicle.png";  //地图开车

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/Carapackage_SmallPackage_C.png";  //地图小空投

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/arrow.png";  //地图人物转向

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Armor_Repair_Kit_C.png";   //防具修理包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Vehicle_Repair_Kit_C.png";   //载具修理包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Helmet_Repair_Kit_C.png";   //头盔修理包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/InstantRevivalKit_C.png";  //一秒扶

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Tiger_SelfRevive_C.png";  //自救器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Rubberboat_C.png";  //皮划艇

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_9mm_C.png";    //9毫米子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_12Guage_C.png";     //12口径子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_12GuageSlug_C.png";    //O12子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_40mm_C.png";  //40mm烟雾弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_40mmBluezone_C.png";  //40mm篮圈弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_45ACP_C.png";    //.45子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_57mm_C.png";     //P90子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_300Magnum_C.png";   //马格南子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_303Ball_C.png";      //马格南子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_556mm_C.png";   //556子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_762mm_C.png";    //762子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_Bolt_C.png";     //箭

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_Flare_C.png";   //信号枪子弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_Mortar_C.png";     //迫击炮弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Armor_C_01_Lv3_C.png";  //三级甲

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Armor_D_01_Lv2_C.png";   //二级甲

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Armor_E_01_Lv1_C.png";    //一级甲

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_AngledForeGrip_C.png";   //三角握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_Foregrip_C.png";    //垂直握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_HalfGrip_C.png";     //红色握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_LaserPointer_C.png";   //激光瞄准器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_LightweightForeGrip_C.png";  //轻型握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_QuickDraw_Large_Crossbow_C.png";  //箭袋

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_ThumbGrip_C.png";   //拇指握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Lower_TiltedGrip_C.png";   //斜向握把

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_Extended_Large_C.png";   //步枪扩容

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_Extended_Medium_C.png";   //冲锋扩容

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_Extended_Small_C.png";    //手枪扩容

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_Extended_SniperRifle_C.png";  //狙击扩容

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Large_C.png";  //步枪快扩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Medium_C.png";  //冲锋快扩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Small_C.png"; //手枪快扩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_ExtendedQuickDraw_SniperRifle_C.png"; //狙击快扩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_QuickDraw_Large_C.png";  //步枪快速

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_QuickDraw_Medium_C.png";   //冲锋快速

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_QuickDraw_Small_C.png";   //手枪快速

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Magazine_QuickDraw_SniperRifle_C.png";   //狙击快速

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Choke_C.png"; //扼流圈

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Compensator_Large_C.png";   //步枪补偿

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_AR_MuzzleBrake_C.png";   //步枪制退器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Compensator_Medium_C.png";   //冲锋枪补偿

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Compensator_SniperRifle_C.png";   //狙击补偿

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Duckbill_C.png";  //鸭嘴枪口

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_FlashHider_Large_C.png";  //步枪消焰器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_FlashHider_Medium_C.png";   //冲锋枪消焰器 

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_FlashHider_SniperRifle_C.png";//狙击枪消焰器 

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Suppressor_Large_C.png";   //步枪消音器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Suppressor_Medium_C.png";//冲锋枪消音器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Suppressor_Small_C.png";//手枪消音器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Muzzle_Suppressor_SniperRifle_C.png";//狙击枪消音器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_SideRail_DotSight_RMR_C.png";  //侧边瞄准器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Stock_AR_Composite_C.png";  //战术枪托

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Stock_AR_HeavyStock_C.png";  //重型枪托

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Stock_SniperRifle_BulletLoops_C.png";   //子弹袋

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Stock_SniperRifle_CheekPad_C.png";    //托腮板

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Stock_UZI_C.png";   //折叠枪托

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_DotSight_01_C.png";   //红点

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_Holosight_C.png";    //全息

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_Aimpoint_C.png";   //2倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_Scope3x_C.png";   //3倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_ACOG_01_C.png";    //4倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_DualOptic_4x1x_C.png";    //多倍率混合瞄具

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_Scope6x_C.png";    //6倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_CQBSS_C.png";    //8倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_PM2_01_C.png";    //15倍镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Attach_Weapon_Upper_Thermal_C.png";    //热成像

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_B_08_Lv3_C.png";   //三级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_C_01_Lv3_C.png";   //三级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_C_02_Lv3_C.png";   //三级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_F_02_Lv2_C.png";    //二级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_F_01_Lv2_C.png";   //二级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_BlueBlocker_Lv1.png";    //电磁包L1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_BlueBlocker.png";    //电磁包L2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_BlueBlocker_Lv3.png";    //电磁包L3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_E_02_Lv1_C.png";   //一级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Back_E_01_Lv1_C.png";   //一级包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Bluechip_C.png";  //蓝色晶片

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Revival_Transmitter_C.png";  //蓝色晶片发射器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Boost_AdrenalineSyringe_C.png";   //肾上腺素

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Boost_EnergyDrink_C.png";    //能量饮料  

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Boost_PainKiller_C.png";    //止疼药

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Heal_FirstAid_C.png";   //急救包

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Heal_MedKit_C.png";   //医疗箱

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Heal_Bandage_C.png";   //绷带

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_TacPack_C.png";   //战术套件

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Heal_BattleReadyKit_C.png";   //战斗准备套件

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_TraumaBag_C.png";    //医疗套件

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_IntegratedRepair_C.png";    //三合一套件

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_01_C.png";   //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_02_C.png";      //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_03_C.png";     //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_04_C.png";      //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_05_C.png";     //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_06_C.png";     //吉利服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Head_E_01_Lv1_C.png";  //一级头

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Head_E_02_Lv1_C.png";   //一级头

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Head_F_01_Lv2_C.png";  //二级头

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Head_F_02_Lv2_C.png";  //二级头

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Head_G_01_Lv3_C.png";  //三级头

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_BTSecretRoom_Key_C.png";  //海岛钥匙

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Chimera_Key_C.png";  //火山钥匙

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Heaven_Key_C.png";  //钥匙

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_DihorOtok_Key_C.png";  //门禁卡

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Secuity_Keycard_C.png";  //蒂斯顿门禁卡

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Tiger_Key_C.png";  //泰戈钥匙

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Neon_Key_C.png";  //荣都钥匙

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Neon_Gold_C.png";  //金条

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Neon_Coin_C.png";  //交易币

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_BulletproofShield_C.png";   //折叠盾牌

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Mountainbike_C.png";   //折叠自行车

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_EmergencyPickup_C.png";  //紧急呼救器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_JerryCan_C.png";   //汽油桶

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_ACE32_C.png";   //ACE

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_AK47_C.png";   //AK

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_AUG_C.png";   //AUG

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_AWM_C.png";   //M24

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Berreta686_C.png";   //S686

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_BerylM762_C.png";   //M762

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_BizonPP19_C.png";   //野牛冲锋枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Crossbow_C.png";   //十字弩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_BluezoneGrenade_C.png";   //蓝圈手雷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_C4_C.png";   //C4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Cowbar_C.png";   //撬棍

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Pickaxe_C.png";   //镐

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Ziplinegun_C.png";   //绳索发射器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ammo_ZiplinegunHook_C.png";   //绳索

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_DecoyGrenade_C.png";   //诱饵弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_DesertEagle_C.png";    //沙漠之鹰

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_DP12_C.png";   //DBS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_DP28_C.png";   //DP28

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Dragunov_C.png";   //德拉贡诺夫

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Drone_C.png";   //无人机

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Duncans_M416_C.png";   //M416

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_FAMASG2_C.png";   //FAMASI

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_FlareGun_C.png";    //信号枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_PackageFlare_C.png";    //应急补给信号弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_FlashBang_C.png";    //闪光弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_FNFal_C.png";   //SLR

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_G18_C.png";   //P18C

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_G36C_C.png";   //G36C

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Grenade_C.png";   //手雷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Groza_C.png";   //狗杂

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_HK416_C.png";   //M416

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_JS9_C.png";   //JS9

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/WeapJS9_C.png";   //JS9

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Julies_Kar98k_C.png";   //98K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M9_C.png";   //P92

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_L6_C.png";    //巴雷特

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Kar98k_C.png";   //98K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_K2_C.png";   //K2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M16A4_C.png";   //M16A4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M24_C.png";   //M24·

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M79_C.png";   //M79

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_BZGL_C.png";   //篮圈枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Bluebomb_Subzero_C.png";   //篮圈扩散器

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Ghillie_BlueBlocker_C.png";   //防寒服

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M249_C.png";    //M249

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_M1911_C.png";  //P1911

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Machete_C.png";   //砍刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mads_QBU88_C.png";   //QBU

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_MG3_C.png";   //MG3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mini14_C.png";    //Mini14

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mk12_C.png";   //Mk12

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mk14_C.png";   //Mk14

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mk47Mutant_C.png";   //Mk47

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Molotov_C.png";     //燃烧瓶

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mortar_C.png";   //迫击炮

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Mosin_C.png";   //莫辛甘纳

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_MP5K_C.png";    //MP5K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_MP9_C.png";    //MP9

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_NagantM1895_C.png";   //R1895

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_OriginS12_C.png";   //O12

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_P90_C.png";    //P90

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Pan_C.png";    //平底锅

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_PanzerFaust100M_C.png";    //火箭筒

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_QBU88_C.png";    //QBU

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_QBZ95_C.png";   //QBZ95

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Rhino_C.png";   //R45

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Saiga12_C.png";   //S12K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Sawnoff_C.png";   //锯齿短喷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_SCAR-L_C.png";     //SCAR - L

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Sickle_C.png";   //镰刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/WeapSickle_C.png";   //镰刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/WeapSickleProjectile_C.png";    //镰刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_SKS_C.png";   //SKS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_SmokeBomb_C.png";  //烟雾弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/WeapSmokeBomb_C.png";  //烟雾弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_SpikeTrap_C.png";    //尖刺陷阱

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Spotter_Scope_C.png";   //望远镜

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_StickyGrenade_C.png";  //粘性炸弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_StunGun_C.png";   //电击枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Thompson_C.png";   //汤姆逊

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_UMP_C.png";   //UMP

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_UZI_C.png";   //UZI

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Vector_C.png";   //Vector

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_VSS_C.png";   //VSS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_vz61Skorpion_C.png";   //蝎式手枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Win1894_C.png";   //Win94

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/All/Item_Weapon_Winchester_C.png";  //S1897

	//手持武器贴图

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/ACE.png";   //ACE

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/AKM.png";  //AKM

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/AUG.png";  //AUG

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/AWM.png";  //AWM

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/BerylM762.png";  //BerylM762

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/C4.png";  //C4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/DBS.png";  //DBS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/DP28.png";  //DP28

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/FAMASI.png";  //FAMASI

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/G36C.png";  //G36C

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Groza.png";  //Groza

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/JS9.png";  //JS9

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/K2.png";  //K2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Kar98k.png";  //Kar98k

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Lynx.png";  //Lynx

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/M16A4.png";  //M16A4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/M24.png";  //M24

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/M249.png";  //M249

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/M416.png";  //M416

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/MG3.png";  //MG3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Mini14.png";  //Mini14

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Mk12.png";  //Mk12

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Mk14.png";  //Mk14

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Mk47.png";  //Mk47

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/MP5K.png";  //MP5K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/MP9.png";  //MP9

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/O12.png";  //O12

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/P18C.png";  //P18C

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/P90.png";  //P90

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/P92.png";  //P92

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/P1911.png";  //P1911

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/QBU.png";  //QBU

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/QBZ95.png";  //QBZ95

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/R45.png";  //R45

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/R1895.png";  //R1895

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/S12K.png";  //S12K

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/S686.png";  //S686

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/S1897.png";  //S1897

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/SCAR-L.png";  //SCAR-L

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/SKS.png";  //SKS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/SLR.png";  //SLR

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/UMP.png";  //UMP

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/UZI.png";  //UZI

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Vector.png";  //Vector

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/VSS.png";  //VSS

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/Win94.png";  //Win94

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/德拉贡诺夫.png";  //德拉贡诺夫

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/电击枪.png";  //电击枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/火箭筒.png";  //火箭筒

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/尖刺陷阱.png";  //尖刺陷阱

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/锯齿短喷.png";  //锯齿短喷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/砍刀.png";  //砍刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/篮圈手雷.png";  //蓝圈手雷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/镰刀.png";  //镰刀

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/莫辛甘纳.png";  //莫辛甘纳

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/弩.png";  //弩

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/平底锅.png";  //平底锅

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/撬棍.png";  //撬棍

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/燃烧瓶.png";  //燃烧瓶

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/沙漠之鹰.png";  //沙漠之鹰

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/闪光弹.png";  //闪光弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/手雷.png";  //手雷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/汤姆逊.png";  //汤姆逊

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/蝎式手枪.png";  //蝎式手枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/信号枪.png";  //信号枪

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/烟雾弹.png";  //烟雾弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/野牛PP19.png";  //野牛PP19

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/诱饵手雷.png";  //诱饵手雷

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Weapon/粘性炸弹.png";  //粘性炸弹

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	//段位相关

	IconUrl = "Assets/image/RankImage/Bronze-1.png";  //青铜1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Bronze-2.png"; // 青铜2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Bronze-3.png";  // 青铜3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Bronze-4.png";  // 青铜4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Bronze-5.png";  // 青铜5

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Silver-1.png";  //白银1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Silver-2.png";  //白银2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Silver-3.png";  //白银3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Silver-4.png";  //白银4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Silver-5.png";  //白银5

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Gold-1.png";  //黄金1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Gold-2.png";  //黄金2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Gold-3.png";  //黄金3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Gold-4.png";  //黄金4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Gold-5.png";  //黄金5

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Platinum-1.png";  //铂金1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Platinum-2.png";  //铂金2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Platinum-3.png";  //铂金3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Platinum-4.png";  //铂金4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Platinum-5.png";  //铂金5

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Crystal-1.png";  //水晶1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Crystal-2.png";  //水晶2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Crystal-3.png";  //水晶3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Crystal-4.png";  //水晶4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Diamond-1.png";  //钻石1

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Diamond-2.png";  //钻石2

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Diamond-3.png";  //钻石3

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Diamond-4.png";  //钻石4

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Diamond-5.png";  //钻石5

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Master.png";  //大师

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Survivor.png";  //生存者

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/RankImage/Unranked.png";  //未定级

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);

	IconUrl = "Assets/image/Map/PaoX.png";  //logo

	GImGuiTextureMap[IconUrl] = Texture::LoadTexture(g_pd3dDevice, IconUrl);
}


void ClickThrough(bool v)
{
	if (v) {
		nv_edit = nv_default_in_game | WS_VISIBLE;
		if (GetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE) != nv_ex_edit)
			SetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE, nv_ex_edit);
	}
	else {
		nv_edit = nv_default | WS_VISIBLE;
		if (GetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE) != nv_ex_edit_menu)
			SetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE, nv_ex_edit_menu);
	}
}
// Main code
void BeginDraw(ImGuiIO& io)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	if (GameData.Config.ESP.Mouse)
	{
		GameData.Config.Overlay.pt = GameData.Keyboard.GetCursorPosition();
		if ((GameData.Config.Overlay.pt.x >= menuRect.left && GameData.Config.Overlay.pt.x <= menuRect.right &&
			GameData.Config.Overlay.pt.y >= menuRect.top && GameData.Config.Overlay.pt.y <= menuRect.bottom) && GameData.Config.Menu.Show)
		{
			io.DisplaySize = ImVec2((float)GameData.Config.Overlay.ScreenWidth, (float)GameData.Config.Overlay.ScreenHeight);

			// —— 鼠标位置 —— 
			io.MousePos = ImVec2(
				(float)GameData.Config.Overlay.pt.x,
				(float)GameData.Config.Overlay.pt.y);

			// —— 鼠标按键 —— 
			io.MouseDown[0] = GameData.Keyboard.IsKeyDown(VK_LBUTTON);  // 左键
			io.MouseDown[1] = GameData.Keyboard.IsKeyDown(VK_MBUTTON);  // 中键
			io.MouseDown[2] = GameData.Keyboard.IsKeyDown(VK_RBUTTON);  // 右键
			io.MouseDown[3] = GameData.Keyboard.IsKeyDown(VK_XBUTTON1);  // X1
			io.MouseDown[4] = GameData.Keyboard.IsKeyDown(VK_XBUTTON2);  // X2

			// —— 键盘按键 —— （持续按住检测） 
			for (int vk = 0; vk < IM_ARRAYSIZE(io.KeysDown); vk++)
				io.KeysDown[vk] = GameData.Keyboard.IsKeyDown(vk);

			// —— 修饰键 —— 
			io.KeyCtrl = io.KeysDown[VK_CONTROL] || io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL];
			io.KeyShift = io.KeysDown[VK_SHIFT] || io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT];
			io.KeyAlt = io.KeysDown[VK_MENU] || io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
			io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
		}
	}


	ImGui::NewFrame();
}
void EndDraw(ImGuiIO& io)
{
	ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	ESP::DrawFPS(io);
	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	g_pSwapChain->Present((GameData.Config.Overlay.FusionMode && !GameData.Config.Overlay.VSync) ? 0 : 1, 0);
}

void SetWindowMouseTransparent(HWND hwnd, bool transparent)
{
	LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (transparent)
	{
		exStyle |= WS_EX_TRANSPARENT;
	}
	else
	{
		exStyle &= ~WS_EX_TRANSPARENT;
	}
	SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
}

void UpdateWindowTransparency()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(GameData.Config.Overlay.hWnd, &mousePos);

	if (ImGui::IsWindowHovered(4)) {
		SetWindowMouseTransparent(GameData.Config.Overlay.hWnd, false);
		SetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE, nv_ex_edit_menu);
	}
	else {
		SetWindowMouseTransparent(GameData.Config.Overlay.hWnd, true);
		SetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE, nv_ex_edit);
	}

	if (mousePos.x >= menuRect.left && mousePos.x <= menuRect.right &&
		mousePos.y >= menuRect.top && mousePos.y <= menuRect.bottom) {
		// 鼠标在菜单区域内，不穿透
		SetWindowLong(GameData.Config.Overlay.hWnd, GWL_EXSTYLE, nv_ex_edit_menu);
	}
}
RECT rc;
std::string g_pvar;
char g_user[52]{ 0 };
char g_pass[52]{ 0 };
int g_Handle_bol = 0;

static ImVec2 ui_offset = { 0, 0 };

std::string g_expTime = "到期时间：2034-12-30";

namespace font_inter
{
	ImFont* msyh_font = nullptr;
	ImFont* inter_black = nullptr;
	ImFont* inter_bold = nullptr;
	ImFont* inter_medium = nullptr;
	ImFont* inter_regular = nullptr;

	ImFont* inter_black_preview = nullptr;
	ImFont* inter_bold_name = nullptr;

	ImFont* icon_notify;




}

std::string ConvertToUTF8(const std::string& input)
{
	int len = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL, 0);
	std::wstring wstr(len, 0);
	MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, &wstr[0], len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string utf8_str(len, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], len, NULL, NULL);

	return utf8_str;
}

enum OperationType {
	OP_NONE = 0,
	OP_LOGIN,
	OP_REGISTER,
	OP_RECHARGE,
	OP_CHANGE_PASSWORD,
	OP_UNBIND
};

struct AsyncRequest {
	OperationType operation;
	char username[128];
	char password[40];
	char new_password[40];
	char super_password[40];
	char key[40];
	bool is_processing;
	bool is_completed;
	bool is_success;
	std::string error_message;
	std::string success_message;
	int current_auth_page_result; // 用于页面跳转
};

static AsyncRequest g_asyncRequest = {};
static HANDLE g_operationThread = nullptr;

// 异步操作线程函数
DWORD WINAPI AsyncOperationProc(LPVOID lpParam) {
	AsyncRequest* request = (AsyncRequest*)lpParam;
	SPErrorCode iError = SPCODE_NOERROR;
	char szErrorMsg[256] = { 0 };

	switch (request->operation) {
	case OP_LOGIN:
	{
		if (SP_CloudUserLogin(g_Ctx_Cloud, request->username, request->password, &iError) == false) {
			if (iError == SPCODE_BINDMSGDIFF) {
				request->error_message = U8("绑定另一设备! 请先解绑后再登录");
				request->is_success = false;
			}
			else if (iError == SPCODE_MAXONLINE) {
				request->error_message = U8("已达到最大在线数量! 请踢掉其他在线设备后再登录");
				request->is_success = false;
			}
			else {
				SP_Cloud_GetErrorMsg(iError, szErrorMsg);
				request->error_message = GbkToUtf8(szErrorMsg);
				request->is_success = false;
			}
		}
		else {
			g_szUser = request->username;
			g_szPassword = request->password;

			__int64 expiredTs = 0;
			if (SP_Cloud_GetExpiredTimeStamp(g_Ctx_Cloud, &expiredTs, nullptr)) {
				GameData.ExperTime = SPFormatTimestamp(expiredTs);
			}

			int bForce = 0, dwVer = 0, bDirectUrl = 0;
			char szUrl[2049] = { 0 }, szRunExe[101] = { 0 }, szRunCmd[129] = { 0 };
			if (SP_Cloud_GetUpdateInfo(g_Ctx_Cloud, &bForce, &dwVer, &bDirectUrl, szUrl, szRunExe, szRunCmd, nullptr)) {
				GameData.Version = std::to_string(dwVer);
				GameData.Url = szUrl;
				GameData.pbForceUpdating = (bForce != 0);
			}

			request->success_message = std::string(request->username) + U8(":欢迎登入");
			request->is_success = true;
		}
		break;
	}

	case OP_REGISTER:
	{
		if (SP_Cloud_UserRegister(g_Ctx_Cloud, request->username, request->password, request->super_password, request->key, &iError)) {
			request->success_message = U8("注册成功");
			request->is_success = true;
			request->current_auth_page_result = 0;
		}
		else {
			SP_Cloud_GetErrorMsg(iError, szErrorMsg);
			request->error_message = GbkToUtf8(szErrorMsg);
			request->is_success = false;
		}
		break;
	}

	case OP_RECHARGE:
	{
		tagUserRechargedInfo rechargeInfo = { 0 };

		if (SP_Cloud_UserRecharge(g_Ctx_Cloud, request->username, request->key, &rechargeInfo, &iError)) {
			std::string newExpTime = SPFormatTimestamp(rechargeInfo.u64NewExpiredTimeStamp);
			request->success_message = U8("充值成功,到期时间: ") + newExpTime;
			request->is_success = true;
		}
		else {
			SP_Cloud_GetErrorMsg(iError, szErrorMsg);
			request->error_message = GbkToUtf8(szErrorMsg);
			request->is_success = false;
		}
		break;
	}

	case OP_CHANGE_PASSWORD:
	{
		if (SP_Cloud_UserChangePWD(g_Ctx_Cloud, request->username, request->super_password, request->new_password, &iError)) {
			request->success_message = U8("密码修改成功");
			request->is_success = true;
			request->current_auth_page_result = 0;
		}
		else {
			SP_Cloud_GetErrorMsg(iError, szErrorMsg);
			request->error_message = GbkToUtf8(szErrorMsg);
			request->is_success = false;
		}
		break;
	}

	case OP_UNBIND:
	{
		char szPCSign[33] = { 0 };
		if (SP_Cloud_GetPCSign(g_Ctx_Cloud, szPCSign)) {
			if (SP_Cloud_UserRemovePCSign(g_Ctx_Cloud, request->username, request->password, szPCSign, 0, &iError)) {
				request->success_message = U8("解绑成功");
				request->is_success = true;
			}
			else {
				SP_Cloud_GetErrorMsg(iError, szErrorMsg);
				request->error_message = GbkToUtf8(szErrorMsg);
				request->is_success = false;
			}
		}
		else {
			request->error_message = U8("获取机器码失败");
			request->is_success = false;
		}
		break;
	}
	}

	request->is_completed = true;
	request->is_processing = false;
	return 0;
}

// 启动异步操作的辅助函数
void StartAsyncOperation(OperationType op, const char* username, const char* password = "",
	const char* new_password = "", const char* super_password = "", const char* key = "") {
	if (g_asyncRequest.is_processing) return; // 如果已有操作在处理中，直接返回

	g_asyncRequest.operation = op;
	strcpy_s(g_asyncRequest.username, username);
	strcpy_s(g_asyncRequest.password, password);
	strcpy_s(g_asyncRequest.new_password, new_password);
	strcpy_s(g_asyncRequest.super_password, super_password);
	strcpy_s(g_asyncRequest.key, key);
	g_asyncRequest.is_processing = true;
	g_asyncRequest.is_completed = false;
	g_asyncRequest.is_success = false;
	g_asyncRequest.error_message.clear();
	g_asyncRequest.success_message.clear();
	g_asyncRequest.current_auth_page_result = -1;

	// 启动操作线程
	g_operationThread = CreateThread(NULL, 0, AsyncOperationProc, &g_asyncRequest, 0, NULL);
}




void LoadImages()
{
	if (images_loaded)
		return;

	D3DX11_IMAGE_LOAD_INFO iInfo;
	ID3DX11ThreadPump* threadPump{ nullptr };
	// From Bytes
	D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, avatar_bytes, sizeof(avatar_bytes), &iInfo, threadPump, &Avatar/*shader*/, 0);

	D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, dotaimage, sizeof(dotaimage), &iInfo, threadPump, &back_images[0]/*shader*/, 0);


	D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, productdota, sizeof(productdota), &iInfo, threadPump, &product_images[0]/*shader*/, 0);

	D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logotype, sizeof(logotype), &iInfo, threadPump, &Logo/*shader*/, 0);

	images_loaded = true;
}

HWND hwnd;

void move_window() {

	GetWindowRect(hwnd, &rc);
	MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, WIDTH, HEIGHT, TRUE);
	ImGui::SetWindowPos(ImVec2(0.f, 0.f));
}

void RenderBlur(HWND hwnd)
{
	struct ACCENTPOLICY
	{
		int na;
		int nf;
		int nc;
		int nA;
	};
	struct WINCOMPATTRDATA
	{
		int na;
		PVOID pd;
		ULONG ul;
	};

	const HINSTANCE hm = LoadLibrary(L"user32.dll");
	if (hm)
	{
		typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

		const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
		if (SetWindowCompositionAttribute)
		{
			ACCENTPOLICY policy = { 3, 0, 0, 0 };

			WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
			SetWindowCompositionAttribute(hwnd, &data);
		}
		FreeLibrary(hm);
	}
}





bool DrawMain(int Width, int Height, ImGuiIO& io) {

	// 添加验证相关的静态变量
	static char Pwd[40] = { 0 };
	static bool Save = 1;
	static char NewPwd[40] = { 0 };
	static char SuperPwd[40] = { 0 };
	static char Key[40] = { 0 };


	static bool is_config_loaded = false;
	if (!is_config_loaded) {
		std::ifstream configFile("lic.ini");
		if (configFile.is_open()) {
			std::string line;
			while (std::getline(configFile, line)) {
				// 跳过空行或注释行
				if (line.empty() || line[0] == '[' || line[0] == '#') {
					continue;
				}

				// 解析 save 字段
				if (line.find("save=") == 0) {
					Save = std::stoi(line.substr(5)) == 1;
				}

				// 只有在保存选项开启时才加载用户名和密码
				if (Save) {
					// 解析 user 字段
					if (line.find("user=") == 0) {
						std::strncpy(GameData.Acct, line.substr(5).c_str(), sizeof(GameData.Acct) - 1);
					}

					// 解析 pass 字段
					if (line.find("pass=") == 0) {
						std::strncpy(Pwd, line.substr(5).c_str(), sizeof(Pwd) - 1);
					}
				}
			}
			configFile.close();
		}
		is_config_loaded = true; // 标记为已加载
	}



	UpdateWindowTransparency();

	BeginDraw(io);




	
	ESP::DrawESP();
	if (GameData.Config.Menu.Show)
	{
		newMenu::newmenu(GameData.Config.Overlay.hWnd);
		if (GameData.Config.ESP.Mouse)
		{
			if (GameData.Config.Overlay.pt.x != 0 && GameData.Config.Overlay.pt.y != 0)
			{
				ImDrawList* draw = ImGui::GetForegroundDrawList();
				ImVec2 center((float)GameData.Config.Overlay.pt.x, (float)GameData.Config.Overlay.pt.y);

				// 参数配置
				float radius = 8.0f;
				float crossSize = 10.0f;
				ImU32 fillColor = IM_COL32(255, 50, 50, 220);
				ImU32 borderColor = IM_COL32(28, 28, 28, 255);

				// 绘制中心圆
				draw->AddCircleFilled(center, radius, fillColor);
				draw->AddCircle(center, radius, borderColor, 0, 2.0f);

				// 绘制十字线
				draw->AddLine(ImVec2(center.x - crossSize, center.y),
					ImVec2(center.x + crossSize, center.y), borderColor, 1.5f);
				draw->AddLine(ImVec2(center.x, center.y - crossSize),
					ImVec2(center.x, center.y + crossSize), borderColor, 1.5f);
			}
		}
	}
	if (GameData.Config.ESP.DataSwitch)
	{
		newMenu::Render(GameData.Config.Overlay.hWnd);
	}
	
	EndDraw(io);
	return true;
}
std::string g_ver = "3.3";  //版本号

bool g_isImGuiInitialized = false;

// 初始化ImGui的函数
bool InitializeImGui(HWND hwnd) {
	if (g_isImGuiInitialized) return true; // 已初始化则直接返回

	// 创建ImGui上下文
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// 设置ImGui配置
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// 加载字体和设置样式
	ImFontConfig cfg;
	ImFontConfig cfg_regular;
	// 修改这里，添加回Bitmap标志
	cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint |
		ImGuiFreeTypeBuilderFlags_LightHinting |
		ImGuiFreeTypeBuilderFlags_LoadColor |
		ImGuiFreeTypeBuilderFlags_Bitmap;  // 添加回这个标志

	cfg_regular.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint |
		ImGuiFreeTypeBuilderFlags_LightHinting |
		ImGuiFreeTypeBuilderFlags_LoadColor;



	// 菜单 UI：18px 使用完整简体字库，避免缺字显示为 ?
	cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

	ImFontConfig cfg_menu = cfg;
	cfg_menu.OversampleH = 1;
	cfg_menu.OversampleV = 1;
	cfg_menu.PixelSnapH = true;
	const ImWchar* cnFull = io.Fonts->GetGlyphRangesChineseFull();
	const ImWchar* cnCommon = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();

	ImFont* menuUiFull = io.Fonts->AddFontFromFileTTF(
		"c:/windows/fonts/msyh.ttc", 18.f, &cfg_menu, cnFull);
	ImFont* menuHeaderFull = io.Fonts->AddFontFromFileTTF(
		"c:/windows/fonts/msyhbd.ttc", 18.f, &cfg_menu, cnFull);
	for (int i = 0; i < font::kMenuFontCount; ++i) {
		font::menu_ui_sizes[i] = menuUiFull;
		font::menu_header_sizes[i] = menuHeaderFull ? menuHeaderFull : menuUiFull;
	}
	font::menu_ui = menuUiFull;
	font::menu_ui_bold = menuHeaderFull ? menuHeaderFull : menuUiFull;

	font::calibri_bold = font::menu_ui_bold;
	font::calibri_regular = font::menu_ui;
	font::calibri_bold_hint = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyhbd.ttc", 18.f, &cfg, cnFull);


	font::myth_bold = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyhbd.ttc", 20.f, nullptr, cnCommon);
	font::myth_bold_b = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyhbd.ttc", 26.f, &cfg, cnCommon);


	{ // font


		font::icomoon_default = io.Fonts->AddFontFromMemoryTTF(icomoon_sizeof, sizeof(icomoon_sizeof), 35.f, &cfg, cnCommon);
		font::icomoon_menu = io.Fonts->AddFontFromMemoryTTF(icomoon_sizeof, sizeof(icomoon_sizeof), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		font::pixel_7_small = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 14.f, &cfg, cnCommon);
		font::icomoon = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), 20, &cfg, io.Fonts->GetGlyphRangesCyrillic());		//font::weapon_val = io.Fonts->AddFontFromMemoryTTF(weapon_icon, sizeof(weapon_icon), 55, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		font::weapon_pubg = io.Fonts->AddFontFromMemoryTTF(weapon_icon, sizeof(weapon_icon), 30, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		
	}

	// 常规字体：default 用完整字库供中文 UI 回退
	io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 19.f, &cfg_regular, cnFull);
	default_font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 19.f, &cfg_regular, cnFull);
	big_font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 28.f, &cfg_regular, cnCommon);

	// 粗体字体使用微软雅黑粗体 (msyhbd.ttc)
	bold_font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyhbd.ttc", 21.f, &cfg, cnFull);
	bold_big_font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyhbd.ttc", 23.f, &cfg, cnCommon);
	big_icon = io.Fonts->AddFontFromMemoryTTF(&icomoon1, sizeof icomoon1, 35, NULL, io.Fonts->GetGlyphRangesCyrillic());

	// 图标字体保持不变
	icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon1, sizeof icomoon1, 22, NULL, io.Fonts->GetGlyphRangesCyrillic());
	medium_icon_font = io.Fonts->AddFontFromMemoryTTF(&icomoon1, sizeof icomoon1, 32, NULL, io.Fonts->GetGlyphRangesCyrillic());
	icon_small = io.Fonts->AddFontFromMemoryTTF(&icomoon1, sizeof icomoon1, 18, NULL, io.Fonts->GetGlyphRangesCyrillic());
	dot_font = io.Fonts->AddFontFromMemoryTTF(&icomoon1, sizeof icomoon1, 7, NULL, io.Fonts->GetGlyphRangesCyrillic());

	// 菜单字体兜底，避免加载失败时空指针崩溃
	if (!font::menu_ui) {
		for (int i = 0; i < font::kMenuFontCount; ++i) {
			if (font::menu_ui_sizes[i]) { font::menu_ui = font::menu_ui_sizes[i]; break; }
		}
	}
	if (!font::menu_ui) font::menu_ui = default_font;
	if (!font::menu_ui_bold) {
		for (int i = 0; i < font::kMenuFontCount; ++i) {
			if (font::menu_header_sizes[i]) { font::menu_ui_bold = font::menu_header_sizes[i]; break; }
		}
	}
	if (!font::menu_ui_bold) font::menu_ui_bold = bold_font ? bold_font : font::menu_ui;
	font::calibri_bold = font::menu_ui_bold;
	font::calibri_regular = font::menu_ui;
	if (!font::calibri_bold_hint) font::calibri_bold_hint = font::menu_ui_bold;

	io.Fonts->Build();
	ImGui::GetIO().FontDefault = font::menu_ui ? font::menu_ui : default_font;

	// 设置样式
	ImGui::StyleColorsDark();
	ImGuiSetStyle();

	// 初始化平台/渲染器后端
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	g_isImGuiInitialized = true;
	return true;
}

// 清理ImGui的函数
void CleanupImGui() {
	if (!g_isImGuiInitialized) return;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	g_isImGuiInitialized = false;
}




int Overlay::Init(HWND TargetWnd, DRAW_PROC DrawProc, int Width, int Height)
{


	ImGui_ImplWin32_EnableDpiAwareness();
	HWND DesktopWindow = GetDesktopWindow();

	if (!TargetWnd) {
		TargetWnd = DesktopWindow;
	}
	if (!Width || !Height)
	{
		RECT rect = { 0, 0, 0, 0 };
		::GetClientRect(DesktopWindow, &rect);
		Width = rect.right - rect.left;
		Height = rect.bottom - rect.top;
		GameData.Config.Overlay.ScreenWidth = (rect.right - rect.left);
		GameData.Config.Overlay.ScreenHeight = (rect.bottom - rect.top);
		GameData.Config.Overlay.ScreenX = GameData.Config.Overlay.ScreenWidth / 2;
		GameData.Config.Overlay.ScreenY = GameData.Config.Overlay.ScreenHeight / 2;
	}

	WNDCLASSEXW wc;
	memset(&wc, 0, sizeof(WNDCLASSEXW));
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = NULL;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)(RGB(0, 0, 0));
	wc.lpszClassName = L"CEF-OSC-WIDGET";
	wc.hIconSm = NULL;
	RegisterClassExW(&wc);
	HWND hwnd = CreateWindowEx((WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED),
		wc.lpszClassName, L"NVIDIA GeForce Overlay", WS_POPUP,
		0, 0, GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN), 0, 0, 0, 0);

	SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, 2);
	MARGINS margins = { -1 };
	::DwmExtendFrameIntoClientArea(hwnd, &margins);
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	if (!CreateDeviceD3D(hwnd)) {
		CleanupDeviceD3D();
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
		MessageBox(nullptr, L"d3d fail!", L"", MB_OK);
		return 0;
	}

	GameData.Config.Overlay.hWnd = hwnd;
	InitializeImGui(hwnd);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)Width, (float)Height);

	D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
	if (texture::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &texture::logo, 0);
	if (texture::playermoder == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, playermoder_size, sizeof(playermoder_size), &info, pump, &texture::playermoder, 0);
	if (texture::weapon_image == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, weapon_image, sizeof(weapon_image), &info, pump, &texture::weapon_image, 0);
	if (texture::rank == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, ranking, sizeof(ranking), &info, pump, &texture::rank, 0);
	if (texture::item == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, item, sizeof(item), &info, pump, &texture::item, 0);
	if (texture::bq == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, bq, sizeof(bq), &info, pump, &texture::bq, 0);
	if (texture::sj == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, sj, sizeof(sj), &info, pump, &texture::sj, 0);
	if (texture::lj == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, lj, sizeof(lj), &info, pump, &texture::lj, 0);
	if (texture::jq == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, jq, sizeof(jq), &info, pump, &texture::jq, 0);
	if (texture::sdq == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, sdq, sizeof(sdq), &info, pump, &texture::sdq, 0);
	if (texture::sq == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, sq, sizeof(sq), &info, pump, &texture::sq, 0);
	if (texture::cfq == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, cfq, sizeof(cfq), &info, pump, &texture::cfq, 0);
	if (texture::pj == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, pj, sizeof(pj), &info, pump, &texture::pj, 0);
	if (texture::yp == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, yp, sizeof(yp), &info, pump, &texture::yp, 0);
	if (texture::fj == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, fj, sizeof(fj), &info, pump, &texture::fj, 0);
	if (texture::zd == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, zd, sizeof(zd), &info, pump, &texture::zd, 0);
	if (texture::tzw == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, tzw, sizeof(tzw), &info, pump, &texture::tzw, 0);
	if (texture::ys == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, ys, sizeof(ys), &info, pump, &texture::ys, 0);

	ShaderResource();

	PNSImG();

	ClickThrough(false);

	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.f);

	Throttler Throttlered;
	CameraData Camera;
	float TimeSeconds = 0.f;
	bool is = true;
	while (is)
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				is = false;
		}
		if (GameData.Config.Overlay.hWnd == hwnd)
		{
			SetWindowPos(hwnd, (HWND)-1, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		if (!is)
			break;

		if (TargetWnd != DesktopWindow)
		{
			RECT rect = { 0, 0, 0, 0 };
			::GetClientRect(TargetWnd, &rect);
			is = DrawProc((int)(rect.right - rect.left), (int)(rect.bottom - rect.top), io);
		}
		else
		{
			is = DrawProc((int)io.DisplaySize.x, (int)io.DisplaySize.y, io);
		}
		if (GameData.Config.ESP.PhysXDebug && GameData.Scene == Scene::Gaming) {
			std::atomic_store(&GameData.NextHintMeshData, LineTrace::getNextHint());
		}
		if (GameData.Config.Window.IsLogin) {
			if (!GameData.Config.Overlay.UseThread) {
				auto hScatter = mem.CreateScatterHandle();
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

				mem.CloseScatterHandle(hScatter);
			}
		}
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupImGui();
	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	GameData.Config.Window.Main = false;

	return 0;
}

int Overlay::login() {
	// 验证已移除，直接设置登录状态并返回
	GameData.Config.Window.IsLogin = true;
	return 0;

	// 以下代码已跳过
	static char Pwd[40] = { 0 };
	static bool Save = 1;
	static char NewPwd[40] = { 0 };
	static char SuperPwd[40] = { 0 };
	static char Key[40] = { 0 };


	static bool is_config_loaded_login = false;
	if (!is_config_loaded_login) {
		std::ifstream configFile("Key.ini");
		if (configFile.is_open()) {
			std::string line;
			while (std::getline(configFile, line)) {
				if (line.empty() || line[0] == '[' || line[0] == '#') {
					continue;
				}

				if (line.find("save=") == 0) {
					Save = std::stoi(line.substr(5)) == 1;
				}

				if (Save) {
					if (line.find("user=") == 0) {
						std::strncpy(GameData.Acct, line.substr(5).c_str(), sizeof(GameData.Acct) - 1);
					}

					if (line.find("pass=") == 0) {
						std::strncpy(Pwd, line.substr(5).c_str(), sizeof(Pwd) - 1);
					}
				}
			}
			configFile.close();
		}
		is_config_loaded_login = true;
	}


	WNDCLASSEXW wc;
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = nullptr;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = L"ImGui";
	wc.lpszClassName = L"Example";
	wc.hIconSm = LoadIcon(0, IDI_APPLICATION);


	RegisterClassExW(&wc);
	hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"Loader", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (WIDTH / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (HEIGHT / 2), WIDTH, HEIGHT, 0, 0, 0, 0);

	SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	POINT mouse;
	rc = { 0 };
	GetWindowRect(hwnd, &rc);

	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	InitializeImGui(hwnd);
	SetWindowRgn(hwnd, CreateRoundRectRgn(0, 0, WIDTH, HEIGHT, 51, 51), FALSE);

	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	if (!Utils::IsDiagnosticLoggingEnabled())
		::ShowWindow(GetConsoleWindow(), SW_HIDE);
	::UpdateWindow(hwnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig cfg;
	ImFontConfig cfg_regular;
	cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;
	cfg_regular.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;


	ImGui::StyleColorsDark();
	ImGuiSetStyle();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.f);

	ImGuiStyle& s = ImGui::GetStyle();

	s.FramePadding = ImVec2(5, 5);
	s.ItemSpacing = ImVec2(20, 20);
	s.WindowPadding = ImVec2(0, 0);
	s.FrameRounding = 10.f;
	s.WindowRounding = 30.f;
	s.WindowBorderSize = 0.f;
	s.PopupBorderSize = 0.f;
	s.WindowPadding = ImVec2(0, 0);
	s.ChildBorderSize = 10;

	bool done = false;
	bool loginSuccess = false;
	bool shouldExitImmediately = false; // 新增标志

	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		{



			LoadImages();
			ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));
			ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
			{
				auto draw = ImGui::GetWindowDrawList();
				const auto& p = ImGui::GetWindowPos();

				move_window();

				anim_speed = ImGui::GetIO().DeltaTime * 8.f;

				static int menu_state = 0;
				static int state_autorization = 0;




				static bool is_verification_initialized = false;



				// 初始化SP Cloud验证系统（只初始化一次）
				if (!is_verification_initialized) {
					g_Ctx_Cloud = SP_Cloud_Create();
					if (!g_Ctx_Cloud) {
						MessageBoxA(0, "SP Cloud初始化失败", "验证错误", MB_ICONERROR);
						ExitProcess(1);
					}
					// TODO: 请修改为你的服务器IP和端口
					SP_CloudSetConnInfo(g_Ctx_Cloud, "kaka", "116.198.248.174", 1000, 30 * 1000, 1, false);
					is_verification_initialized = true;

				}

				draw->AddRectFilled(p, p + ImVec2(WIDTH, HEIGHT), theme_a::winbg, 32);

				static float rotation;
				rotation = ImLerp(rotation, rotation + 0.02f, anim_speed);

				Trinage_background();

				static float logotype_offset;
				static float product_page_offset = -500.f;

				logotype_offset = ImLerp(logotype_offset, state_autorization == 3 ? 476 : 0.f, anim_speed);
				product_page_offset = ImLerp(product_page_offset, menu_state == 2 ? 0.f : -500.f, anim_speed);

				if (logotype_offset > 475.f)
					menu_state = 2;

				const char* product_icons[4] = { "d", "g", "t", "e" };
				const char* products_text[4] = { "Bow Club", " Bow Club", "Bow Club", "Bow Club" };

				if (menu_state == 0) {
					static bool login_page = true;
					static float login_page_offset = 0.f;
					static float reg_page_offset = 0.f;
					static float reset_page_offset = 0.f;
					static float recharge_page_offset = 0.f;
					static float images_offset = 0.f;
					static int current_auth_page = 0; // 0=登录, 1=注册, 2=改密, 3=充值

					if (g_asyncRequest.is_completed) {
						if (g_asyncRequest.is_success) {
							// 操作成功
							notifications::Message(g_asyncRequest.success_message.c_str(), "s");

							// 根据操作类型执行相应的后续处理
							switch (g_asyncRequest.operation) {
							case OP_LOGIN:
								// 保存配置
							{
								loginSuccess = true;
								std::ofstream configFile("Key.ini");
								if (configFile.is_open()) {
									configFile << "save=" << (Save ? "1" : "0") << "\n";
									if (Save) {
										configFile << "user=" << GameData.Acct << "\n";
										configFile << "pass=" << Pwd << "\n";
									}
									configFile.close();
								}
							}
							state_autorization = 3;
							// 启动心跳线程
							CreateThread(0, 0, LPTHREAD_START_ROUTINE(SPHeartbeat), 0, 0, 0);
							break;

							case OP_REGISTER:
							case OP_CHANGE_PASSWORD:
								if (g_asyncRequest.current_auth_page_result >= 0) {
									current_auth_page = g_asyncRequest.current_auth_page_result;
								}
								break;
							}
						}
						else {
							// 操作失败
							notifications::Message(g_asyncRequest.error_message.c_str(), "w");

							// 根据操作类型执行相应的错误处理
							switch (g_asyncRequest.operation) {
							case OP_LOGIN:
							case OP_REGISTER:
								theme_a::inputtext_color[0] = ImColor(35, 24, 30, 255);
								theme_a::inputtext_color[1] = ImColor(201, 80, 80, 255);
								theme_a::inputtext_color[2] = ImColor(201, 80, 80, 255);
								state_autorization = 2;
								break;
							}
						}

						// 重置状态
						g_asyncRequest.is_completed = false;
						g_asyncRequest.operation = OP_NONE;

						// 关闭线程句柄
						if (g_operationThread) {
							CloseHandle(g_operationThread);
							g_operationThread = nullptr;
						}
					}


					// 计算页面偏移
					login_page_offset = ImLerp(login_page_offset, current_auth_page == 0 ? 0.f : 500.f, anim_speed);
					reg_page_offset = ImLerp(reg_page_offset, current_auth_page == 1 ? 0.f : 500.f, anim_speed);
					reset_page_offset = ImLerp(reset_page_offset, current_auth_page == 2 ? 0.f : 500.f, anim_speed);
					recharge_page_offset = ImLerp(recharge_page_offset, current_auth_page == 3 ? 0.f : 500.f, anim_speed);
					images_offset = ImLerp(images_offset, state_autorization == 3 ? 450.f : 0.f, anim_speed);

					if (state_autorization == 2)
					{
						static DWORD dwTickStart = GetTickCount();
						if (GetTickCount() - dwTickStart > 5000)
						{
							theme_a::inputtext_color[0] = theme_a::background_color;
							theme_a::inputtext_color[1] = theme_a::main_color;
							theme_a::inputtext_color[2] = theme_a::text_color[0];
							state_autorization = 0;
							dwTickStart = GetTickCount();
						}
					}

					static float icons_alpha[4];
					for (int i = 0; i < 4; i++) {
						images_alpha[i] = ImLerp(images_alpha[i], i == current_image ? 1.f : 0.f, anim_speed);

						// 修复向量运算 - 手动计算坐标
						ImVec2 img_min = ImVec2(p.x - 12 - images_offset, p.y - 0);
						ImVec2 img_max = ImVec2(p.x + 325 - images_offset, p.y + 550 - 0);

						draw->AddImageRounded(back_images[i], img_min, img_max, ImVec2(0, 0), ImVec2(1, 1),
							ImColor(1.f, 1.f, 1.f, images_alpha[i]), -100.f);
					}

					// 修复渐变矩形的坐标计算
					ImVec2 gradient_min = ImVec2(p.x - 15 + images_offset, p.y - 0);
					ImVec2 gradient_max = ImVec2(p.x + 325 - images_offset, p.y + 550);

					draw->AddRectFilledMultiColor(
						gradient_min,
						gradient_max,
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f),
						theme_a::winbg,
						theme_a::winbg,
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f)
					);

					// 显示最新公告
					if (state_autorization != 3) {
						ImVec2 notice_pos = ImVec2(p.x + 30 - images_offset, p.y + 80);
						ImVec2 bg_min = notice_pos + ImVec2(-10, 0);
						ImVec2 bg_max = bg_min + ImVec2(280, 355);
						draw->AddRectFilled(bg_min, bg_max, ImColor(0, 0, 0, 180), 8);

						draw->AddText(font::myth_bold, 28.f, notice_pos, ImColor(255, 255, 255, 240), U8("系统公告"));
						draw->AddLine(notice_pos + ImVec2(0, 35), notice_pos + ImVec2(200, 35), ImColor(255, 255, 255, 100));

						ImGui::PushFont(font::calibri_regular);
						ImVec2 text_pos = notice_pos + ImVec2(0, 45);
						draw->AddText(font::calibri_regular, 18.f, text_pos + ImVec2(1, 1), ImColor(0, 0, 0, 180), g_NoticeText.c_str(), NULL, 260.0f);
						draw->AddText(font::calibri_regular, 18.f, text_pos, ImColor(230, 230, 230, 255), g_NoticeText.c_str(), NULL, 260.0f);
						ImGui::PopFont();
					}

					// 登录页面
// 在登录页面添加"记住密码"复选框
					if (login_page_offset < 400.f) {
						ImGui::SetCursorPos(ImVec2(388 + login_page_offset, 135));
						ImGui::BeginGroup();

						ImGui::NewInputTextEx(U8("账号"), "c", GameData.Acct, 128, ImVec2(350, 50), 0);
						ImGui::NewInputTextEx(U8("密码"), "b", Pwd, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);

						if (ImGui::NewButton(U8("登录"), ImVec2(350, 50), ""))
						{
							if (!g_asyncRequest.is_processing) {
								notifications::Message(U8("正在登录，请稍候..."), "s");
								StartAsyncOperation(OP_LOGIN, GameData.Acct, Pwd);
							}
						}
						// 充值按钮 - 左对齐输入框
						ImGui::SetCursorPos(ImVec2(388 + login_page_offset, ImGui::GetCursorPos().y));
						if (ImGui::NewButton(U8("充值"), ImVec2(170, 50), ""))
						{
							current_auth_page = 3;
						}

						// 解绑按钮 - 右对齐输入框
						ImGui::SameLine();
						static bool needConfirm = false;

						if (ImGui::NewButton(needConfirm ? U8("确认解绑?") : U8("解绑"),
							ImVec2(170, 50), "9"))
						{
							if (!needConfirm) {
								needConfirm = true;
								notifications::Message(U8("再次点击确认解绑"), "w");
							}
							else {
								if (!g_asyncRequest.is_processing) {
									notifications::Message(U8("正在解绑，请稍候..."), "s");
									StartAsyncOperation(OP_UNBIND, GameData.Acct);
								}
								needConfirm = false;
							}
						}

						// 点击其他地方或5秒后重置
						static float lastClickTime = 0.0f;
						if (needConfirm) {
							if (ImGui::GetTime() - lastClickTime > 5.0f || ImGui::IsMouseClicked(0)) {
								if (!ImGui::IsItemHovered()) {
									needConfirm = false;
								}
							}
						}
						if (needConfirm) {
							lastClickTime = ImGui::GetTime();
						}

						// 添加一些垂直间距
						ImGui::Dummy(ImVec2(0, 10));

						// "没有账号?注册" 按钮 - 在充值按钮下方（左对齐）
						ImGui::SetCursorPos(ImVec2(388 + login_page_offset, ImGui::GetCursorPos().y));
						ImGui::TextDisabled(U8(""));
						ImGui::SameLine(0, 0);
						if (button_text(U8(""), U8("注册")))
						{
							current_auth_page = 1;
						}

						// "忘记密码?改密" 按钮 - 在解绑按钮下方（右对齐）
						ImGui::SameLine();
						ImGui::SetCursorPosX(388 + login_page_offset + 180); // 与解绑按钮右对齐
						if (button_text(U8("  "), U8(" 改密 ")))
						{
							current_auth_page = 2;
						}
						ImGui::EndGroup();
					}


					if (reg_page_offset < 400.f)
					{
						ImGui::SetCursorPos(ImVec2(388 + reg_page_offset, 135));
						ImGui::BeginGroup();

						ImGui::NewInputTextEx(U8("账号"), "c", GameData.Acct, 128, ImVec2(350, 50), 0);
						ImGui::NewInputTextEx(U8("密码"), "b", Pwd, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);
						ImGui::NewInputTextEx(U8("超级密码"), "b", SuperPwd, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);
						ImGui::NewInputTextEx(U8("卡密"), "b", Key, 128, ImVec2(350, 50), 0);

						if (ImGui::NewButton(U8("注册"), ImVec2(350, 50), " "))
						{
							if (!g_asyncRequest.is_processing) {
								notifications::Message(U8("正在注册，请稍候..."), "s");
								StartAsyncOperation(OP_REGISTER, GameData.Acct, Pwd, "", SuperPwd, Key);
							}
						}

						// 添加一些垂直间距
						ImGui::Dummy(ImVec2(0, 5));

						// 返回登录按钮 - 居中显示
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (350 - CalcTextSize("Have account? LOG IN").x) / 2);
						if (button_text(U8(""), U8(" 登入")))
						{
							current_auth_page = 0;
						}

						ImGui::Dummy(ImVec2(0, 10));

						ImGui::EndGroup();
					}

					// 改密页面
					if (reset_page_offset < 400.f)
					{
						ImGui::SetCursorPos(ImVec2(388 + reset_page_offset, 135));
						ImGui::BeginGroup();

						ImGui::NewInputTextEx(U8("账号"), "c", GameData.Acct, 128, ImVec2(350, 50), 0);
						ImGui::NewInputTextEx(U8("超级密码"), "b", SuperPwd, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);
						ImGui::NewInputTextEx(U8("新密码"), "b", NewPwd, 128, ImVec2(350, 50), ImGuiInputTextFlags_Password);

						if (ImGui::NewButton(U8("改密"), ImVec2(350, 50), " "))
						{
							if (!g_asyncRequest.is_processing) {
								notifications::Message(U8("正在修改密码，请稍候..."), "s");
								StartAsyncOperation(OP_CHANGE_PASSWORD, GameData.Acct, "", NewPwd, SuperPwd);
							}
						}

						ImGui::SetCursorScreenPos(ImVec2(center_text(p + ImVec2(388 + reset_page_offset, 0), p + ImVec2(388 + 350 + reset_page_offset, 0), " Back to GameData.Acct").x, ImGui::GetCursorPos().y));
						if (button_text(" ", U8("返回登录")))
						{
							current_auth_page = 0;
						}
						ImGui::EndGroup();
					}

					// 充值页面
					if (recharge_page_offset < 400.f)
					{
						ImGui::SetCursorPos(ImVec2(388 + recharge_page_offset, 135));
						ImGui::BeginGroup();

						ImGui::NewInputTextEx(U8("账号"), "c", GameData.Acct, 128, ImVec2(350, 50), 0);
						ImGui::NewInputTextEx(U8("卡密"), "b", Key, 128, ImVec2(350, 50), 0);


						if (ImGui::NewButton(U8("充值"), ImVec2(350, 50), " 1"))
						{
							if (!g_asyncRequest.is_processing) {
								notifications::Message(U8("正在充值，请稍候..."), "s");
								StartAsyncOperation(OP_RECHARGE, GameData.Acct, "", "", "", Key);
							}
						}

						ImGui::SetCursorScreenPos(ImVec2(center_text(p + ImVec2(388 + recharge_page_offset, 0), p + ImVec2(388 + 350 + recharge_page_offset, 0), " Back to GameData.Acct").x, ImGui::GetCursorPos().y));
						if (button_text(" ", U8("返回登录")))
						{
							current_auth_page = 0;
						}
						ImGui::EndGroup();
					}

					PushFont(icon_font);
					if (current_image < 3) {
						ImGui::SetCursorPos(ImVec2(282 - images_offset, 261));
						ImGui::TextColored(theme_a::text_color[1], "1");
						if (ImGui::IsItemClicked())
							current_image++;
					}

					if (current_image > 0) {
						ImGui::SetCursorPos(ImVec2(20 - images_offset, 261));
						ImGui::TextColored(theme_a::text_color[1], "2");
						if (ImGui::IsItemClicked())
							current_image--;
					}
					PopFont();

					draw->AddShadowCircle(p + ImVec2(WIDTH / 2, HEIGHT + 150), 150.f, theme_a::main_color_shadow, 750.f, ImVec2(0, 0));
				}

				if (menu_state == 2)
				{
					PushFont(bold_font);

					// 计算文本居中位置
					ImVec2 center_pos = center_text(p, ImVec2(p.x + WIDTH, p.y + 0), GameData.Acct);
					ImVec2 text_pos = ImVec2(center_pos.x, p.y + 50.f - ImGui::GetScrollY());
					ImVec2 text_size = CalcTextSize(GameData.Acct);

					// 修复阴影矩形坐标
					ImVec2 shadow_min = ImVec2(text_pos.x + 0, text_pos.y + 5);
					ImVec2 shadow_max = ImVec2(text_pos.x + text_size.x - 0, text_pos.y + text_size.y - 3);
					draw->AddShadowRect(shadow_min, shadow_max,
						ImColor(theme_a::main_color.Value.x, theme_a::main_color.Value.y, theme_a::main_color.Value.z, 0.45f),
						35.f, ImVec2(0, 0));

					draw->AddText(text_pos, theme_a::main_color, GameData.Acct);
					PopFont();

					PushFont(bold_big_font);
					ImVec2 bow_center = center_text(p, ImVec2(p.x + WIDTH, p.y + 0), U8("哒哒哒"));
					//draw->AddText(ImVec2(bow_center.x, p.y + 68.f - ImGui::GetScrollY()), theme_a::text_color[0], U8("ABC漏打"));
					PopFont();

					ImGui::SetCursorPos(ImVec2(50 + product_page_offset, 135));
					ImGui::BeginGroup();
					std::string expiry_display = GameData.ExperTime.empty() ? U8("未登录") : GameData.ExperTime;

					if (ImGui::Product("", product_images[0], 0, expiry_display.c_str(), "PUBG", "Yesterday"))
					{
						notifications::Message(U8("正在启动"), "s");

						// 正确关闭ImGui组件
						//ImGui::EndGroup();
						//ImGui::End();

						// 短暂延迟让用户看到"正在启动"消息
						Sleep(1000);

						// 设置标志，让主循环退出
						done = true;
						GameData.Config.Window.IsLogin = true;
						//GameData.Config.Window.Main = true;

						//return 0;  // 返回登录成功
					}

					ImGui::Dummy(ImVec2(700, 110));
					ImGui::EndGroup();

					// 修复阴影圆圈坐标
					ImVec2 shadow_circle_pos = ImVec2(p.x + WIDTH * 0.5f, p.y + HEIGHT + 150);
					draw->AddShadowCircle(shadow_circle_pos, 150.f, theme_a::main_color_shadow, 750.f, ImVec2(0, 0));

					// 修复渐变矩形坐标
					ImVec2 gradient1_min = ImVec2(p.x + 45 + product_page_offset, p.y + HEIGHT - 50);
					ImVec2 gradient1_max = ImVec2(p.x + WIDTH - 45 - product_page_offset, p.y + HEIGHT);
					draw->AddRectFilledMultiColor(gradient1_min, gradient1_max,
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f),
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f),
						theme_a::winbg, theme_a::winbg);

					ImVec2 gradient2_min = ImVec2(p.x + 45 + product_page_offset, p.y + 0);
					ImVec2 gradient2_max = ImVec2(p.x + WIDTH - 45 - product_page_offset, p.y + 45);
					draw->AddRectFilledMultiColor(gradient2_min, gradient2_max,
						theme_a::winbg, theme_a::winbg,
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f),
						ImColor(theme_a::winbg.Value.x, theme_a::winbg.Value.y, theme_a::winbg.Value.z, 0.f));
				}

				// 修复阴影圆圈坐标计算
				ImVec2 shadow_pos = ImVec2(p.x + 563 - logotype_offset, p.y + 67 - ImGui::GetScrollY());
				draw->AddShadowCircle(shadow_pos, 30.f, ImColor(theme_a::main_color.Value.x, theme_a::main_color.Value.y, theme_a::main_color.Value.z, 0.5f), 75.f, ImVec2(0, 0));


				ImGui::SetCursorPos(ImVec2(750, 20));
				ImGui::PushFont(bold_big_font);
				if (button_text(U8(""), U8("X")))
				{
					// 让桌面和任务栏重新显示
					HWND Progman = FindWindowA("Progman", NULL);
					HWND TrayWnd = FindWindowA("Shell_TrayWnd", NULL);
					if (Progman) ShowWindow(Progman, SW_SHOW);
					if (TrayWnd) ShowWindow(TrayWnd, SW_SHOW);

					// 强制终止当前进程
					TerminateProcess(GetCurrentProcess(), 1);
				}
				ImGui::PopFont();

				const char* logo_text = U8("PUBG-哒哒哒 (二改keke)");
				float right_offset = 20.0f;  // 向右偏移的像素数

				// 正确的方法
				ImGui::PushFont(bold_big_font);  // 先设置字体
				ImVec2 text_size = ImGui::CalcTextSize(logo_text);
				ImVec2 center_pos = ImVec2(p.x + 531 + 17.5f - logotype_offset - text_size.x / 2 + right_offset,
					p.y + 34 + 17.5f + 2 - text_size.y / 2 - ImGui::GetScrollY());
				draw->AddText(center_pos, ImColor(255, 255, 255, 255), logo_text);
				ImGui::PopFont();  // 恢复字体

				ImRotateStart();
				draw->AddShadowCircle(p + ImVec2(WIDTH + 150, HEIGHT / 2), 200.f, theme_a::main_color_shadow, 600.f, ImVec2(0, 0));
				draw->AddShadowCircle(p + ImVec2(WIDTH + 150, HEIGHT / 2 - 100.f), 200.f, theme_a::main_color_shadow, 600.f, ImVec2(0, 0));
				draw->AddShadowCircle(p + ImVec2(WIDTH + 450, HEIGHT / 2), 200.f, theme_a::main_color_shadow, 600.f, ImVec2(0, 0));
				draw->AddShadowCircle(p + ImVec2(WIDTH + 450, HEIGHT / 2 + 100.f), 200.f, theme_a::main_color_shadow, 600.f, ImVec2(0, 0));
				ImRotateEnd(rotation);

				notifications::NotifyUpdate(p);
			}

			ImGui::Render();
			const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
			g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
			g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			g_pSwapChain->Present(1, 0); // Present with vsync
		}

	}

	// 清理登录界面资源EndDraw(io);
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	if (g_operationThread) {
		// 等待线程完成
		WaitForSingleObject(g_operationThread, 3000); // 最多等待3秒
		CloseHandle(g_operationThread);
		g_operationThread = nullptr;
	}


	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	g_isLoginUIActive = false;

	// 返回登录是否成功
	return loginSuccess ? 0 : 1;
}

bool CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & 16384)
		{

			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
