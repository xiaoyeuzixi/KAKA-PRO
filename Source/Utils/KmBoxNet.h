#pragma once

#include <Utils/NetConfig/kmboxNet.h>
#include <Utils/NetConfig/HidTable.h>
#include <iostream>

class KmBoxNet
{
public:
    static bool Init(char* IP, char* Port, char* UUID)
    {
        //连接前 PING 一下
        std::string pingCommand = "ping ";
        pingCommand += IP;
        pingCommand += " >nul";
        system(pingCommand.c_str());

        kmNet_init((char*)IP, (char*)Port, (char*)UUID);
        return true;
    }

    static void Clear()
    {
        std::thread([]() {
            std::string pingCommand = "ping ";
            pingCommand += GameData.Config.AimBot.IP;
            pingCommand += " >nul";
            system(pingCommand.c_str());
            }).detach();
    }

    static void Close()
    {
        kmNet_reboot();
    }

    static void Move(int X, int Y)
    {
        kmNet_mouse_move(X, Y);
    }
    static void simulateClick()
    {
        kmNet_mouse_left(1);
        kmNet_mouse_left(0);
    }

    static void PressTheLeft()
    {
        kmNet_mouse_left(1);
    }

    static void PopUpTheLeft()
    {
        kmNet_mouse_left(1);
        kmNet_mouse_left(0);
    }

    static void mouse_scroll_up()
    {
        kmNet_mouse_wheel(-1);
 
    }
    static void mouse_scroll_down()
    {
        
        kmNet_mouse_wheel(1);
    }


};