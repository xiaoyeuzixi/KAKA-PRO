#pragma once

#include <Windows.h>
#include <cstdint>

class Decrypt
{
public:
	//解密OBJID
	static DWORD CIndex(DWORD value);
	//解密指针
	static uint64_t Xe(uint64_t addr);
	//销毁解密函数
	static void DestroyXe();
};
//#pragma once
//#include <Windows.h>
//#include <cstdint>
//
//// 这行保持原样，不动它
//static auto DecFunction = reinterpret_cast<uint64_t(*)(uint64_t key, uint64_t base)>(0);
//
//class Decrypt
//{
//private:
//    // 【新增】私有静态变量，用来缓存配置
//    // 这些变量外部访问不到，不会污染其他文件
//    static DWORD s_Key1;
//    static DWORD s_Key2;
//    static DWORD s_Key3;
//    static int   s_Rval;
//    static int   s_Sval;
//    static int   s_Dval;
//    static bool  s_IsRor;
//    static bool  s_IsInitialized; // 标记是否初始化过
//
//public:
//    // 【新增】初始化函数 (建议在游戏启动时显式调用)
//    static void InitCache();
//
//    // 保持原样，其他文件调用这个接口的方式完全不用变
//    static DWORD CIndex(DWORD value);
//
//    // 保持原样
//    static uint64_t Xe(uint64_t addr);
//
//    // 保持原样
//    static void DestroyXe();
//};
