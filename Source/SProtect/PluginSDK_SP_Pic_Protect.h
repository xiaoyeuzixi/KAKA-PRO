#pragma once
#include "PluginSDK_SP_Pic_Protect_Holder.h"
#ifdef __cplusplus
extern "C" {
#else 
#define bool int
#endif
	//
	// PIC支持的最大线程数(并发) 60个
	// 同一个线程里面编号1到编号16可随意使用
	//
	__declspec(dllimport) void __stdcall SPPicBegin_1();
	__declspec(dllimport) void __stdcall __SPPicCheck_1();
	__declspec(dllimport) void __stdcall SPPicEnd_1();
	__declspec(dllimport) void __stdcall SPPicBegin_2();
	__declspec(dllimport) void __stdcall __SPPicCheck_2();
	__declspec(dllimport) void __stdcall SPPicEnd_2();
	__declspec(dllimport) void __stdcall SPPicBegin_3();
	__declspec(dllimport) void __stdcall __SPPicCheck_3();
	__declspec(dllimport) void __stdcall SPPicEnd_3();
	__declspec(dllimport) void __stdcall SPPicBegin_4();
	__declspec(dllimport) void __stdcall __SPPicCheck_4();
	__declspec(dllimport) void __stdcall SPPicEnd_4();
	__declspec(dllimport) void __stdcall SPPicBegin_5();
	__declspec(dllimport) void __stdcall __SPPicCheck_5();
	__declspec(dllimport) void __stdcall SPPicEnd_5();
	__declspec(dllimport) void __stdcall SPPicBegin_6();
	__declspec(dllimport) void __stdcall __SPPicCheck_6();
	__declspec(dllimport) void __stdcall SPPicEnd_6();
	__declspec(dllimport) void __stdcall SPPicBegin_7();
	__declspec(dllimport) void __stdcall __SPPicCheck_7();
	__declspec(dllimport) void __stdcall SPPicEnd_7();
	__declspec(dllimport) void __stdcall SPPicBegin_8();
	__declspec(dllimport) void __stdcall __SPPicCheck_8();
	__declspec(dllimport) void __stdcall SPPicEnd_8();
	__declspec(dllimport) void __stdcall SPPicBegin_9();
	__declspec(dllimport) void __stdcall __SPPicCheck_9();
	__declspec(dllimport) void __stdcall SPPicEnd_9();
	__declspec(dllimport) void __stdcall SPPicBegin_10();
	__declspec(dllimport) void __stdcall __SPPicCheck_10();
	__declspec(dllimport) void __stdcall SPPicEnd_10();
	__declspec(dllimport) void __stdcall SPPicBegin_11();
	__declspec(dllimport) void __stdcall __SPPicCheck_11();
	__declspec(dllimport) void __stdcall SPPicEnd_11();
	__declspec(dllimport) void __stdcall SPPicBegin_12();
	__declspec(dllimport) void __stdcall __SPPicCheck_12();
	__declspec(dllimport) void __stdcall SPPicEnd_12();
	__declspec(dllimport) void __stdcall SPPicBegin_13();
	__declspec(dllimport) void __stdcall __SPPicCheck_13();
	__declspec(dllimport) void __stdcall SPPicEnd_13();
	__declspec(dllimport) void __stdcall SPPicBegin_14();
	__declspec(dllimport) void __stdcall __SPPicCheck_14();
	__declspec(dllimport) void __stdcall SPPicEnd_14();
	__declspec(dllimport) void __stdcall SPPicBegin_15();
	__declspec(dllimport) void __stdcall __SPPicCheck_15();
	__declspec(dllimport) void __stdcall SPPicEnd_15();
	__declspec(dllimport) void __stdcall SPPicBegin_16();
	__declspec(dllimport) void __stdcall __SPPicCheck_16();
	__declspec(dllimport) void __stdcall SPPicEnd_16();
#ifdef __cplusplus
}
#endif


#define SPPicCheck_1(){\
	__SPPicCheck_1();\
	SPPIC_Holder()\
}

#define SPPicCheck_2(){\
	__SPPicCheck_2();\
	SPPIC_Holder()\
}

#define SPPicCheck_3(){\
	__SPPicCheck_3();\
	SPPIC_Holder()\
}

#define SPPicCheck_4(){\
	__SPPicCheck_4();\
	SPPIC_Holder()\
}

#define SPPicCheck_5(){\
	__SPPicCheck_5();\
	SPPIC_Holder()\
}

#define SPPicCheck_6(){\
	__SPPicCheck_6();\
	SPPIC_Holder()\
}

#define SPPicCheck_7(){\
	__SPPicCheck_7();\
	SPPIC_Holder()\
}

#define SPPicCheck_8(){\
	__SPPicCheck_8();\
	SPPIC_Holder()\
}

#define SPPicCheck_9(){\
	__SPPicCheck_9();\
	SPPIC_Holder()\
}

#define SPPicCheck_10(){\
	__SPPicCheck_10();\
	SPPIC_Holder()\
}

#define SPPicCheck_11(){\
	__SPPicCheck_11();\
	SPPIC_Holder()\
}

#define SPPicCheck_12(){\
	__SPPicCheck_12();\
	SPPIC_Holder()\
}

#define SPPicCheck_13(){\
	__SPPicCheck_13();\
	SPPIC_Holder()\
}

#define SPPicCheck_14(){\
	__SPPicCheck_14();\
	SPPIC_Holder()\
}

#define SPPicCheck_15(){\
	__SPPicCheck_15();\
	SPPIC_Holder()\
}

#define SPPicCheck_16(){\
	__SPPicCheck_16();\
	SPPIC_Holder()\
}
