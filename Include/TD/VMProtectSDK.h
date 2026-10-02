#ifndef VMPROTECTSDK_H
#define VMPROTECTSDK_H

#ifdef __cplusplus
extern "C" {
#endif

	// VMProtect 常用函数定义

	// 用于保护的区域开始
	__declspec(dllimport) void VMProtectBegin(const char* name);
	__declspec(dllimport) void VMProtectBeginVirtualization(const char* name);
	__declspec(dllimport) void VMProtectBeginMutation(const char* name);
	__declspec(dllimport) void VMProtectBeginUltra(const char* name);

	// 用于保护的区域结束
	__declspec(dllimport) void VMProtectEnd(void);

	// 检测是否被调试
	__declspec(dllimport) int VMProtectIsDebuggerPresent(void);

	// 检测是否在虚拟机中
	__declspec(dllimport) int VMProtectIsVirtualMachinePresent(void);

	// 检测程序是否经过修改
	__declspec(dllimport) int VMProtectIsProtected(void);

#ifdef __cplusplus
}
#endif

#endif // VMPROTECTSDK_H
