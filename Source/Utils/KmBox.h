#pragma once
#include <iostream>
#include <windows.h>
#include <string>

class KmBox
{
public:
    static bool Init(int com);
    static void MortarWheel(int state);
    static void Move(int x, int y);
    static void simulateClick();
    static void PressTheLeft();
    static void PopUpTheLeft();
    static void Close();
    static void Clear();
};