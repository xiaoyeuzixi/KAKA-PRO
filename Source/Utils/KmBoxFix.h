#pragma once
#include "COM.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <Windows.h>

// 修复版的KmBox类，解决COM端口连接问题
class KmBoxFix
{
public:
    // 增强版初始化函数，添加COM端口有效性检查和错误处理
    static bool Init(int com)
    {
        // 记录连接参数，用于调试
        lastCOM = com;
        
        // 确保COM端口号有效
        if (com <= 0) {
            std::cout << "错误: COM端口号无效 (" << com << ")" << std::endl;
            // 尝试使用默认COM端口
            com = 1;
        }
        
        // 尝试打开COM端口
        bool isOpen = myserial.open(com, 115200);
        
        if (!isOpen) {
            std::cout << "错误: 无法打开COM" << com << std::endl;
            return false;
        }
        
        // 发送控制字符
        char ctrlC = 0x03;
        myserial.write(&ctrlC, 1);
        Sleep(100);
        
        // 设置频率
        SetFreq(1000);
        Sleep(100);
        
        // 保存连接状态
        isConnected = true;
        return true;
    }

    // 关闭连接
    static void Close()
    {
        myserial.close();
        isConnected = false;
    }

    // 鼠标移动
    static void Move(int x, int y)
    {
        if (!isConnected) return;
        
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.move(%d,%d)\r\n", x, y);
        myserial.write(buff);
    }

    // 清理连接
    static void Clear()
    {
        if (!isConnected) return;
        
        char ctrlC = 0x03;
        myserial.write(&ctrlC, 1);
    }
    
    // 设置频率
    static void SetFreq(int freq)
    {
        if (!isConnected) return;
        
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.freq(%d)\r\n", freq);
        myserial.write(buff);
    }
    
    // 获取连接状态
    static bool IsConnected()
    {
        return isConnected;
    }
    
    // 获取上次使用的COM端口
    static int GetLastCOM()
    {
        return lastCOM;
    }

private:
    static _com myserial;
    static bool isConnected;
    static int lastCOM;
};

// 静态成员初始化
_com KmBoxFix::myserial;
bool KmBoxFix::isConnected = false;
int KmBoxFix::lastCOM = 0;
