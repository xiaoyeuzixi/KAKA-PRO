#pragma once

#if defined(__APPLE__) || defined(__unix__)
#define SP_IMPORT 
#define SP_API
#define SP_WCHAR unsigned short
#else
#define SP_IMPORT __declspec(dllimport)
#define SP_API __stdcall
#define SP_WCHAR wchar_t
#ifdef _WIN64
#pragma comment(lib, "SProtectSDK64.lib")
#else
#pragma comment(lib, "SProtectSDK32.lib")
#endif // _WIN64
#endif // __APPLE__ || __unix__

#ifdef __cplusplus
extern "C" {
#else 
#define bool int
#endif
// 可选虚拟机 但默认为 Fast 虚拟机类型
SP_IMPORT void SP_API SPBegin(const char* szMarkName);
// 快速虚拟机
SP_IMPORT void SP_API SPBeginVirtualizeFast(const char* szMarkName);
// 变异+乱序虚拟机
SP_IMPORT void SP_API SPBeginVirtualizeUltra(const char* szMarkName);
// 代码变异
SP_IMPORT void SP_API SPBeginMutation(const char* szMarkName);

// * * * * * * * * * * * * 该水印为代码加密水印, 并自动查找范围内代码是否可以抽取成antidump水印 begin * * * * * * * * * * * * 
// 
// 可选虚拟机 但默认为 Fast 虚拟机类型
SP_IMPORT void SP_API SPBeginVMAntiDump(const char* szMarkName);
// 快速虚拟机
SP_IMPORT void SP_API SPBeginVMFastAntiDump(const char* szMarkName);
// 变异+乱序虚拟机
SP_IMPORT void SP_API SPBeginVMUltraAntiDump(const char* szMarkName);
//
// * * * * * * * * * * * * end * * * * * * * * * * * * 

// 代码结束标记
SP_IMPORT void SP_API SPEnd(void);

// 代码防dump标记
SP_IMPORT void  SP_API SPAntiDumpBegin(void);

// 代码防dump结束标记
SP_IMPORT void  SP_API SPAntiDumpEnd(void);

// 检查是否已经使用SP加壳过 true已经加壳 false未加壳
SP_IMPORT bool  SP_API SPIsProtected(void);

// 代码效验水印 快速模式
SP_IMPORT bool SP_API SPCrc(void);

// 检测调试器
SP_IMPORT __int64 SP_API SPCheckDebugger(void);

// 检测是否在虚拟机环境中运行
SP_IMPORT bool SP_API SPCheckVMacPC(void);

// 字符串加密解密
SP_IMPORT const char* SP_API SPDecryptStringA(const char* value);
SP_IMPORT const SP_WCHAR* SP_API SPDecryptStringW(const SP_WCHAR* value);
SP_IMPORT bool SP_API SPFreeString(const void* value);

SP_IMPORT void* SP_API SPGetCloudInfo(void* param);

#ifdef __cplusplus
}
#endif
