#include "COM.h"
#include "KmBox.h"
#include <iostream>
#include <string>
#include <algorithm>
#include "common/Data.h"
#include <thread>

_com myserial;

void Write(int x, int y)
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.move(%d,%d)\r\n", x, y);
    myserial.write(buff);
}

void ReadKmBox()
{
    while (GameData.Config.AimBot.Connected && GameData.Config.AimBot.Controller == 0)
    {
        Sleep(500);
        char buff[1024];
        myserial.read(buff, 100);
        std::cout << buff << std::endl;
    }
}
void WriteWheel(int state)
{
    char buff[120];
    snprintf(buff, sizeof(buff), "km.wheel(%d)\r", state);
    myserial.write(buff);

}
void SetScreen(int w, int h)
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.screen(%d, %d)\r\n", w, h);
    myserial.write(buff);
}

void Delay(int time)
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.delay(%d)\r\n", time);
    myserial.write(buff);
}

void SetFreq(int freq)
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.freq(%d)\r\n", freq);
    myserial.write(buff);
}

void Reboot()
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.reboot()\r\n");
    myserial.write(buff);
}

bool KmBox::Init(int com)
{
    bool isOpen = myserial.open(com, 115200);
    char ctrlC = 0x03;
    char ctrlD = 0x04;
    myserial.write(&ctrlC, 1);
    Sleep(100);
    SetFreq(1000);
    Sleep(100);
    return isOpen;
}
void KmBox::simulateClick()
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.click(0)\r\n", time);
    myserial.write(buff);
}

void KmBox::PressTheLeft()
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.left(1)\r\n", time);
    myserial.write(buff);
}

void KmBox::PopUpTheLeft()
{
    char buff[1024];
    snprintf(buff, sizeof(buff), "km.left(0)\r\n", time);
    myserial.write(buff);
}
void KmBox::MortarWheel(int state)
{
    char buff[120];
    snprintf(buff, sizeof(buff), "km.wheel(%d)\r", state);
    myserial.write(buff);
}
void KmBox::Close()
{
    myserial.close();
}

void KmBox::Move(int x, int y)
{
    Write(x, y);
}

void KmBox::Clear()
{
    char ctrlC = 0x03;
    myserial.write(&ctrlC, 1);
}