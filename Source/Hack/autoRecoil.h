#pragma once
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"      // 假设这些头文件路径正确
#include "common/Entitys.h"
#include "utils/KmBox.h"
#include "utils/Lurker.h"
#include "utils/KmBoxNet.h"
#include "utils/MoBox.h"
#include <cmath> // 用于 fabsf

//class Recoil
//{
//public:
//	/**
//	 * @brief 根据配置将鼠标移动指令分发给对应的硬件控制器
//	 * @param X 横向移动距离
//	 * @param Y 纵向移动距离
//	 */
//	static void Move(int X, int Y)
//	{
//		if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
//		{
//			return;
//		}
//
//		switch (GameData.Config.AimBot.Controller) {
//		case 0:
//			KmBox::Move(X, Y);
//			break;
//		case 1:
//			KmBoxNet::Move(X, Y);
//			break;
//		case 2:
//			Lurker::Move(X, Y);
//			break;
//		case 3:
//			MoBox::Move(X, Y);
//			break;
//		default:
//			return;
//		}
//	}
//
//	/**
//	 * @brief 自动后坐力补偿主函数
//	 * 该函数通过监测开火时视角的偏移量，动态计算鼠标需要反向移动的距离，以达到压枪效果。
//	 */
//	static void autoRecoil()
//	{
//		// 静态变量，用于在循环中保持状态
//		static bool isCompensating = false; // 标记当前是否正在进行后坐力补偿
//		static float initialYaw = 0.0f;     // 开火瞬间的水平视角（Yaw）
//		static float initialPitch = 0.0f;   // 开火瞬间的垂直视角（Pitch）
//		static FRotator lastRotation;       // 上一帧的视角，用于检测玩家的主动移动
//
//		// 阈值常量，可在此直接修改
//		const float YAW_RESET_THRESHOLD = 0.35f; // 当玩家单帧水平移动超过此阈值时，重置压枪基准点（用于跟枪）
//
//		auto recolHandle = mem.CreateScatterHandle();
//
//		while (true)
//		{
//			Sleep(1); // 减少CPU占用
//
//			// 总开关检查
//			if (GameData.Radar.Visibility || GameData.Config.AimBot.aimboot || !GameData.Config.AimBot.Recoilenanlek)
//			{
//				isCompensating = false; // 退出压枪状态
//				continue;
//			}
//
//			// 武器类型检查，只对特定枪械生效
//			if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::AR &&
//				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::SMG &&
//				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::LMG)
//			{
//				isCompensating = false; // 退出压枪状态
//				continue;
//			}
//
//			// 检测开火键（鼠标左键VK_LBUTTON=1）
//			if (GameData.Keyboard.IsKeyDown(1))
//			{
//				FRotator controlRotation;
//				// 为了效率，只在开火时读取内存
//				mem.AddScatterRead(recolHandle, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["ControlRotation_CP"], &controlRotation);
//				mem.ExecuteReadScatter(recolHandle);
//
//				// --- 状态管理 ---
//				if (!isCompensating)
//				{
//					// 这是开火的第一帧
//					isCompensating = true;
//					initialYaw = controlRotation.Yaw;
//					initialPitch = controlRotation.Pitch;
//					lastRotation = controlRotation;
//					continue; // 从下一帧开始计算和补偿
//				}
//
//				// --- 动态基准点重置 ---
//				// 如果玩家在开火时大幅度横向移动鼠标（比如跟枪），我们需要重置压枪的基准点
//				float yawDeltaSinceLastFrame = fabsf(controlRotation.Yaw - lastRotation.Yaw);
//				if (yawDeltaSinceLastFrame > YAW_RESET_THRESHOLD)
//				{
//					initialYaw = controlRotation.Yaw;
//					initialPitch = controlRotation.Pitch; // 同时重置垂直基准，防止拉枪后压枪位置错误
//				}
//
//				// --- 获取当前倍镜的压枪参数 (硬编码版本) ---
//				float xMultiplier = 10.0f; // 横向补偿系数 (默认值)
//				float yMultiplier = 10.0f; // 纵向补偿系数 (默认值)
//				uint32_t fov = static_cast<uint32_t>(GameData.Camera.FOV);
//
//				if (fov <= 75 && fov >= 52) { // 红点/全息
//					xMultiplier = 12.0f;
//					yMultiplier = 12.5f;
//				}
//				else if (fov == 40) { // 2倍
//					xMultiplier = 14.0f;
//					yMultiplier = 14.5f;
//				}
//				else if (fov == 26) { // 3倍
//					xMultiplier = 16.0f;
//					yMultiplier = 16.5f;
//				}
//				else if (fov == 19 || fov == 20) { // 4倍
//					xMultiplier = 18.0f;
//					yMultiplier = 18.5f;
//				}
//				else if (fov == 13) { // 6倍
//					xMultiplier = 22.0f;
//					yMultiplier = 22.5f;
//				}
//				else if (fov == 10) { // 8倍
//					xMultiplier = 28.0f;
//					yMultiplier = 28.5f;
//				}
//				// 注意: interval 和 yRecoil 仍然从您的配置文件中读取，因为您的原代码就是这样做的
//				int scopeIndex = 0;
//				if (fov <= 75 && fov >= 52) scopeIndex = 0; else if (fov == 40) scopeIndex = 1; else if (fov == 26) scopeIndex = 2; else if (fov == 19 || fov == 20) scopeIndex = 3; else if (fov == 13) scopeIndex = 4; else if (fov == 10) scopeIndex = 5;
//				int interval = GameData.Config.AimBot.interval[scopeIndex];
//
//				// --- 计算补偿值 ---
//				float yawOffset = controlRotation.Yaw - initialYaw;
//				float pitchOffset = controlRotation.Pitch - initialPitch;
//
//				// 规范化角度差，防止突变（例如从-179度跳到+179度）
//				yawOffset = (yawOffset > 180.f) ? yawOffset - 360.f : (yawOffset < -180.f) ? yawOffset + 360.f : yawOffset;
//				pitchOffset = (pitchOffset > 180.f) ? pitchOffset - 360.f : (pitchOffset < -180.f) ? pitchOffset + 360.f : pitchOffset;
//
//				// 计算鼠标需要移动的像素
//				int moveX = -static_cast<int>(yawOffset * xMultiplier);
//
//				// 这里的符号非常重要，请根据实际效果调整。
//				// 通常后坐力使Pitch值增大，所以pitchOffset会是正数。
//				// 我们需要向下移动鼠标来补偿，如果你的鼠标坐标系向下为正，那么moveY应该为正。
//				// 如果pitchOffset是正数，那么moveY的计算应该是 `pitchOffset * yMultiplier`。
//				// 你的旧代码是 `LeanLeftOffset * 10`，这里我们保持一致。
//				int moveY = static_cast<int>(pitchOffset * yMultiplier);
//
//				// --- 应用补偿 ---
//				// 方案：只使用动态压枪。这是最能体现“镜头旋转压枪”的方案。
//				Move(moveX, moveY);
//
//				Sleep(interval);
//
//				// 更新上一帧的旋转数据
//				lastRotation = controlRotation;
//			}
//			else
//			{
//				// 如果松开了左键，重置状态
//				isCompensating = false;
//			}
//		}
//
//		// 虽然在无限循环中通常走不到这里，但这是个好习惯
//		mem.CloseScatterHandle(recolHandle);
//	}
//};

class Recoil
{
public:
	/**
	 * @brief 根据配置将鼠标移动指令分发给对应的硬件控制器
	 * @param X 横向移动距离
	 * @param Y 纵向移动距离
	 */
	static void Move(int X, int Y)
	{
		if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
		{
			return;
		}

		switch (GameData.Config.AimBot.Controller) {
		case 0:
			KmBox::Move(X, Y);
			break;
		case 1:
			KmBoxNet::Move(X, Y);
			break;
		case 2:
			Lurker::Move(X, Y);
			break;
		case 3:
			MoBox::Move(X, Y);
			break;
		default:
			return;
		}
	}

	/**
	 * @brief 自动后坐力补偿主函数
	 * 该函数通过监测开火时视角的偏移量，动态计算鼠标需要反向移动的距离，以达到压枪效果。
	 */
	static void autoRecoil()
	{
		// 静态变量，用于在循环中保持状态
		static bool isCompensating = false; // 标记当前是否正在进行后坐力补偿
		static float initialYaw = 0.0f;     // 开火瞬间的水平视角（Yaw）
		static float initialPitch = 0.0f;   // 开火瞬间的垂直视角（Pitch）
		static FRotator lastRotation;       // 上一帧的视角，用于检测玩家的主动移动

		// [新增] 移动平均窗口
		static std::deque<float> pitchWindow;
		static std::deque<float> yawWindow;
		const size_t WINDOW_SIZE = 10; // 10帧的平滑窗口 (约 50-100ms)

		// 阈值常量，可在此直接修改
		const float YAW_RESET_THRESHOLD = 0.35f; // 当玩家单帧水平移动超过此阈值时，重置压枪基准点（用于跟枪）

		auto recolHandle = mem.CreateScatterHandle();

		while (true)
		{
			Sleep(1); // 减少CPU占用

			// 总开关检查
			if (GameData.Radar.Visibility || GameData.Config.AimBot.aimboot || !GameData.Config.AimBot.Recoilenanlek || GameData.AimBot.Lock)
			{
				isCompensating = false; // 退出压枪状态
				continue;
			}

			// 武器类型检查，只对特定枪械生效
			if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::AR &&
				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::SMG &&
				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::LMG)
			{
				isCompensating = false; // 退出压枪状态
				continue;
			}

			// 检测开火键（鼠标左键VK_LBUTTON=1）
			if (GameData.Keyboard.IsKeyDown(1))
			{
				FRotator controlRotation;
				// 为了效率，只在开火时读取内存
				mem.AddScatterRead(recolHandle, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["ControlRotation_CP"], &controlRotation);
				mem.ExecuteReadScatter(recolHandle);

				// --- 状态管理 ---
				if (!isCompensating)
				{
					// 这是开火的第一帧
					isCompensating = true;
					initialYaw = controlRotation.Yaw;
					initialPitch = controlRotation.Pitch;
					lastRotation = controlRotation;
					// 清空历史窗口
					pitchWindow.clear();
					yawWindow.clear();
					continue; // 从下一帧开始计算和补偿
				}

				// --- 动态基准点重置 ---
				// 如果玩家在开火时大幅度横向移动鼠标（比如跟枪），我们需要重置压枪的基准点
				float yawDeltaSinceLastFrame = fabsf(controlRotation.Yaw - lastRotation.Yaw);
				if (yawDeltaSinceLastFrame > YAW_RESET_THRESHOLD)
				{
					initialYaw = controlRotation.Yaw;
					initialPitch = controlRotation.Pitch; // 同时重置垂直基准，防止拉枪后压枪位置错误
				}

				// --- 获取当前倍镜的压枪参数 ---
				// 优先使用配置文件中的参数，并应用增量模式的放大系数
				uint32_t fov = static_cast<uint32_t>(GameData.Camera.FOV);

				int scopeIndex = 0;
				if (fov <= 75 && fov >= 52) scopeIndex = 0;      // 红点/全息
				else if (fov == 40) scopeIndex = 1;              // 2倍
				else if (fov == 26) scopeIndex = 2;              // 3倍
				else if (fov == 19 || fov == 20) scopeIndex = 3; // 4倍
				else if (fov == 13) scopeIndex = 4;              // 6倍
				else if (fov == 10) scopeIndex = 5;              // 8倍

				// 从配置读取系数 (这样您可以在菜单中调节强度)
				// 如果配置值为0或异常，给予默认值保护
				float configX = (float)GameData.Config.AimBot.xRecoil[scopeIndex];
				float configY = (float)GameData.Config.AimBot.yRecoil[scopeIndex];

				if (configX <= 0.1f) configX = 15.0f;
				if (configY <= 0.1f) configY = 15.0f;

				float xMultiplier = configX;
				float yMultiplier = configY;

				int interval = GameData.Config.AimBot.interval[scopeIndex];

				// --- 计算补偿值 (增量模式) ---
				// 计算当前帧与上一帧的视角差值
				float yawDiff = controlRotation.Yaw - lastRotation.Yaw;
				float pitchDiff = controlRotation.Pitch - lastRotation.Pitch;

				// 规范化角度差
				if (yawDiff > 180.f) yawDiff -= 360.f;
				else if (yawDiff < -180.f) yawDiff += 360.f;

				if (pitchDiff > 180.f) pitchDiff -= 360.f;
				else if (pitchDiff < -180.f) pitchDiff += 360.f;

				// [用户建议]: 检测枪的状态，看它开枪是一个什么速度窗口往上抬，然后压一下垂直轴，横轴轻微压。
				// 实现方案：引入移动平均 (Moving Average) 来计算"速度窗口"，过滤掉单帧的剧烈抖动。

				// 1. 更新滑动窗口
				pitchWindow.push_back(pitchDiff);
				if (pitchWindow.size() > WINDOW_SIZE) pitchWindow.pop_front();

				yawWindow.push_back(yawDiff);
				if (yawWindow.size() > WINDOW_SIZE) yawWindow.pop_front();

				// 2. 计算平均速度 (Speed Window)
				float avgPitchSpeed = 0.0f;
				if (!pitchWindow.empty()) {
					avgPitchSpeed = std::accumulate(pitchWindow.begin(), pitchWindow.end(), 0.0f) / pitchWindow.size();
				}

				float avgYawSpeed = 0.0f;
				if (!yawWindow.empty()) {
					avgYawSpeed = std::accumulate(yawWindow.begin(), yawWindow.end(), 0.0f) / yawWindow.size();
				}

				// [关键修正]：既然用户反馈"上下飞"或者"没作用"，说明平滑算法在某些情况下与游戏实际后坐力脱节。
// 让我们尝试回退到最原始、最粗暴但也是最有效的"绝对位置锁定"方案，只保留第一枪的特殊处理。
// 这种方案不依赖"速度"，而是依赖"位移"，枪抬多少就压多少，不存在震荡问题。

// 切换回绝对位置算法：
// 需要计算相对于开火起点的总偏移量
				float totalYawOffset = controlRotation.Yaw - initialYaw;
				float totalPitchOffset = controlRotation.Pitch - initialPitch;

				// 规范化角度
				if (totalYawOffset > 180.f) totalYawOffset -= 360.f;
				else if (totalYawOffset < -180.f) totalYawOffset += 360.f;
				if (totalPitchOffset > 180.f) totalPitchOffset -= 360.f;
				else if (totalPitchOffset < -180.f) totalPitchOffset += 360.f;

				// 第一枪优化：前几帧强制增加下压力度
				// 这里我们不需要修改 Offset，而是修改 Multiplier
				float currentYMultiplier = yMultiplier;
				if (pitchWindow.size() <= 5) // 前5帧
				{
					currentYMultiplier *= 1.5f; // 1.5倍力度按住第一枪

				}

				// 计算鼠标移动量 (绝对位置方案)
// 枪口上抬 -> Pitch增加 -> Offset > 0 -> 需要向下压 -> MoveY > 0
// 公式： MoveY = Offset * Multiplier
				int moveY = static_cast<int>(totalPitchOffset * currentYMultiplier);

				// 水平方向
				int moveX = -static_cast<int>(totalYawOffset * xMultiplier);

				// [重要] 这里不需要速度阈值保护，因为我们用的是绝对位置，如果甩枪过快，会触发上面的 YAW_RESET_THRESHOLD 重置基准点


				// --- 应用补偿 ---
				if (moveX != 0 || moveY != 0) {
					Move(moveX, moveY);
				}

				Sleep(interval);

				// 更新上一帧的旋转数据
				lastRotation = controlRotation;
			}
			else
			{
				// 如果松开了左键，重置状态
				isCompensating = false;
			}
		}

		// 虽然在无限循环中通常走不到这里，但这是个好习惯
		mem.CloseScatterHandle(recolHandle);
	}
};