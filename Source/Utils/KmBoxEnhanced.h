#pragma once
#include "COM.h"
#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>

// 增强版的KmBox类，添加更多错误处理和调试功能
class KmBoxEnhanced
{
public:
    // 初始化连接函数，添加更多错误处理
    static bool Init(int com)
    {
        // 确保COM端口号有效
        if (com <= 0) {
            std::cout << "错误: COM端口号无效 (" << com << ")" << std::endl;
            return false;
        }
        
        // 尝试打开COM端口
        _com serialPort;
        bool isOpen = serialPort.open(com, 115200);
        
        if (!isOpen) {
            std::cout << "错误: 无法打开COM" << com << std::endl;
            return false;
        }
        
        // 发送控制字符
        char ctrlC = 0x03;
        serialPort.write(&ctrlC, 1);
        Sleep(100);
        
        // 设置频率
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.freq(%d)\r\n", 1000);
        serialPort.write(buff);
        Sleep(100);
        
        // 保存成功打开的串口
        myserial = serialPort;
        return true;
    }

    // 关闭连接
    static void Close()
    {
        myserial.close();
    }

    // 鼠标移动
    static void Move(int x, int y)
    {
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.move(%d,%d)\r\n", x, y);
        myserial.write(buff);
    }

    // 清理连接
    static void Clear()
    {
        char ctrlC = 0x03;
        myserial.write(&ctrlC, 1);
    }
    
    // 设置屏幕尺寸
    static void SetScreen(int w, int h)
    {
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.screen(%d, %d)\r\n", w, h);
        myserial.write(buff);
    }
    
    // 设置延迟
    static void SetDelay(int time)
    {
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.delay(%d)\r\n", time);
        myserial.write(buff);
    }
    
    // 重启设备
    static void Reboot()
    {
        char buff[1024];
        snprintf(buff, sizeof(buff), "km.reboot()\r\n");
        myserial.write(buff);
    }

private:
    static _com myserial;
};

// 静态成员初始化
_com KmBoxEnhanced::myserial;
