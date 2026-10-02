#pragma once
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"
#include "common/Entitys.h"
#include "utils/KmBox.h"
#include "utils/Lurker.h"
#include "utils/KmBoxNet.h"
#include "utils/MoBox.h"
#include <Hack/LineTraceHook.h>
#include <Hack/LineTrace.h>
//#include <Hack/Mortar.h>
#include <set>
#define MAX_flt			(3.402823466e+38F)
#define SMALL_NUMBER		(1.e-8f)

constexpr inline auto NAME_None = FName(0, 0);
constexpr inline auto INDEX_NONE = -1;
WeaponData CurrentWeaponData;
float RemainMouseX = 0.0f;
float RemainMouseY = 0.0f;
float AutoSwitchTargetStartTime = 0;
float RecoilTimeStartTime = 0;
FVector RecoilLocation;
float LineTraceSingleRecoilTimeStartTime = 0;
FVector LineTraceSingleRecoilLocation;
uint64_t LastCurrentWeapon = 0;
float AutomaticShootingTime = 0;
float LastRandomTime = 0.0f;
float RandomXSpeedFactor = 1.0f;
float RandomYSpeedFactor = 1.0f;
float RandomInitialValueFactor = 1.0f;
// 在全局区域添加保存所选骨骼的静态变量
static std::vector<int> g_selectedRandomBones;
static float g_lastRandomBoneSwitch = 0;
static int g_currentRandomBoneIndex = 0;
static bool g_prevIsScoping = false;
bool GetLocation1(Player TargetCharacter, int FirstBoneIndex, EBoneIndex* VisibilityBoneIndex)
{

    if (TargetCharacter.Skeleton.BonesVisiablity[FirstBoneIndex])
    {
        *VisibilityBoneIndex = (EBoneIndex)FirstBoneIndex;
        return true;
    }
    else
    {
        for (auto a : SkeletonLists::Skeleton)
        {
            for (int bone : a)
            {

                if (TargetCharacter.Skeleton.BonesVisiablity[bone])
                {
                    *VisibilityBoneIndex = (EBoneIndex)bone;
                    return true;
                }
            }
        }
    }

    return false;
}
double CalculateFlightTimeSimple(double targetDistance, double initialSpeed = 15.0)
{
    if (targetDistance <= 0 || initialSpeed <= 0)
    {
        return 0.0; // 无效输入
    }
    return (targetDistance / initialSpeed) * 1000.0; // 时间 = 距离 / 速度 → 毫秒
}
double CalculateMaxHeight(double targetDistance, double initialSpeed = 15.0, double gravity = 9.81)
{
    if (targetDistance <= 0 || initialSpeed <= 0) return 0.0;
    if (targetDistance < 50.0 || targetDistance > 80.0) return 0.0;

    double maxDistance = (initialSpeed * initialSpeed) / gravity;
    if (targetDistance > maxDistance) {
        double minSpeed = sqrt(targetDistance * gravity);
        return (initialSpeed * initialSpeed - minSpeed * minSpeed) / (2 * gravity);
    }

    double sin2theta = (targetDistance * gravity) / (initialSpeed * initialSpeed);
    double theta = M_PI_2 - 0.5 * asin(sin2theta);
    return (initialSpeed * initialSpeed) * pow(sin(theta), 2) / (2 * gravity);
}
std::atomic<bool> isGrenade = false;
std::atomic<bool> isGrenadeEx = false;
float g_highTime = 0.f;
void ClacGrenade()
{
    isGrenade = true;
    Sleep(g_highTime);
    KmBoxNet::PopUpTheLeft();
    Sleep(2000);//自瞄雷切换延迟
    isGrenade = false;
    isGrenadeEx = false;
}


void TriggerMouseLeft(const AimBotConfig& Config)
{
    if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
    {
        return;
    }

    switch (GameData.Config.AimBot.Controller) {
    case 0:
        // KmBox鼠标点击实现
        KmBox::PressTheLeft();
        std::this_thread::sleep_for(std::chrono::milliseconds(Config.Delay1));
        KmBox::PopUpTheLeft();
        break;
    case 1:
        // KmBoxNet鼠标点击实现
        kmNet_mouse_left(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(Config.Delay1));
        kmNet_mouse_left(0);
        break;

    default:
        return;
    }
}


int GetRandomBodyPart(AimBotConfig& config) {
    if (!config.RandomBodyParts) return -1;  // 如果未启用随机身体部位，返回-1

    // 设置默认的身体部位选择已经在AimBotConfig中初始化
    float currentTime = GetTickCount64();

    // 第一次运行或者配置更改时，重新计算随机骨骼列表
    if (g_selectedRandomBones.empty() || (currentTime - g_lastRandomBoneSwitch > 10000)) {
        g_selectedRandomBones.clear();

        // 统计已启用的身体部位数量
        std::vector<int> enabledParts;
        for (int i = 0; i < 17; i++) {
            if (config.RandomBodyPartsList[i]) {
                enabledParts.push_back(BoneIndex[i]);
            }
        }

        // 如果没有启用任何部位，返回-1
        if (enabledParts.empty()) return -1;

        // 最多选择config.RandomBodyPartCount个骨骼，如果可用骨骼不足则全选
        int selectCount = (enabledParts.size() < config.RandomBodyPartCount) ?
            (int)enabledParts.size() : config.RandomBodyPartCount;

        // 随机选择selectCount个骨骼
        while (g_selectedRandomBones.size() < (size_t)selectCount) {
            if (enabledParts.empty()) break;

            int randomIndex = rand() % enabledParts.size();
            g_selectedRandomBones.push_back(enabledParts[randomIndex]);
            enabledParts.erase(enabledParts.begin() + randomIndex);
        }

        g_currentRandomBoneIndex = 0;
    }

    // 如果骨骼列表为空（未选择任何骨骼），返回-1
    if (g_selectedRandomBones.empty()) return -1;

    // 根据随机速度切换目标骨骼，使用真正的随机选择
    if (currentTime - g_lastRandomBoneSwitch > config.RandomSpeed) {
        // 不再使用循环递增，而是每次真正随机选择一个索引
        g_currentRandomBoneIndex = rand() % g_selectedRandomBones.size();
        g_lastRandomBoneSwitch = currentTime;
    }

    return g_selectedRandomBones[g_currentRandomBoneIndex];
}
class AimBot
{
public:




    static void setTargetScale(int miwei) {
        targetScale2 = miwei;
        adjusting = true;
    }

    // 执行滚动操作并添加日志
    static void executeScroll(int steps) {
        steps = std::clamp(steps, 0, 100);  // 限制在合理范围内

        for (int i = 0; i < steps; i++) {
            mouse_scroll_down();
            //Sleep(1);
        }

        // Utils::Log(1, "DOWN: %d steps", steps);
    }
    // 执行调整操作
    static void adjust(int targetScale) {
        if (!adjusting) return;

        //int loop = 85;
        //while ((loop-=1) > 0) {
        //    for (size_t i = 0; i < 100; i++) {
        //        mouse_scroll_up();
        //    }

        //    for (size_t i = 0; i < loop; i++) {
        //        mouse_scroll_down();
        //        Sleep(10);
        //    }

        //    Utils::Log(1, "DOWN: %d", loop);
        //    Sleep(4000);
        //}
        for (size_t i = 0; i < 85; i++) {
            mouse_scroll_up();
        }
        static std::vector<std::pair<int, int>> s_vMap = {
             {121 ,81},
             {122 ,81},
             {123 ,81},
             {124 ,81},
             {125 ,81},
             {128 ,80},
             {129 ,80},
             {130 ,80},
             {131 ,80},
             {132 ,80},
             {133 ,80},
             {134 ,80},
             {135 ,80},
             {136 ,80},
             {137 ,80},
             {138 ,80},
             {139 ,79},
             {140 ,79},
             {141 ,79},
             {142 ,79},
             {143 ,79},
             {144 ,79},
             {145 ,79},
             {146 ,79},
             {147 ,79},
             {148 ,79},
             {149 ,78},
             {150 ,78},
             {151 ,78},
             {152 ,78},
             {153 ,78},
             {154 ,78},
             {155 ,78},
             {156 ,78},
             {157 ,78},
             {158 ,78},
             {159 ,78},
             {160 ,78},
             {161 ,78},
             {162 ,78},
             {163 ,77},
             {164 ,77},
             {165 ,77},
             {166 ,77},
             {167 ,77},
             {168 ,77},
             {169 ,77},
             {170 ,77},
             {171 ,77},
             {172 ,77},
             {173 ,77},
             {174 ,77},
             {175 ,77},
             {176 ,76},
             {177 ,76},
             {178 ,76},
             {179 ,76},
             {180 ,76},
             {181 ,76},
             {182 ,76},
             {183 ,76},
             {184 ,76},
             {185 ,76},
             {186 ,76},
             {187 ,75},
             {188 ,75},
             {189, 75},
             {190 ,75},
             {191 ,74},
             {192 ,74},
             {193 ,74},
             {194 ,74},
             {195 ,74},
             {196 ,74},
             {197 ,74},
             {198 ,74},
             {199 ,74},
             {200 ,74},
             {201 ,74},
             {202 ,74},
             {203 ,74},
             {204 ,74},
             {205 ,73},
             {206 ,73},
             {207 ,73},
             {208 ,73},
             {209 ,73},
             {210 ,73},
             {211 ,73},
             {212 ,73},
             {213 ,73},
             {214 ,73},
             {215 ,73},
             {216 ,73},
             {217 ,72},
             {218 ,72},
             {219 ,72},
             {220 ,72},
             {221 ,72},
             {222 ,72},
             {223 ,72},
             {224 ,72},
             {225 ,72},
             {226 ,72},
             {227 ,72},
             {228 ,72},
             {229 ,72},
             {230 ,72},
             {231 ,71},
             {232 ,71},
             {233 ,71},
             {234 ,71},
             {235 ,71},
             {236 ,71},
             {237 ,71},
             {238 ,71},
             {239 ,71},
             {240 ,71},
             {241 ,71},
             {242 ,71},
             {243 ,71},
             {244 ,71},
             {245 ,70},
             {246 ,70},
             {247 ,70},
             {248 ,70},
             {249 ,70},
             {250 ,70},
             {251 ,70},
             {252 ,70},
             {253 ,70},
             {254 ,70},
             {255 ,70},
             {259 ,69},
             {260 ,69},
             {261 ,69},
             {262 ,69},
             {263 ,69},
             {264 ,69},
             {265 ,69},
             {266 ,69},
             {268 ,69},
             {267 ,69},
             {269 ,69},
             {270 ,68},
             {271 ,68},
             {272 ,68},
             {273 ,68},
             {274 ,68},
             {275 ,68},
             {276 ,68},
             {277 ,68},
             {278 ,68},
             {279 ,67},
             {280 ,67},
             {281 ,67},
             {282 ,67},
             {283 ,67},
             {284, 67},
             {285 ,67},
             {286 ,67},
             {287 ,67},
             {288 ,67},
             {289 ,66},
             {290 ,66},
             {291 ,66},
             {292 ,66},
             {293 ,66},
             {294 ,66},
             {295 ,66},
             {296 ,66},
             {297 ,66},
             {298 ,66},
             {299 ,65},
             {300 ,65},
             {301 ,65},
             {302 ,65},
             {303 ,65},
             {304 ,65},
             {305 ,65},
             {306 ,65},
             {307 ,65},
             {308 ,65},
             {309 ,65},
             {310 ,65},
             {311 ,64},
             {312 ,64},
             {313 ,64},
             {314 ,64},
             {315 ,64},
             {316 ,64},
             {317 ,64},
             {318 ,64},
             {319 ,64},
             {320 ,64},
             {321 ,63},
             {322 ,63},
             {323 ,63},
             {324 ,63},
             {325 ,63},
             {326 ,63},
             {327 ,63},
             {328 ,63},
             {329 ,63},
             {330 ,63},
             {331 ,63},
             {332 ,63},
             {333 ,63},
             {334 ,62},
             {335 ,62},
             {336 ,62},
             {337 ,62},
             {338 ,62},
             {339, 62},
             {340 ,62},
             {341 ,62},
             {342 ,61},
             {343 ,61},
             {344 ,61},
             {345 ,61},
             {346 ,61},
             {347 ,61},
             {348 ,61},
             {349 ,61},
             {350 ,61},
             {351 ,61},
             {352 ,61},
             {353 ,61},
             {354 ,61},
             {355 ,60},
             {356 ,60},
             {357 ,60},
             {358 ,60},
             {359 ,60},
             {360 ,60},
             {361 ,60},
             {362 ,60},
             {363 ,60},
             {364 ,60},
             {365 ,60},
             {366 ,59},
             {367 ,59},
             {368 ,59},
             {369 ,59},
             {370 ,59},
             {371 ,59},
             {372 ,59},
             {373 ,59},
             {374 ,59},
             {375 ,59},
             {376 ,58},
             {377 ,58},
             {378 ,58},
             {379 ,58},
             {380 ,58},
             {381 ,58},
             {382 ,58},
             {383 ,58},
             {384 ,58},
             {385 ,57},
             {386 ,57},
             {387 ,57},
             {389 ,57},
             {390 ,57},
             {391 ,57},
             {391 ,56},
             {392 ,56},
             {393 ,56},
             {394 ,56},
             {395 ,56},
             {396 ,56},
             {397 ,55},
             {398 ,55},
             {399 ,55},
             {400 ,55},
             {401 ,55},
             {402 ,55},
             {403 ,55},
             {404 ,55},
             {405 ,55},
             {406 ,55},
             {407 ,55},
             {408 ,55},
             {409 ,55},
             {410 ,55},
             {411 ,55},
             {412 ,55},
             {413 ,55},
             {414 ,54},
             {415 ,54},
             {416 ,54},
             {417 ,54},
             {418 ,54},
             {419 ,54},
             {420 ,54},
             {421 ,54},
             {422 ,54},
             {423 ,54},
             {424 ,54},
             {425 ,53},
             {426 ,53},
             {427 ,53},
             {428 ,53},
             {429 ,53},
             {430 ,53},
             {431 ,53},
             {432 ,52},
             {433 ,52},
             {434 ,52},
             {435 ,52},
             {436 ,52},
             {437 ,52},
             {438 ,52},
             {439 ,52},
             {440 ,52},
             {441 ,52},
             {442 ,52},
             {443 ,52},
             {444 ,52},
             {445 ,52},
             {446 ,51},
             {447 ,51},
             {448 ,51},
             {449 ,51},
             {450 ,51},
             {451 ,51},
             {452 ,51},
             {453 ,51},
             {454 ,51},
             {455 ,51},
             {456 ,50},
             {457 ,50},
             {458 ,50},
             {459 ,50},
             {460 ,50},
             {461 ,50},
             {462 ,50},
             {463 ,50},
             {464 ,50},
             {465 ,49},
             {466 ,49},
             {467 ,49},
             {468 ,48},
             {469 ,48},
             {470 ,48},
             {471 ,48},
             {472 ,48},
             {473 ,47},
             {474 ,47},
             {475 ,47},
             {476 ,47},
             {477 ,47},
             {478 ,47},
             {479 ,47},
             {480 ,47},
             {481 ,47},
             {482 ,47},
             {483 ,47},
             {484 ,47},
             {485 ,47},
             {486 ,47},
             {487 ,47},
             {488 ,47},
             {489 ,46},
             {490 ,46},
             {491 ,46},
             {492 ,46},
             {493 ,46},
             {494 ,46},
             {495 ,46},
             {496 ,46},
             {497 ,46},
             {498 ,46},
             {499 ,46},
             {500 ,45},
             {501 ,45},
             {502 ,45},
             {503 ,45},
             {504 ,45},
             {505 ,45},
             {506 ,45},
             {507 ,45},
             {508 ,45},
             {509 ,42},
             {510 ,42},
             {511 ,42},
             {512 ,42},
             {513 ,42},
             {514 ,42},
             {515 ,42},
             {516 ,42},
             {517 ,42},
             {518 ,43},
             {519 ,43},
             {520 ,43},
             {521 ,43},
             {522 ,43},
             {523 ,43},
             {524 ,43},
             {525 ,42},
             {526 ,42},
             {527 ,42},
             {528 ,42},
             {529 ,42},
             {530 ,42},
             {531 ,42},
             {532 ,42},
             {533 ,41},
             {534 ,41},
             {535 ,41},
             {536 ,41},
             {537 ,41},
             {538 ,41},
             {539 ,41},
             {540 ,41},
             {541 ,41},
             {542 ,41},
             {543 ,41},
             {544 ,40},
             {545 ,40},
             {546 ,40},
             {547 ,40},
             {548 ,40},
             {549 ,40},
             {550 ,39},
             {551 ,39},
             {552 ,39},
             {553 ,39},
             {554 ,39},
             {555 ,38},
             {556 ,38},
             {557 ,38},
             {558 ,38},
             {559 ,38},
             {560 ,38},
             {561 ,38},
             {562 ,37},
             {563 ,37},
             {564 ,37},
             {565 ,37},
             {566 ,37},
             {567, 37},
             {568 ,37},
             {569 ,37},
             {570 ,37},
             {571 ,36},
             {572 ,36},
             {573 ,36},
             {574 ,36},
             {575 ,36},
             {576 ,36},
             {578 ,35},
             {579 ,35},
             {580 ,35},
             {580 ,34},
             {581 ,34},
             {582 ,34},
             {583 ,34},
             {584 ,33},
             {585 ,33},
             {586 ,33},
             {587 ,33},
             {588 ,33},
             {589 ,33},
             {590 ,33},
             {591 ,33},
             {592 ,33},
             {593 ,33},
             {594 ,33},
             {595 ,33},
             {596 ,32},
             {597 ,32},
             {598 ,32},
             {599 ,32},
             {600 ,32},
             {601 ,32},
             {602 ,32},
             {603 ,32},
             {604 ,32},
             {605 ,31},
             {606 ,31},
             {607 ,31},
             {608 ,31},
             {609 ,31},
             {610 ,30},
             {611 ,30},
             {612 ,30},
             {613 ,30},
             {614 ,30},
             {615 ,30},
             {616 ,29},
             {617 ,29},
             {618 ,29},
             {619 ,29},
             {620 ,29},
             {621 ,29},
             {622 ,28},
             {623 ,28},
             {624 ,28},
             {625 ,28},
             {626 ,28},
             {627 ,27},
             {628 ,27},
             {629 ,27},
             {630 ,27},
             {631 ,27},
             {632 ,26},
             {633 ,26},
             {634 ,26},
             {635 ,26},
             {636 ,26},
             {637 ,25},
             {638 ,25},
             {639 ,25},
             {640 ,25},
             {641 ,25},
             {642 ,24},
             {643 ,24},
             {644 ,24},
             {645 ,24},
             {646 ,24},
             {647 ,24},
             {648 ,24},
             {649 ,23},
             {650 ,23},
             {651 ,22},
             {652 ,22},
             {653 ,22},
             {654 ,22},
             {655 ,22},
             {656 ,22},
             {657 ,21},
             {658 ,21},
             {659 ,21},
             {660 ,20},
             {661 ,20},
             {662 ,20},
             {663 ,20},
             {664 ,19},
             {665 ,19},
             {666 ,19},
             {667 ,19},
             {668 ,18},
             {669 ,18},
             {670 ,18},
             {671 ,18},
             {672 ,17},
             {673 ,17},
             {674 ,17},
             {675 ,16},
             {676 ,16},
             {677 ,16},
             {678 ,15},
             {679 ,15},
             {680 ,15},
             {681 ,14},
             {682 ,14},
             {683 ,14},
             {684 ,13},
             {685 ,13},
             {686 ,12},
             {687 ,12},
             {688 ,12},
             {689 ,11},
             {690 ,10},
             {691 ,10},
             {692 ,9},
             {693 ,9},
             {694 ,8},
             {695 ,8},
             {696 ,7},
             {697 ,6},
             {698 ,5},
             {699 ,4},
             {699 ,3},
             {699 ,2},
             {700 ,1},
        };
        /*static std::vector<std::pair<int, int>> s_vMap = {
                      { 121  ,     81} ,
                      { 133  ,     80} ,
                      { 145  ,     79} ,
                      { 157  ,     78} ,
                      { 169  ,     77} ,
                      { 181  ,     76} ,
                      { 193  ,     75} ,
                      { 193  ,     74} ,
                      { 216  ,     73} ,
                      { 216  ,     72} ,
                      { 239  ,     71} ,
                      { 250  ,     70} ,
                      { 262  ,     69} ,
                      { 273  ,     68} ,
                      { 284  ,     67} ,
                      { 295  ,     66} ,
                      { 307  ,     65} ,
                      { 317  ,     64} ,
                      { 328  ,     63} ,
                      { 339  ,     62} ,
                      { 350  ,     61} ,
                      { 360  ,     60} ,
                      { 371  ,     59} ,
                      { 381  ,     58} ,
                      { 391  ,     57} ,
                      { 391  ,     56} ,
                      { 411  ,     55} ,
                      { 421  ,     54} ,
                      { 431  ,     53} ,
                      { 440  ,     52} ,
                      { 450  ,     51} ,
                      { 459  ,     50} ,
                      { 468  ,     49} ,
                      { 468  ,     48} ,
                      { 476  ,     47} ,
                      { 495  ,     46} ,
                      { 503  ,     45} ,
                      { 512  ,     42} ,
                      { 520  ,     43} ,
                      { 528  ,     42} ,
                      { 536  ,     41} ,
                      { 544  ,     40} ,
                      { 551  ,     39} ,
                      { 559  ,     38} ,
                      { 566  ,     37} ,
                      { 573  ,     36} ,
                      { 580  ,     35} ,
                      { 580  ,     34} ,
                      { 587  ,     33} ,
                      { 600  ,     32} ,
                      { 606  ,     31} ,
                      { 612  ,     30} ,
                      { 618  ,     29} ,
                      { 624  ,     28} ,
                      { 629  ,     27} ,
                      { 634  ,     26} ,
                      { 639  ,     25} ,
                      { 644  ,     24} ,
                      { 649  ,     23} ,
                      { 653  ,     22} ,
                      { 658  ,     21} ,
                      { 662  ,     20} ,
                      { 666  ,     19} ,
                      { 669  ,     18} ,
                      { 673  ,     17} ,
                      { 676  ,     16} ,
                      { 679  ,     15} ,
                      { 682  ,     14} ,
                      { 685  ,     13} ,
                      { 687  ,     12} ,
                      { 689  ,     11} ,
                      { 691  ,     10} ,
                      {  693 ,    9  } ,
                      {  695 ,    8  } ,
                      {  696 ,    7  } ,
                      {  697 ,    6  } ,
                      {  698 ,    5  } ,
                      {  699 ,    4  } ,
                      {  699 ,    3  } ,
                      {  699 ,    2  } ,
                      {  700 ,    1  } ,
        };*/
        if (s_vMap.empty()) return;

        // 修改边界检查：当目标值小于121时直接返回
        if (targetScale < s_vMap.front().first) {  // 121 是第一个元素
            return;  // 不执行任何滚动操作
        }
        if (targetScale >= s_vMap.back().first) {
            executeScroll(s_vMap.back().second);
            return;
        }

        // 二分查找 + 邻近检查（保持原逻辑）
        size_t low = 0;
        size_t high = s_vMap.size() - 1;
        size_t index = 0;

        while (low <= high) {
            index = low + (high - low) / 2;
            const auto& current = s_vMap[index];

            // 检查当前点是否在目标值±5范围内
            if (std::abs(current.first - targetScale) <= 8) {
                executeScroll(current.second);
                return;
            }

            if (current.first == targetScale) {
                executeScroll(current.second);
                return;
            }

            if (targetScale < current.first) {
                if (index > 0 && targetScale > s_vMap[index - 1].first) {
                    break; // 找到目标区间
                }
                high = index - 1;
            }
            else {
                low = index + 1;
            }
        }

        // 线性插值（原逻辑不变）
        const auto& lower = s_vMap[index - 1];
        const auto& upper = s_vMap[index];
        double ratio = static_cast<double>(targetScale - lower.first) / (upper.first - lower.first);
        int cnt = static_cast<int>(lower.second + ratio * (upper.second - lower.second) + 0.5);
        executeScroll(cnt);
    }

    static void StopAiming(bool UseSleep = true)
    {

        g_isMortars = false;
        //if (UseSleep) Sleep(1);
        GameData.AimBot.Lock = false;
        GameData.AimBot.Target = 0;
        RemainMouseX = 0.f;
        RemainMouseY = 0.f;
        AutoSwitchTargetStartTime = 0;
        RecoilTimeStartTime = 0;
        RecoilLocation = { 0.f, 0.f, 0.f };
        LineTraceSingleRecoilTimeStartTime = 0;
        LineTraceSingleRecoilLocation = { 0.f, 0.f, 0.f };
        LastCurrentWeapon = 0;
        Data::SetEnemyInfoMap({});
    }

    static void CycleTime(float MinTime, float MaxTime, float& InTime, int& CycleCount)
    {
        float InitTime = InTime;
        float Duration = MaxTime - MinTime;

        if (InTime > MaxTime)
        {
            CycleCount = FloorToInt((MaxTime - InTime) / Duration);
            InTime = InTime + Duration * CycleCount;
        }
        else if (InTime < MinTime)
        {
            CycleCount = FloorToInt((InTime - MinTime) / Duration);
            InTime = InTime - Duration * CycleCount;
        }

        if (InTime == MaxTime && InitTime < MinTime)
        {
            InTime = MinTime;
        }

        if (InTime == MinTime && InitTime > MaxTime)
        {
            InTime = MaxTime;
        }

        CycleCount = Abs(CycleCount);
    }

    static void RemapTimeValue(float& InTime, float& CycleValueOffset, FRichCurve RichCurve, int KeysNum, std::vector<FRichCurveKey> Keys)
    {
        const int32 NumKeys = KeysNum;

        if (NumKeys < 2)
        {
            return;
        }

        if (InTime <= Keys[0].Time)
        {
            if (RichCurve.PreInfinityExtrap != RCCE_Linear && RichCurve.PreInfinityExtrap != RCCE_Constant)
            {
                float MinTime = Keys[0].Time;
                float MaxTime = Keys[NumKeys - 1].Time;

                int CycleCount = 0;
                CycleTime(MinTime, MaxTime, InTime, CycleCount);

                if (RichCurve.PreInfinityExtrap == RCCE_CycleWithOffset)
                {
                    float DV = Keys[0].Value - Keys[NumKeys - 1].Value;
                    CycleValueOffset = DV * CycleCount;
                }
                else if (RichCurve.PreInfinityExtrap == RCCE_Oscillate)
                {
                    if (CycleCount % 2 == 1)
                    {
                        InTime = MinTime + (MaxTime - InTime);
                    }
                }
            }
        }
        else if (InTime >= Keys[NumKeys - 1].Time)
        {
            if (RichCurve.PostInfinityExtrap != RCCE_Linear && RichCurve.PostInfinityExtrap != RCCE_Constant)
            {
                float MinTime = Keys[0].Time;
                float MaxTime = Keys[NumKeys - 1].Time;

                int CycleCount = 0;
                CycleTime(MinTime, MaxTime, InTime, CycleCount);

                if (RichCurve.PostInfinityExtrap == RCCE_CycleWithOffset)
                {
                    float DV = Keys[NumKeys - 1].Value - Keys[0].Value;
                    CycleValueOffset = DV * CycleCount;
                }
                else if (RichCurve.PostInfinityExtrap == RCCE_Oscillate)
                {
                    if (CycleCount % 2 == 1)
                    {
                        InTime = MinTime + (MaxTime - InTime);
                    }
                }
            }
        }
    }
    static void AimBotAPI_SG(FVector2D MoveXY, AimBotConfig Config)
    {
        if (MoveXY.X == 0 && MoveXY.Y == 0) {
            return;
        }

        float MouseX = MoveXY.X * Config.XSpeed / 100.0f;
        float MouseY = MoveXY.Y * Config.YSpeed / 100.0f;

        if (abs(MouseX) > 0 || abs(MouseY) > 0) {
            Move(MouseX, MouseY);
        }

        //const float Threshold = 3.f;//靠近人物的范围，越小需要越靠近目标才会开枪，提高准确度，但是会降低容错，越低，开枪概率越小
        if (abs(MoveXY.X) < Config.Threshold && abs(MoveXY.Y) < Config.Threshold) {


            TriggerMouseLeft(Config);

        }
    }
    static float Eval(float InTime, float InDefaultValue, FRichCurve RichCurve, int KeysNum, std::vector<FRichCurveKey> Keys)
    {
        // Remap time if extrapolation is present and compute offset value to use if cycling 
        float CycleValueOffset = 0;
        RemapTimeValue(InTime, CycleValueOffset, RichCurve, KeysNum, Keys);

        const int32 NumKeys = KeysNum;

        // If the default value hasn't been initialized, use the incoming default value
        float InterpVal = RichCurve.DefaultValue == MAX_flt ? InDefaultValue : RichCurve.DefaultValue;

        if (NumKeys == 0)
        {
            // If no keys in curve, return the Default value.
        }
        else if (NumKeys < 2 || (InTime <= Keys[0].Time))
        {
            if (RichCurve.PreInfinityExtrap == RCCE_Linear && NumKeys > 1)
            {
                float DT = Keys[1].Time - Keys[0].Time;

                if (IsNearlyZero(DT))
                {
                    InterpVal = Keys[0].Value;
                }
                else
                {
                    float DV = Keys[1].Value - Keys[0].Value;
                    float Slope = DV / DT;

                    InterpVal = Slope * (InTime - Keys[0].Time) + Keys[0].Value;
                }
            }
            else
            {
                // Otherwise if constant or in a cycle or oscillate, always use the first key value
                InterpVal = Keys[0].Value;
            }
        }
        else if (InTime < Keys[NumKeys - 1].Time)
        {
            // perform a lower bound to get the second of the interpolation nodes
            int32 first = 1;
            int32 last = NumKeys - 1;
            int32 count = last - first;

            while (count > 0)
            {
                int32 step = count / 2;
                int32 middle = first + step;

                if (InTime >= Keys[middle].Time)
                {
                    first = middle + 1;
                    count -= step + 1;
                }
                else
                {
                    count = step;
                }
            }

            int32 InterpNode = first;
            const float Diff = Keys[InterpNode].Time - Keys[InterpNode - 1].Time;

            if (Diff > 0.f && Keys[InterpNode - 1].InterpMode != RCIM_Constant)
            {
                const float Alpha = (InTime - Keys[InterpNode - 1].Time) / Diff;
                const float P0 = Keys[InterpNode - 1].Value;
                const float P3 = Keys[InterpNode].Value;

                if (Keys[InterpNode - 1].InterpMode == RCIM_Linear)
                {
                    InterpVal = Lerp(P0, P3, Alpha);
                }
                else
                {
                    const float OneThird = 1.0f / 3.0f;
                    const float P1 = P0 + (Keys[InterpNode - 1].LeaveTangent * Diff * OneThird);
                    const float P2 = P3 - (Keys[InterpNode].ArriveTangent * Diff * OneThird);

                    InterpVal = BezierInterp(P0, P1, P2, P3, Alpha);
                }
            }
            else
            {
                InterpVal = Keys[InterpNode - 1].Value;
            }
        }
        else
        {
            if (RichCurve.PostInfinityExtrap == RCCE_Linear)
            {
                float DT = Keys[NumKeys - 2].Time - Keys[NumKeys - 1].Time;

                if (IsNearlyZero(DT))
                {
                    InterpVal = Keys[NumKeys - 1].Value;
                }
                else
                {
                    float DV = Keys[NumKeys - 2].Value - Keys[NumKeys - 1].Value;
                    float Slope = DV / DT;

                    InterpVal = Slope * (InTime - Keys[NumKeys - 1].Time) + Keys[NumKeys - 1].Value;
                }
            }
            else
            {
                // Otherwise if constant or in a cycle or oscillate, always use the last key value
                InterpVal = Keys[NumKeys - 1].Value;
            }
        }
        return InterpVal + CycleValueOffset;
    }

    static void SimulateWeaponTrajectory(FVector Direction, float Distance, float TrajectoryGravityZ,
        float BallisticDragScale, float BallisticDropScale,
        float BDS, float SimulationSubstepTime, float VDragCoefficient,
        FRichCurve RichCurve, int KeysNum, std::vector<FRichCurveKey> Keys,
        float& BulletDrop, float& TravelTime)
    {
        float TravelDistance = 0.0f;
        float CurrentDrop = 0.0f;
        BulletDrop = 0.0f;
        TravelTime = 0.0f;

        Direction.Normalize();
        Direction = Direction * 100.0f;

        while (1)
        {
            float BulletSpeed = Eval(TravelDistance * BDS * BallisticDragScale, 0.0, RichCurve, KeysNum, Keys);

            FVector Velocity = Direction * BulletSpeed;
            Velocity.Z += CurrentDrop;

            FVector Acceleration = Velocity * SimulationSubstepTime;
            float AccelerationLen = Acceleration.Length() / 100.0f;
            if (TravelDistance + AccelerationLen > Distance)
            {
                float RemainDistance = Distance - TravelDistance;
                float AccelerationSpeed = AccelerationLen / SimulationSubstepTime;
                float RemainTime = RemainDistance / AccelerationSpeed;

                TravelTime += RemainTime;
                BulletDrop += RemainTime * CurrentDrop;
                break;
            }
            TravelDistance += AccelerationLen;
            TravelTime += SimulationSubstepTime;
            BulletDrop += SimulationSubstepTime * CurrentDrop;
            CurrentDrop += SimulationSubstepTime * TrajectoryGravityZ * 100 * VDragCoefficient * BallisticDropScale;
        }
    }

    static float GetDragForce(float Distance) {
        std::string WeaponEntityName = GameData.LocalPlayerInfo.WeaponEntityInfo.Name;
        std::set<std::string> GunNamesOne = {
            "WeapAK47_C", "WeapLunchmeats_AK47_C", "WeapGroza_C", "WeapBerylM762_C",
            "WeapMini14_C", "WeapQBU88_C", "Weapon_G36C_C", "WeapKar98k_C",
            "WeapMosinNagant_C", "WeapJulies_Kar98k_C", "WeapM24_C", "WeapAWM_C",
            "WeapHK416_C", "WeapDuncans_M416_C", "WeapK2_C", "WeapSCAR-L_C",
            "WeapM16A4_C", "WeapQBZ95_C", "WeapAUG_C", "Weapon_Mosin_C",
            "Weapon_M249_C", "WeaponMk14_C", "Weapon_L6_C", "WeapMG3_C",
            "WeapMads_QBU88_C","WeapFamasG2_C"
        };

        std::set<std::string> GunNamesTwo = {
            "WeapSKS_C",
            "WeapFNFal_C",
            "Weapon_Mk47Mutant_C",
            "WeapMk14_C",
            "WeapMk12_C",
            "WeapDragunov_C"
        };

        float FallOffDecay = 0.0f;
        float DistanceDecay = 0.0f;
        //if (WeaponEntityName == "WeapKar98k_C")
        if (GunNamesOne.find(WeaponEntityName) != GunNamesOne.end()) {
            if (Distance > 10 && Distance <= 50) {
                FallOffDecay = 11.f;
                DistanceDecay = 0.41f;
            }
            if (Distance > 50 && Distance <= 100) {
                FallOffDecay = 12.f;
                DistanceDecay = 0.42f;
            }
            if (Distance > 100 && Distance <= 150) {
                FallOffDecay = 13.f;
                DistanceDecay = 0.43f;
            }
            if (Distance > 150 && Distance <= 200) {
                FallOffDecay = 14.f;
                DistanceDecay = 0.44f;
            }
            if (Distance > 200 && Distance <= 250) {
                FallOffDecay = 15.f;
                DistanceDecay = 0.45f;
            }
            if (Distance > 250 && Distance <= 300) {
                FallOffDecay = 20.f;
                DistanceDecay = 0.6f;
            }
            if (Distance > 300 && Distance <= 350) {
                FallOffDecay = 22.f;
                DistanceDecay = 0.8f;
            }
            if (Distance > 350 && Distance <= 400) {
                FallOffDecay = 25.f;
                DistanceDecay = 0.95f;
            }
            if (Distance > 400 && Distance <= 450) {
                FallOffDecay = 30.f;
                DistanceDecay = 1.1f;
            }
            if (Distance > 450 && Distance <= 500) {
                FallOffDecay = 35.f;
                DistanceDecay = 1.3f;
            }
            if (Distance > 500 && Distance <= 550) {
                FallOffDecay = 40.f;
                DistanceDecay = 1.35f;
            }
            if (Distance > 550 && Distance <= 600) {
                FallOffDecay = 45.f;
                DistanceDecay = 1.45f;
            }
            if (Distance > 600 && Distance <= 650) {
                FallOffDecay = 50.f;
                DistanceDecay = 1.5f;
            }
            if (Distance > 650 && Distance <= 700) {
                FallOffDecay = 55.f;
                DistanceDecay = 1.61f;
            }
            if (Distance > 700 && Distance <= 800) {
                FallOffDecay = 60.f;
                DistanceDecay = 1.635f;
            }
            if (Distance > 800 && Distance <= 900) {
                FallOffDecay = 65.f;
                DistanceDecay = 1.88f;
            }
            if (Distance > 900 && Distance <= 1000) {
                FallOffDecay = 70.f;
                DistanceDecay = 2.1f;
            }
            if (Distance > 1000 && Distance <= 1050) {
                FallOffDecay = 75.f;
                DistanceDecay = 2.2f;
            }
        }
        else if (GunNamesTwo.find(WeaponEntityName) != GunNamesTwo.end())
        {
            if (Distance > 10 && Distance <= 50) {
                FallOffDecay = 11.f;
                DistanceDecay = 0.41f;
            }
            if (Distance > 50 && Distance <= 100) {
                FallOffDecay = 12.f;
                DistanceDecay = 0.42f;
            }
            if (Distance > 100 && Distance <= 150) {
                FallOffDecay = 13.f;
                DistanceDecay = 0.43f;
            }
            if (Distance > 150 && Distance <= 200) {
                FallOffDecay = 14.f;
                DistanceDecay = 0.44f;
            }
            if (Distance > 200 && Distance <= 250) {
                FallOffDecay = 15.f;
                DistanceDecay = 0.45f;
            }
            if (Distance > 250 && Distance <= 300) {
                FallOffDecay = 20.f;
                DistanceDecay = 0.6f;
            }
            if (Distance > 300 && Distance <= 350) {
                FallOffDecay = 22.f;
                DistanceDecay = 0.8f;
            }
            if (Distance > 350 && Distance <= 400) {
                FallOffDecay = 25.f;
                DistanceDecay = 1.1f;
            }
            if (Distance > 400 && Distance <= 450) {
                FallOffDecay = 30.f;
                DistanceDecay = 1.5f;
            }
            if (Distance > 450 && Distance <= 500) {
                FallOffDecay = 35.f;
                DistanceDecay = 1.95f;
            }
            if (Distance > 500 && Distance <= 550) {
                FallOffDecay = 40.f;
                DistanceDecay = 2.f;
            }
            if (Distance > 550 && Distance <= 600) {
                FallOffDecay = 45.f;
                DistanceDecay = 2.1f;
            }
            if (Distance > 600 && Distance <= 650) {
                FallOffDecay = 50.f;
                DistanceDecay = 2.7f;
            }
            if (Distance > 650 && Distance <= 700) {
                FallOffDecay = 55.f;
                DistanceDecay = 2.75f;
            }
            if (Distance > 700 && Distance <= 800) {
                FallOffDecay = 60.f;
                DistanceDecay = 3.5f;
            }
            if (Distance > 800 && Distance <= 900) {
                FallOffDecay = 75.f;
                DistanceDecay = 3.7f;
            }
            if (Distance > 900 && Distance <= 1000) {
                FallOffDecay = 85.f;
                DistanceDecay = 3.75f;
            }
            if (Distance > 1000 && Distance <= 1050) {
                FallOffDecay = 95.f;
                DistanceDecay = 3.85f;
            }
        }
        else {
            if (Distance <= 100) {
                DistanceDecay = 1.11f;
            }
            else if (Distance <= 150) {
                DistanceDecay = 1.165f;
            }
            else if (Distance <= 200) {
                DistanceDecay = 1.22f;
            }
            else if (Distance <= 250) {
                DistanceDecay = 1.275f;
            }
            else if (Distance <= 300) {
                DistanceDecay = 1.33f;
            }
            else if (Distance <= 350) {
                DistanceDecay = 1.385f;
            }
            else if (Distance <= 400) {
                DistanceDecay = 1.44f;
            }
            else if (Distance <= 450) {
                DistanceDecay = 1.495f;
            }
            else if (Distance <= 500) {
                DistanceDecay = 1.55f;
            }
            else if (Distance <= 400) {
                DistanceDecay = 1.7f;
            }
        }

        //Utils::Log(1, "%s %f", WeaponEntityName, DistanceDecay);

        return DistanceDecay;
    }

    static float  GetPredicted(const FVector& GunLocation, FVector TargetPos, FVector TargetVelocity, FWeaponTrajectoryConfig TrajectoryConfig) {
        FVector Results = { 0.f, 0.f, 0.f };
        const float DistanceToTarget = GunLocation.Distance(TargetPos) / 100.0f;
        float TimeToReach = DistanceToTarget / TrajectoryConfig.InitialSpeed;
        float Gravity = 9.800000191f;
        //抬枪
        float Drop = 0.5f * Gravity * TimeToReach * TimeToReach * 50.0f;
        //Utils::Log(1, "Drop: %f", Drop);

        float Force = GetDragForce(DistanceToTarget);
        Drop = Drop * Force;
        if (DistanceToTarget <= 120)
        {
            Drop = 0;
        }

        return Drop;
    }


    static void PressTheLeft()
    {
        if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
        {
            return;
        }
        switch (GameData.Config.AimBot.Controller) {
        case 0:
            KmBox::PressTheLeft();
            break;
        case 1:
            KmBoxNet::PressTheLeft();
            break;
        case 2:
            Lurker::PressTheLeft();
            break;
        case 3:
            MoBox::PressTheLeft();
            break;
        default:
            return;
        }
    }

    static void PopUpTheLeft()
    {
        if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
        {
            return;
        }
        switch (GameData.Config.AimBot.Controller) {
        case 0:
            KmBox::PopUpTheLeft();
            break;
        case 1:
            KmBoxNet::PopUpTheLeft();
            break;
        case 2:
            Lurker::PopUpTheLeft();
            break;
        case 3:
            MoBox::PopUpTheLeft();
            break;
        default:
            return;
        }
    }


    static void mouse_scroll_up()
    {
        if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
        {
            return;
        }
        switch (GameData.Config.AimBot.Controller) {
        case 0:
            KmBox::PopUpTheLeft();
            break;
        case 1:
            KmBoxNet::mouse_scroll_up();
            break;
        case 2:
            Lurker::PopUpTheLeft();
            break;
        case 3:
            MoBox::PopUpTheLeft();
            break;
        default:
            return;
        }
    }

    static void mouse_scroll_down()
    {
        if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
        {
            return;
        }
        switch (GameData.Config.AimBot.Controller) {
        case 0:
            KmBox::PopUpTheLeft();


            break;
        case 1:
            KmBoxNet::mouse_scroll_down();
            break;
        case 2:
            Lurker::PopUpTheLeft();
            break;
        case 3:
            MoBox::PopUpTheLeft();
            break;
        default:
            return;
        }
    }
    static void simulateClick()
    {
        if (!GameData.Config.AimBot.Connected || !GameData.Config.AimBot.Enable)
        {
            return;
        }
        switch (GameData.Config.AimBot.Controller) {
        case 0:
            KmBox::simulateClick();
            break;
        case 1:
            KmBoxNet::simulateClick();
            break;
        case 2:
            Lurker::simulateClick();
            break;
        case 3:
            MoBox::simulateClick();
            break;
        default:
            return;
        }
    }

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

    static void AimBotAPI(FVector2D MoveXY, AimBotConfig Config)
    {
        FVector FMouseXY = { (float)MoveXY.X, (float)MoveXY.Y, 0.0f };
        FMouseXY.Normalize();

        if (MoveXY.X == 0 && MoveXY.Y == 0) {
            RemainMouseX = RemainMouseY = 0.0f;
            return;
        }

        float InitialValue = Config.InitialValue;
        // 随机自瞄逻辑
        if (Config.RandomAim) {
            float CurrentTime = GetTickCount() / 1000.0f;
            if (CurrentTime - LastRandomTime > (Config.RandomInterval / 1000.0f)) {
                // 生成新的随机因子
                RandomXSpeedFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * Config.RandomFactor;
                RandomYSpeedFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * Config.RandomFactor;
                RandomInitialValueFactor = 1.0f + (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * Config.RandomFactor;
                LastRandomTime = CurrentTime;
            }

            // 应用随机因子
            Config.XSpeed *= RandomXSpeedFactor;
            Config.YSpeed *= RandomYSpeedFactor;
            InitialValue *= RandomInitialValueFactor;
        }
        if (GameData.AimBot.Type == EntityType::Wheel)
        {
            Config.XSpeed = Config.AimWheelSpeed;
            Config.YSpeed = Config.AimWheelSpeed;
        }

        float MouseX = RemainMouseX + std::clamp((InitialValue * (Config.XSpeed / 100.0f)) * FMouseXY.X, -(float)abs(MoveXY.X), (float)abs(MoveXY.X));
        float MouseY = RemainMouseY + std::clamp((InitialValue * (Config.YSpeed / 100.0f)) * FMouseXY.Y, -(float)abs(MoveXY.Y), (float)abs(MoveXY.Y));
        //MouseX = round(MouseX);
        //MouseY = round(MouseY);
        RemainMouseX = MouseX - truncf(MouseX);
        RemainMouseY = MouseY - truncf(MouseY);
        //Utils::Log(1, "Move: %f %f", MouseX, MouseY);
        if (abs(MouseX) > 0 || abs(MouseY) > 0) {
            //  Utils::Log(1, "Move: %f %f", MouseX, MouseY);
            Move(MouseX, MouseY);
        }
    }

    static bool GetBoneIsAllFalse(const bool Bones[17])
    {
        for (size_t i = 0; i < 17; i++)
        {
            if (Bones[i]) {
                return true;
            }
        }
        return false;
    }

    static void GetScopingAttachPointRelativeZ(
        const VMMDLL_SCATTER_HANDLE& hScatter,
        const FTransform WeaponComponentToWorld,
        float& ScopingAttachPointRelativeZ,
        FTransform& SocketWorldTransform,
        FTransform& ScopeMeshComponentToWorld
    )
    {
        FRotator ScopeSocketRelativeRotation;
        FVector ScopeSocketRelativeLocation;
        FVector ScopeSocketRelativeScale;

        if (CurrentWeaponData.ScopeSocket)
        {
            mem.AddScatterReadRequest(hScatter, CurrentWeaponData.ScopeAimCameraSocket + GameData.Offset["StaticRelativeRotation"], (FRotator*)&ScopeSocketRelativeRotation);
            mem.AddScatterReadRequest(hScatter, CurrentWeaponData.ScopeAimCameraSocket + GameData.Offset["StaticRelativeLocation"], (FVector*)&ScopeSocketRelativeLocation);
            mem.AddScatterReadRequest(hScatter, CurrentWeaponData.ScopeAimCameraSocket + GameData.Offset["StaticRelativeScale"], (FVector*)&ScopeSocketRelativeScale);
            mem.AddScatterReadRequest(hScatter, CurrentWeaponData.ScopeStaticMeshComponent + GameData.Offset["ComponentToWorld"], (FTransform*)&ScopeMeshComponentToWorld);
            mem.ExecuteReadScatter(hScatter);
        }

        if (CurrentWeaponData.ScopeSocket)
        {
            SocketWorldTransform = FTransform(ScopeSocketRelativeRotation, ScopeSocketRelativeLocation, ScopeSocketRelativeScale) * WeaponComponentToWorld;
            const float RelativeZ_1 = SocketWorldTransform.GetRelativeTransform(ScopeMeshComponentToWorld).Translation.Z;
            const float RelativeZ_2 = ScopeMeshComponentToWorld.GetRelativeTransform(WeaponComponentToWorld).Translation.Z;
            ScopingAttachPointRelativeZ = RelativeZ_1 + RelativeZ_2;
        }
        else {
            ScopingAttachPointRelativeZ = WeaponComponentToWorld.GetRelativeTransform(SocketWorldTransform).Translation.Z;
        }
    }

    static uint64_t GetStaticMeshComponentScopeType(uint64_t Mesh) {
        uint64_t Result = 0;
        auto AttachedStaticComponentMap = mem.Read<TMap<TEnumAsByte<EWeaponAttachmentSlotID>, uint64_t>>(Mesh + GameData.Offset["AttachedStaticComponentMap"]);
        AttachedStaticComponentMap.GetValue(EWeaponAttachmentSlotID::UpperRail, Result);
        return Result;
    }

    static bool FindSocket(uint64_t StaticMesh, FName InSocketName, ULONG64& OutSocket) {
        if (InSocketName == NAME_None)
            return false;
        auto Sockets = mem.Read<TArray<uint64_t>>(StaticMesh + GameData.Offset["StaticSockets"]);
        if (!Sockets.size()) return false;
        for (const auto& SocketPtr : Sockets.GetVector()) {

            auto SocketName = mem.Read<FName>(SocketPtr + GameData.Offset["StaticSocketName"]);
            if (SocketName == InSocketName) {
                OutSocket = SocketPtr;
                return true;
            }
        }
        return false;

    }

    static bool GetSocketByName(uint64_t Mesh, FName InSocketName, ULONG64& OutSocket)
    {
        uint64_t StaticMMesh = mem.Read<uint64_t>(Mesh + GameData.Offset["StaticMesh"]);
        if (Utils::ValidPtr(StaticMMesh))
            return false;

        return FindSocket(StaticMMesh, InSocketName, OutSocket);
    }

    static std::pair<float, float> GetBulletDropAndTravelTime(const FVector& GunLocation, const FRotator& GunRotation, const FVector& TargetPos,
        float ZeroingDistance, float BulletDropAdd, float InitialSpeed, float TrajectoryGravityZ, float BallisticDragScale,
        float BallisticDropScale, float BDS, float SimulationSubstepTime, float VDragCoefficient, FRichCurve RichCurve, int KeysNum, std::vector<FRichCurveKey> Keys)
    {
        const float ZDistanceToTarget = TargetPos.Z - GunLocation.Z;
        const float DistanceToTarget = GunLocation.Distance(TargetPos) / 100.0f;
        float TravelTime = DistanceToTarget / InitialSpeed;
        float BulletDrop = 0.5f * TrajectoryGravityZ * TravelTime * TravelTime * 100.0f;

        float TravelTimeZero = ZeroingDistance / InitialSpeed;
        float BulletDropZero = 0.5f * TrajectoryGravityZ * TravelTimeZero * TravelTimeZero * 88.0f;

        if (KeysNum > 0)
        {
            SimulateWeaponTrajectory(GunRotation.GetUnitVector(), DistanceToTarget, TrajectoryGravityZ,
                BallisticDragScale, BallisticDropScale,
                BDS, SimulationSubstepTime,
                VDragCoefficient,
                RichCurve, KeysNum, Keys, BulletDrop, TravelTime);


            SimulateWeaponTrajectory(FVector(1.0f, 0.0f, 0.0f), ZeroingDistance, TrajectoryGravityZ,
                BallisticDragScale, BallisticDropScale, BDS, SimulationSubstepTime, VDragCoefficient,
                RichCurve, KeysNum, Keys, TravelTimeZero, BulletDropZero);
        }

        BulletDrop = fabsf(BulletDrop) - fabsf(BulletDropAdd);
        if (BulletDrop < 0.0f)
            BulletDrop = 0.0f;
        BulletDropZero = fabsf(BulletDropZero) + fabsf(BulletDropAdd);

        const float TargetPitch = asinf((ZDistanceToTarget + BulletDrop) / 100.0f / DistanceToTarget);
        const float ZeroPitch = IsNearlyZero(ZeroingDistance) ? 0.0f : atan2f(BulletDropZero / 100.0f, ZeroingDistance);
        const float FinalPitch = TargetPitch - ZeroPitch;
        const float AdditiveZ = DistanceToTarget * sinf(FinalPitch) * 100.0f - ZDistanceToTarget;

        return std::pair(AdditiveZ, TravelTime);
    }

    static void GrenadeHwind(float TargetDistance, float& ProjectHinght, float& Project)
    {
        if (TargetDistance > 10.0f && TargetDistance <= 20.0f)
        {
            float t = (TargetDistance - 10.0f) / (20.0f - 10.0f);
            ProjectHinght = -50 + t * (0 - (-50));
            Project = 4.2f + t * (3.7f - 4.2f);
        }
        else if (TargetDistance > 20.0f && TargetDistance <= 25.0f)
        {
            float t = (TargetDistance - 20.0f) / (25.0f - 20.0f);
            ProjectHinght = 0 + t * (0 - 0);
            Project = 3.7f + t * (3.4f - 3.7f);
        }
        else if (TargetDistance > 25.0f && TargetDistance <= 30.0f)
        {
            float t = (TargetDistance - 25.0f) / (30.0f - 25.0f);
            ProjectHinght = 0 + t * (50 - 0);
            Project = 3.4f + t * (3.1f - 3.4f);
        }
        else if (TargetDistance > 30.0f && TargetDistance <= 35.0f)
        {
            float t = (TargetDistance - 30.0f) / (35.0f - 30.0f);
            ProjectHinght = 50 + t * (80 - 50);
            Project = 3.1f + t * (2.8f - 3.1f);
        }
        else if (TargetDistance > 35.0f && TargetDistance <= 40.0f)
        {
            float t = (TargetDistance - 35.0f) / (40.0f - 35.0f);
            ProjectHinght = 80 + t * (500 - 80);
            Project = 2.8f + t * (2.5f - 2.8f);
        }
        else if (TargetDistance > 40.0f && TargetDistance <= 45.0f)
        {
            float t = (TargetDistance - 40.0f) / (45.0f - 40.0f);
            ProjectHinght = 500 + t * (800 - 500);
            Project = 2.5f + t * (2.3f - 2.5f);
        }
        else if (TargetDistance > 45.0f && TargetDistance <= 50.0f)
        {
            float t = (TargetDistance - 45.0f) / (50.0f - 45.0f);
            ProjectHinght = 800 + t * (1400 - 800);
            Project = 2.3f + t * (1.9f - 2.3f);
        }
        else if (TargetDistance > 50.0f && TargetDistance <= 55.0f)
        {
            float t = (TargetDistance - 50.0f) / (55.0f - 50.0f);
            ProjectHinght = 1400 + t * (1800 - 1400);
            Project = 1.9f + t * (1.6f - 1.9f);
        }
        else if (TargetDistance > 55.0f && TargetDistance <= 60.0f)
        {
            float t = (TargetDistance - 55.0f) / (60.0f - 55.0f);
            ProjectHinght = 1800 + t * (2300 - 1800);
            Project = 1.6f + t * (1.2f - 1.6f);
        }
        else if (TargetDistance > 60.0f && TargetDistance <= 65.0f)
        {
            ProjectHinght = 2300.0f;
            Project = 1.2f;
        }
        else
        {
            ProjectHinght = -50.0f;
            Project = 4.2f;
        }
    }


    static void RocketHwind(float TargetDistance, float& ProjectHeight, float& ProjectTime)
    {
        if (TargetDistance <= 35.0f) {
            ProjectHeight = -100.0f;
            ProjectTime = 0.4f;
        }
        else if (TargetDistance <= 70.0f) {
            ProjectHeight = -110.0f;
            ProjectTime = 0.7f;
        }
        else if (TargetDistance <= 90.0f) {
            ProjectHeight = 180.0f;
            ProjectTime = 1.0f;
        }
        else if (TargetDistance <= 100.0f) {
            ProjectHeight = 360.0f;
            ProjectTime = 1.3f;
        }
        else if (TargetDistance <= 110.0f) {
            ProjectHeight = 420.0f;
            ProjectTime = 1.4f;
        }
        else if (TargetDistance <= 120.0f) {
            ProjectHeight = 800.0f;
            ProjectTime = 1.8f;
        }
        else {
            ProjectHeight = 1600.0f;
            ProjectTime = 2.4f;
        }
    }



    static double calculateDistance(double x1, double y1, double x2, double y2) {
        return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
    }
    static void Run()
    {
        bool AimAndShot = false;
        auto hScatter = mem.CreateScatterHandle();
        auto hWriteScatter = mem.CreateScatterHandle();
        Throttler Throttlered;
        FName FMouseX = { GameData.Offset["MouseX"] };
        FName FMouseY = { GameData.Offset["MouseY"] };
        FInputAxisProperties MouseX;
        FInputAxisProperties MouseY;
        bool FistAim = false;
        AimBotConfig Config;
        Config.FPS = 360;

        while (true)
        {
            //Timer timer("1");

            Throttlered.executeTaskWithSleep("AimBotSleep", std::chrono::milliseconds(static_cast<int>(1000 / Config.FPS)), [] {});

            //std::cout << timer.get() << std::endl;
            //continue;

            //if (GameData.Config.AimBot.Controller == 0 && GameData.Config.AimBot.Delay > 0) std::this_thread::sleep_for(std::chrono::microseconds(GameData.Config.AimBot.Delay));

            if (GameData.Scene != Scene::Gaming)
            {
                Sleep(GameData.ThreadSleep);
                continue;
            }

            if (GameData.bShowMouseCursor || Utils::ValidPtr(GameData.AcknowledgedPawn) || !GameData.Config.AimBot.Enable || GameData.LocalPlayerInfo.Health <= 0)
            {
                if (GameData.LocalPlayerInfo.CurrentWeaponIndex != 255)
                {
                    StopAiming();
                    continue;
                }
            }

            EAnimPawnState PreEvalPawnState;
            if (GameData.LocalPlayerInfo.CurrentWeaponIndex == 255)
            {

                mem.AddScatterRead(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["PreEvalPawnState"], (EAnimPawnState*)&PreEvalPawnState);
                mem.ExecuteReadScatter(hScatter);

                if (PreEvalPawnState != EAnimPawnState::PS_MortarDriver)
                {
                    StopAiming();
                    continue;

                }
            }

            mem.AddScatterRead(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["WeaponConfig_WeaponClass"], (BYTE*)&GameData.LocalPlayerInfo.WeaponClassByte);
            mem.ExecuteReadScatter(hScatter);




            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Other)
            {
                bool isMelee = ((int)GameData.LocalPlayerInfo.WeaponClassByte == (int)EWeaponClass::Class_Melee);
                if (!isMelee) {
                    StopAiming();
                    continue;
                }
            }
            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade && !GameData.Config.AimBot.GrenadePredict)
            {
                StopAiming();
                continue;
            }
            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1 && !GameData.Config.AimBot.PanzerFaust)
            {

                StopAiming();
                continue;
            }
            bool isMelee = ((int)GameData.LocalPlayerInfo.WeaponClassByte == (int)EWeaponClass::Class_Melee);
            Config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];
            if (isMelee) {
                Config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon["SR"];
                Config.Prediction = true;
                Config.AimDistance = 30.0f;
            }
            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade) {

                Config.bIsScoping_CP = false;
                Config.HotkeyMerge = false;
                Config.AimDistance = 100.0f;    // 设置手雷最大瞄准距离为100米
            }
            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1) {

                Config.AimDistance = 120.0f;    // 设置手雷最大瞄准距离为100米
            }
            if (Config.bIsScoping_CP && !GameData.LocalPlayerInfo.IsScoping) {
                StopAiming();
                continue;
            }
            if (!Config.enable)
            {
                StopAiming();
                continue;
            }

            FVector TargetPos;
            float TargetDistance;
            FVector TargetVelocity;
            FVector LastUpdateRotation;
            FVector TargetAcceleration;
            bool IsScoping = false;
            bool IsReloading = false;
            FRotator Recoil;
            FRotator ControlRotation;
            float LeanLeftAlpha_CP = 0.f;
            float LeanRightAlpha_CP = 0.f;
            bool NeedEndHook = false;

            bool IsFirstKey = GameData.Keyboard.IsKeyDown(Config.First.Key);

            bool IsSecondKey = GameData.Keyboard.IsKeyDown(Config.Second.Key);
            bool IsGroggyKey = GameData.Keyboard.IsKeyDown(Config.Groggy.Key);
            bool IsWheelKey = GameData.Keyboard.IsKeyDown(Config.Wheel.Key);

            bool IsGrenadeKey = GameData.Keyboard.IsKeyDown(GameData.Config.AimBot.Grenade);
            bool IsMortarKey = GameData.Keyboard.IsKeyDown(GameData.Config.AimBot.Mortar);

            bool AutoShotgunAimActive = Config.AutomaticShooting &&
                (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SG ||
                    GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::AR ||
                    GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SMG ||
                    GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::DMR ||

                    GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::LMG);

            bool IsAutomaticKey = false;

            // 先默认不包含手雷键
            bool CanAim = IsFirstKey || IsSecondKey || IsGroggyKey || IsWheelKey || IsAutomaticKey || AutoShotgunAimActive;
            if (isMelee) {
                CanAim = GameData.Keyboard.IsKeyDown(VK_RBUTTON);
            }

            // 然后根据武器类型特殊处理
            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade) {
                CanAim = IsGrenadeKey; // 手雷只用手雷键
            }
            else if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1) {
                CanAim = IsMortarKey; // 火箭筒只用迫击炮键
            }


            bool InScopePoint = (Config.HotkeyMerge && !IsFirstKey && IsSecondKey && GameData.LocalPlayerInfo.IsScoping && GameData.Config.AimBot.ShowPoint);

            SHORT BulletNumber = 1.f;
            mem.AddScatterRead(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["CurrentAmmoData"], (SHORT*)&BulletNumber);
            mem.ExecuteReadScatter(hScatter);

            Player Player = Data::GetPlayersItem(GameData.AimBot.Target);


            if (CanAim || (GameData.LocalPlayerInfo.IsScoping && GameData.Config.AimBot.ShowPoint) || PreEvalPawnState == EAnimPawnState::PS_MortarDriver)
            {
                if (Utils::ValidPtr(GameData.AcknowledgedPawn))
                {
                    Sleep(1);
                    continue;
                }

                if (InScopePoint) // 如果处于开镜瞄准点状态
                {
                    CanAim = false; // 禁用瞄准功能
                }
                else // 否则
                {
                    // 热键合并模式 + 未按下第一按键 + 按下第二按键
                    if (Config.HotkeyMerge && !IsFirstKey && IsSecondKey)
                    {
                        bool inForceWindowEarly = (Player.Distance >= 75 && Player.Distance <= 200);
                        if (!inForceWindowEarly) {
                            StopAiming();
                            continue;
                        }
                        CanAim = true;
                    }
                    // 热键合并模式 + 按下第一按键 + 未按下第二按键 + 第一配置骨骼未全禁用
                    else if (Config.HotkeyMerge && IsFirstKey && !IsSecondKey && !GetBoneIsAllFalse(Config.First.Bones))
                    {
                        StopAiming();
                        continue;
                    }
                    // 非热键合并模式 + 按下第一按键 + 未按下第二按键 + 第一配置骨骼未全禁用
                    else if (!Config.HotkeyMerge && IsFirstKey && !IsSecondKey && !GetBoneIsAllFalse(Config.First.Bones))
                    {
                        StopAiming();
                        continue;
                    }
                    // 非热键合并模式 + 按下迫击炮键 + 第一配置骨骼未全禁用
                    else if (!Config.HotkeyMerge && IsMortarKey && !GetBoneIsAllFalse(Config.First.Bones))
                    {
                        StopAiming();
                        continue;
                    }
                    else if (!Config.HotkeyMerge && IsGrenadeKey && !IsFirstKey && !GetBoneIsAllFalse(Config.First.Bones))
                    {
                        StopAiming();
                        continue;
                    }
                    // 非热键合并模式 + 未按下第一按键 + 按下第二按键 + 第二配置骨骼未全禁用
                    else if (!Config.HotkeyMerge && !IsFirstKey && IsSecondKey && !GetBoneIsAllFalse(Config.Second.Bones))
                    {
                        StopAiming();
                        continue;
                    }
                }

                // 倒地目标强制按下倒地自瞄热键
                if (((Player.GroggyHealth <= 99 && Player.GroggyHealth > 0) ||
                    Player.State == CharacterState::Groggy ||
                    Player.CharacterState == ECharacterState::BeHit) &&
                    !IsGroggyKey)
                {
                    StopAiming();
                    continue;
                }

                std::unordered_map<uint64_t, tMapInfo> EnemyInfoMap = Data::GetEnemyInfoMap();
                if (GameData.LocalPlayerInfo.CurrentWeaponIndex != 255 && GameData.LocalPlayerInfo.CurrentWeaponIndex != 4)
                {
                    uint64_t WeaponTrajectoryData = 0;
                    FWeaponTrajectoryConfig TrajectoryConfig{};
                    uint64_t BallisticCurve = 0;
                    FTransform WeaponComponentToWorld;
                    if (Utils::ValidPtr(LastCurrentWeapon) || LastCurrentWeapon != GameData.LocalPlayerInfo.CurrentWeapon)
                    {
                        if (!isMelee) {
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["WeaponTrajectoryData"], (uint64_t*)&WeaponTrajectoryData);
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["TrajectoryGravityZ"], (float*)&CurrentWeaponData.TrajectoryGravityZ);
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["FiringAttachPoint"], (FName*)&CurrentWeaponData.FiringAttachPoint);
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["ScopingAttachPoint"], (FName*)&CurrentWeaponData.ScopingAttachPoint);
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["Mesh3P"], (uint64_t*)&CurrentWeaponData.Mesh3P);
                            mem.ExecuteReadScatter(hScatter);

                            if (Utils::ValidPtr(WeaponTrajectoryData))
                            {

                                if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::Grenade &&
                                    GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::PanzerFaust100M1)
                                {
                                    StopAiming();
                                    continue;

                                }
                            }
                        }

                        if (!isMelee) {
                            mem.AddScatterReadRequest(hScatter, WeaponTrajectoryData + GameData.Offset["TrajectoryConfig"], &CurrentWeaponData.TrajectoryConfig);
                            mem.ExecuteReadScatter(hScatter);

                            CurrentWeaponData.Mesh3P = Decrypt::Xe(CurrentWeaponData.Mesh3P);
                            TrajectoryConfig.BallisticCurve = CurrentWeaponData.GetTrajectoryConfig<uint64_t>(GameData.Offset["BallisticCurve"]);
                            CurrentWeaponData.TrajectoryConfigs.InitialSpeed = CurrentWeaponData.GetTrajectoryConfig<float>(0);
                            CurrentWeaponData.TrajectoryConfigs.SimulationSubstepTime = CurrentWeaponData.GetTrajectoryConfig<float>(0x40);
                            CurrentWeaponData.TrajectoryConfigs.VDragCoefficient = CurrentWeaponData.GetTrajectoryConfig<float>(0x44);
                            CurrentWeaponData.TrajectoryConfigs.BDS = CurrentWeaponData.GetTrajectoryConfig<float>(0x48);
                            CurrentWeaponData.TrajectoryConfigs.ReferenceDistance = CurrentWeaponData.GetTrajectoryConfig<float>(0x34);

                            if (CurrentWeaponData.TrajectoryConfigs.InitialSpeed < 100)
                            {
                                CurrentWeaponData.TrajectoryConfigs.InitialSpeed = 800;
                            }

                            mem.AddScatterReadRequest(hScatter, TrajectoryConfig.BallisticCurve + GameData.Offset["FloatCurves"], (FRichCurve*)&CurrentWeaponData.FloatCurves);
                            mem.AddScatterReadRequest(hScatter, TrajectoryConfig.BallisticCurve + GameData.Offset["FloatCurves"] + GameData.Offset["Keys"], (FRichCurveKeyArray*)&CurrentWeaponData.RichCurveKeyArray);
                            mem.ExecuteReadScatter(hScatter);

                            std::vector<FRichCurveKey> Keys(CurrentWeaponData.RichCurveKeyArray.Count);

                            mem.Read(CurrentWeaponData.RichCurveKeyArray.Data, Keys.data(), sizeof(FRichCurveKey) * CurrentWeaponData.RichCurveKeyArray.Count);
                            CurrentWeaponData.RichCurveKeys = Keys;
                        }

                        LastCurrentWeapon = GameData.LocalPlayerInfo.CurrentWeapon;
                    }
                    uint64_t InputAxisProperties;
                    int InputAxisPropertiesCount;
                    if (!FistAim)
                    {
                        mem.AddScatterReadRequest(hScatter, GameData.PlayerInput + GameData.Offset["InputAxisProperties"], (uint64_t*)&InputAxisProperties);
                        mem.AddScatterReadRequest(hScatter, GameData.PlayerInput + GameData.Offset["InputAxisProperties"] + 0x8, (int*)&InputAxisPropertiesCount);
                        if (!isMelee) {
                            mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["Mesh3P"], (uint64_t*)&CurrentWeaponData.Mesh3P);
                            mem.ExecuteReadScatter(hScatter);
                            CurrentWeaponData.Mesh3P = Decrypt::Xe(CurrentWeaponData.Mesh3P);

                            if (Utils::ValidPtr(CurrentWeaponData.Mesh3P))
                            {
                                StopAiming();
                                continue;
                            }
                        }

                        if (GameData.Config.AimBot.Connected)
                        {
                            if (GameData.Config.AimBot.Controller == 0)
                            {
                                KmBox::Clear();
                            }

                            if (GameData.Config.AimBot.Controller == 1)
                            {
                                KmBoxNet::Clear();
                            }
                        }
                    }

                    if (!FistAim)
                    {
                        std::vector<std::pair<FName, FInputAxisProperties>> InputAxisPropertiesLists;

                        for (int i = 0; i < InputAxisPropertiesCount; i++)
                        {
                            std::pair<FName, FInputAxisProperties> Info;
                            InputAxisPropertiesLists.push_back(Info);
                        }

                        int i = 0;
                        for (auto& InputAxis : InputAxisPropertiesLists)
                        {
                            mem.AddScatterReadRequest(hScatter, InputAxisProperties + (i * (sizeof(FName) + sizeof(FInputAxisProperties))), (std::pair<FName, FInputAxisProperties>*) & InputAxis);
                            i++;
                        }

                        mem.ExecuteReadScatter(hScatter);


                        for (auto& InputAxis : InputAxisPropertiesLists)
                        {

                            if (InputAxis.first.ComparisonIndex == FMouseX.ComparisonIndex || InputAxis.first.Number == FMouseX.ComparisonIndex)
                            {
                                MouseX = InputAxis.second;
                            }
                            if (InputAxis.first.ComparisonIndex == FMouseY.ComparisonIndex || InputAxis.first.Number == FMouseX.ComparisonIndex)
                            {
                                MouseY = InputAxis.second;
                            }

                        }

                        FistAim = true;
                    }

                }


                //Utils::Log(1, "CurrentWeaponData.FiringAttachPoin: %d", CurrentWeaponData.FiringAttachPoint.ComparisonIndex);
               // Utils::Log(1, "CurrentWeaponData.ScopingAttachPoint: %d", CurrentWeaponData.ScopingAttachPoint.ComparisonIndex);


                if (!GameData.AimBot.Lock && !Utils::ValidPtr(CurrentWeaponData.Mesh3P))
                {
                    CurrentWeaponData.ScopeStaticMeshComponent = GetStaticMeshComponentScopeType(CurrentWeaponData.Mesh3P);
                    CurrentWeaponData.ScopeSocket = GetSocketByName(CurrentWeaponData.ScopeStaticMeshComponent, CurrentWeaponData.ScopingAttachPoint, CurrentWeaponData.ScopeAimCameraSocket);
                }

                if (false)
                {

                    FistAim = true;
                    CurrentWeaponData.SkeletalMesh = mem.Read<uint64_t>(CurrentWeaponData.Mesh3P + GameData.Offset["SkeletalMesh"]);
                    CurrentWeaponData.Skeleton = mem.Read<uint64_t>(CurrentWeaponData.SkeletalMesh + GameData.Offset["Skeleton"]);
                    CurrentWeaponData.SkeletalSockets = mem.Read<TArray<uint64_t>>(CurrentWeaponData.Skeleton + GameData.Offset["SkeletalSockets"]);

                    CurrentWeaponData.SkeletalMeshSockets.clear();

                    for (auto& pSocket : CurrentWeaponData.SkeletalSockets.GetVector())
                    {
                        CurrentWeaponData.SkeletalMeshSockets.push_back({ pSocket });
                    }

                    for (auto& SocketItem : CurrentWeaponData.SkeletalMeshSockets)
                    {
                        //Utils::Log(1, "SocketItem.pSocket: %p", SocketItem.pSocket);
                        mem.AddScatterRead(hScatter, SocketItem.pSocket + GameData.Offset["SkeletalSocketName"], (FName*)&SocketItem.SocketName);
                    }
                    mem.ExecuteReadScatter(hScatter);

                    for (auto& SocketItem : CurrentWeaponData.SkeletalMeshSockets)
                    {
                        if (SocketItem.SocketName == CurrentWeaponData.FiringAttachPoint)
                        {
                            CurrentWeaponData.FiringAttachPointSocketBone = SocketItem.pSocket;
                            FRotator RelativeRotation = mem.Read<FRotator>(CurrentWeaponData.FiringAttachPointSocketBone + 0x44);
                            FVector Direction = RelativeRotation.GetUnitVector();
                            Direction.Normalize();

                            //Trajectory.Location = FiringLocation;
                            //Utils::Log(1, "SocketItem: %d %f %f %f", SocketItem.SocketName.ComparisonIndex, Direction.X, Direction.Y, Direction.Z);
                        }
                    }

                }
                //Player Player = Data::GetPlayersItem(GameData.AimBot.Target);
                int AimBone = GameData.AimBot.Bone;
                if (isMelee) {
                    AimBone = EBoneIndex::Head;
                }

                if (GameData.AimBot.Target == 0)
                {
                    StopAiming();
                    continue;
                }


                if (GameData.AimBot.Type == EntityType::Player)
                {
                    // 热键合并模式:只有在目标在准星内且距离不在75-200米范围时才允许锁定
                    if (Config.HotkeyMerge && !IsFirstKey && IsSecondKey)
                    {
                        if (!InScopePoint || (Player.Distance >= 75 && Player.Distance <= 200)) {
                            StopAiming();
                            continue;
                        }
                    }

                    if (CanAim && GameData.AimBot.Target != 0) GameData.AimBot.Lock = true;
                    //Player Player = Data::GetPlayersItem(GameData.AimBot.Target);
                    int AimBone = GameData.AimBot.Bone;
                    if (isMelee) {
                        AimBone = EBoneIndex::Head;
                    }

                    //拉枪自瞄
                    if (Config.FOVenable) {
                        if (isMelee) AimBone = EBoneIndex::Head;
                        FVector2D ScreenLocation = Player.Skeleton.RawScreenBones[AimBone];
                        float Distance = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, ScreenLocation.X, ScreenLocation.Y);

                        if (Distance > Config.FOV) {
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::Grenade && GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::PanzerFaust100M1)
                            {
                                if (!(IsFirstKey && IsSecondKey && Player.Distance >= 75 && Player.Distance <= 200)) {
                                    StopAiming();
                                    continue;
                                }
                            }
                        }
                    }

                    // 首先处理强制部位逻辑
                    int SecondBone = -1;
                    bool useForceSecondBone = false; // 标记是否使用强制第二部位

                    if (IsFirstKey && IsSecondKey)
                    {
                        for (size_t i = 0; i < 17; i++)
                        {
                            if (Config.Second.Bones[i])
                            {
                                SecondBone = BoneIndex[i];
                                break;
                            }
                        }
                        if (SecondBone != -1) {
                            AimBone = SecondBone;
                            useForceSecondBone = true; // 标记已使用强制第二部位
                        }
                    }

                    bool inForceWindow = (Player.Distance >= 75 && Player.Distance <= 200);
                    bool useForce = (useForceSecondBone && inForceWindow);
                    bool respectPrimary = (!useForceSecondBone && inForceWindow);

                    if (useForce) {
                        AimBone = SecondBone;
                        Player.IsVisible = true;
                        Config.VisibleCheck = false;
                        LineTraceSingleRecoilLocation = FVector(0.f, 0.f, 0.f);
                        LineTraceSingleRecoilTimeStartTime = 0;
                        RecoilLocation = FVector(0.f, 0.f, 0.f);
                        RecoilTimeStartTime = 0;
                    }
                    else {
                        if (Player.Skeleton.VisibleBones[GameData.AimBot.Bone] || Player.Skeleton.VisibleBones[SecondBone]) {
                            if (IsFirstKey && IsSecondKey) {
                                AimBone = SecondBone;
                            }
                            else {
                                AimBone = GameData.AimBot.Bone;
                            }
                        }
                        else {
                            for (auto bone : Player.Skeleton.BonePriority) {
                                if (Player.Skeleton.VisibleBones[bone]) {
                                    AimBone = bone;
                                    Player.IsVisible = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!useForceSecondBone && inForceWindow && AimBone == EBoneIndex::ForeHead) {
                        AimBone = EBoneIndex::Head;
                    }

                    if (CanAim && Config.RandomBodyParts && !useForceSecondBone) {
                        int randomBone = GetRandomBodyPart(Config);
                        if (randomBone != -1) {
                            AimBone = randomBone;
                        }
                    }

                    if (respectPrimary) {
                        AimBone = GameData.AimBot.Bone;
                        LineTraceSingleRecoilLocation = FVector(0.f, 0.f, 0.f);
                        LineTraceSingleRecoilTimeStartTime = 0;
                    }


                    if (!useForce) {
                        if (CanAim && Config.LineTraceSingle && RecoilTimeStartTime == 0 && Player.State != CharacterState::Dead)
                        {
 
                            EBoneIndex VisibilityBoneIndex;
                            EBoneIndex targetBone = static_cast<EBoneIndex>(AimBone);
                            if (LineTraceHook::GetLocation(Player, targetBone, &VisibilityBoneIndex))
                            {
                                AimBone = VisibilityBoneIndex;
                                Player = Data::GetPlayersItem(GameData.AimBot.Target);
                                LineTraceSingleRecoilLocation = Player.Skeleton.RawLocationBones[AimBone];
                                LineTraceSingleRecoilTimeStartTime = 0;
                                Config.VisibleCheck = false;
                                Player.IsVisible = true;
                            }
                            else if (LineTraceSingleRecoilLocation == FVector(0.f, 0.f, 0.f)) {
                                Config.VisibleCheck = true;
                                Player.IsVisible = false;
                                GameData.AimBot.PredictedPos = FVector();
                            }
 
 
                        }
                        else
                        {
                            TargetPos = Player.Skeleton.RawLocationBones[AimBone];
                        }
                    } else {
                        TargetPos = Player.Skeleton.RawLocationBones[AimBone];
                        Player.IsVisible = true;
                        Config.VisibleCheck = false;
                        LineTraceSingleRecoilLocation = FVector(0.f, 0.f, 0.f);
                        LineTraceSingleRecoilTimeStartTime = 0;
                        RecoilLocation = FVector(0.f, 0.f, 0.f);
                        RecoilTimeStartTime = 0;
                    }

                    if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade || GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1)
                    {
                        Config.VisibleCheck = false;
                        Player.IsVisible = true;
                        TargetPos = Player.Skeleton.RawLocationBones[AimBone];
                    }
                    else
                    {
                        if (!((!useForceSecondBone) && Player.Distance >= 75 && Player.Distance <= 200)) {
                            if (CanAim && Config.LineTraceSingle && RecoilTimeStartTime == 0 && Player.State != CharacterState::Dead)
                            {
                                EBoneIndex VisibilityBoneIndex;
                                EBoneIndex targetBone = static_cast<EBoneIndex>(AimBone);
                                if (LineTraceHook::GetLocation(Player, targetBone, &VisibilityBoneIndex))
                                {
                                    if (VisibilityBoneIndex == targetBone) {
                                        AimBone = VisibilityBoneIndex;
                                    }
                                    Player = Data::GetPlayersItem(GameData.AimBot.Target);
                                    LineTraceSingleRecoilLocation = Player.Skeleton.RawLocationBones[AimBone];
                                    LineTraceSingleRecoilTimeStartTime = 0;
                                    Config.VisibleCheck = false;
                                    Player.IsVisible = true;
                                }
                                else if (LineTraceSingleRecoilLocation == FVector(0.f, 0.f, 0.f))
                                {
                                    Config.VisibleCheck = true;
                                    Player.IsVisible = false;
                                    GameData.AimBot.PredictedPos = FVector();
                                }
                            }
                            else
                            {
                                TargetPos = Player.Skeleton.RawLocationBones[AimBone];
                            }
                        }
                        else {
                            TargetPos = Player.Skeleton.RawLocationBones[AimBone];
                        }
                    }

                    uint64_t VehicleRiderComponent;
                    uint64_t MovementComponent;
                    int SeatIndex = -1;

                    mem.AddScatterRead(hScatter, Player.Entity + GameData.Offset["VehicleRiderComponent"], (uint64_t*)&VehicleRiderComponent);
                    mem.ExecuteReadScatter(hScatter);

                    mem.AddScatterRead(hScatter, VehicleRiderComponent + GameData.Offset["LastVehiclePawn"], (uint64_t*)&MovementComponent);
                    mem.AddScatterRead(hScatter, VehicleRiderComponent + GameData.Offset["SeatIndex"], (int*)&SeatIndex);
                    mem.ExecuteReadScatter(hScatter);

                    //TargetPos = Player.Skeleton.LocationBones[AimBone];
                    TargetDistance = GameData.Camera.Location.Distance(Player.Location) / 100.0f;
                    FVector ReplicatedMovement;

                    if (LineTraceSingleRecoilLocation != FVector(0.f, 0.f, 0.f))
                    {
                        //Utils::Log(1, "%f %f %f", LineTraceSingleRecoilLocation.X, LineTraceSingleRecoilLocation.Y, LineTraceSingleRecoilLocation.Z);
                        if (LineTraceSingleRecoilTimeStartTime == 0)
                        {
                            LineTraceSingleRecoilTimeStartTime = GetTickCount64();
                        }
                        else if ((GetTickCount64() - LineTraceSingleRecoilTimeStartTime >= Config.RecoilTime * 200) || Config.RecoilTime <= 0) {
                            LineTraceSingleRecoilTimeStartTime = 0;
                            LineTraceSingleRecoilLocation = FVector(0.f, 0.f, 0.f);
                        }

                        TargetPos = LineTraceSingleRecoilLocation;
                    }

                    if (Player.Health <= 0) {
                        bool ShouldSwitchTarget = Config.AutoSwitch &&
                            ((Config.IgnoreGroggy) || (!Config.IgnoreGroggy && Player.GroggyHealth <= 0));

                        if (ShouldSwitchTarget) {
                            Config.SwitchingDelay += Config.RecoilTime;

                            bool TimeElapsed = Config.SwitchingDelay > 0 &&
                                (GetTickCount64() - AutoSwitchTargetStartTime >= Config.SwitchingDelay * 100);

                            if (Config.SwitchingDelay > 0 && AutoSwitchTargetStartTime == 0) {
                                AutoSwitchTargetStartTime = GetTickCount64();
                                RecoilLocation = TargetPos;
                                Data::SetEnemyInfoMap({});
                            }
                            else if (TimeElapsed || Config.SwitchingDelay <= 0) {
                                Data::SetEnemyInfoMap({});
                                AutoSwitchTargetStartTime = 0;
                                RecoilLocation = TargetPos;
                                GameData.AimBot.Lock = false;
                                GameData.AimBot.Target = 0;
                            }
                        }
                        else {
                            if ((Config.IgnoreGroggy) || (!Config.IgnoreGroggy && Player.GroggyHealth <= 0)) {
                                if (Config.RecoilTime > 0 && RecoilTimeStartTime == 0) {
                                    RecoilTimeStartTime = GetTickCount64();
                                    RecoilLocation = TargetPos;
                                    Data::SetEnemyInfoMap({});
                                }
                                else if ((GetTickCount64() - RecoilTimeStartTime >= Config.RecoilTime * 100) || Config.RecoilTime <= 0) {
                                    Sleep(1);
                                    continue;
                                }
                                else {
                                    TargetPos = RecoilLocation;
                                }
                            }
                        }

                        if (RecoilLocation != FVector(0.f, 0.f, 0.f))
                        {
                            TargetPos = RecoilLocation;
                        }
                    }

                    if (TargetPos.X == 0 || TargetPos.Y == 0 || TargetPos.Z == 0)
                    {
                        StopAiming();
                        continue;
                    }

                    if (Config.IgnoreGroggy && !IsGroggyKey && Player.GroggyHealth <= 99 && Player.GroggyHealth > 0 && RecoilTimeStartTime == 0)
                    {
                        StopAiming();
                        continue;
                    }

                    if (Config.VisibleCheck && !Player.IsVisible)
                    {
                        StopAiming();
                        GameData.AimBot.PredictedPos = FVector();
                        continue;
                    }

                    if (SeatIndex == -1)
                    {
                        mem.AddScatterRead(hScatter, Player.MeshComponent + GameData.Offset["ComponentToWorld"], (FTransform*)&Player.ComponentToWorld);
                        mem.AddScatterRead(hScatter, Player.CharacterMovement + GameData.Offset["LastUpdateVelocity"], (FVector*)&TargetVelocity);
                        //mem.AddScatterRead(hScatter, Player.CharacterMovement + GameData.Offset["LastUpdateVelocity"] - 0x28, (FVector*)&TargetAcceleration);
                        mem.AddScatterRead(hScatter, Player.AnimScriptInstance + GameData.Offset["PreEvalPawnState"], (EAnimPawnState*)&Player.PreEvalPawnState);
                        //mem.AddScatterRead(hScatter, Player.CharacterMovement + GameData.Offset["LastUpdateVelocity"] - 0x10, (FVector*)&LastUpdateRotation);
                        //mem.AddScatterRead(hScatter, Player.RootComponent + GameData.Offset["ComponentVelocity"], (FVector*)&LastUpdateRotation);
                    }
                    else {
                        mem.AddScatterRead(hScatter, MovementComponent + GameData.Offset["ReplicatedMovement"], (FVector*)&ReplicatedMovement);
                    }
                    mem.ExecuteReadScatter(hScatter);

                    Player.Location = Player.ComponentToWorld.Translation;

                    if (RecoilTimeStartTime == 0)
                    {
                        float TimeStampDelta = GameData.WorldTimeSeconds - EnemyInfoMap[Player.Entity].TimeStamp;
                        EnemyInfoMap[Player.Entity].TimeStamp = GameData.WorldTimeSeconds;
                        //PosInfo
                        [&] {
                            auto& PosInfo = EnemyInfoMap[Player.Entity].PosInfo.Info;

                            if (Player.State == CharacterState::Dead) {
                                PosInfo.clear();
                            }
                            else {

                                if (TimeStampDelta)
                                    PosInfo.push_front({ GameData.WorldTimeSeconds, Player.Location });

                                //Utils::Log(1, "GameData.WorldTimeSeconds %f", GameData.WorldTimeSeconds);

                                if (PosInfo.size() > 200)
                                    PosInfo.pop_back();

                                float SumTimeDelta = 0.0f;
                                FVector SumPosDif;

                                for (size_t i = 1; i < PosInfo.size(); i++) {
                                    const float DeltaTime = PosInfo[i - 1].Time - PosInfo[i].Time;
                                    const FVector DeltaPos = PosInfo[i - 1].Pos - PosInfo[i].Pos;
                                    const FVector DeltaVelocity = DeltaPos * (1.0f / DeltaTime);
                                    const float DeltaSpeedPerHour = DeltaVelocity.Length() / 100.0f * 3.6f;

                                    if (DeltaTime > 0.05f || DeltaSpeedPerHour > 500.0f) {
                                        PosInfo.clear();
                                    }
                                    else {
                                        SumTimeDelta = SumTimeDelta + DeltaTime;
                                        SumPosDif = SumPosDif + DeltaPos;

                                        if (SumTimeDelta > 0.15f)
                                            break;
                                    }
                                }
                                if (SumTimeDelta > 0.1f) {
                                    Player.Velocity = SumPosDif * (1.0f / SumTimeDelta);
                                }
                            }
                            }();
                        Data::SetEnemyInfoMap(EnemyInfoMap);
                    }

                    if (SeatIndex == -1)
                    {
                        if (Player.PreEvalPawnState == EAnimPawnState::PS_SecondaryLocomotion || Config.PredictionMode == 1)
                        {
                            TargetVelocity = Player.Velocity;
                        }
                        else {
                            TargetVelocity.Z = Player.Velocity.Z;

                        }
                    }
                    else {
                        TargetVelocity = ReplicatedMovement;
                    }

                    // Utils::Log(1, "TargetAcceleration: %f %f %f", TargetAcceleration.X, TargetAcceleration.Y, TargetAcceleration.Z);
                }
                else if (GameData.AimBot.Type == EntityType::Project)
                {
                    auto ProjectsMap = Data::GetProjects();
                    auto it = ProjectsMap.find(GameData.AimBot.Target);
                    if (it == ProjectsMap.end())
                    {
                        StopAiming();
                        continue;
                    }
                    // 如果投掷物已经进入爆炸流程或被判定为不可见（生命周期结束），立即停止自瞄
                    if (it->second.bVisible == 1 || it->second.ExplodeState != EProjectileExplodeState::NotExplode)
                    {
                        StopAiming();
                        continue;
                    }
                    if (CanAim) GameData.AimBot.Lock = true;
                    TargetPos = it->second.Location;
                    TargetVelocity = { 0.f, 0.f, 0.f };
                    TargetDistance = GameData.Camera.Location.Distance(TargetPos) / 100.0f;
                }
                else if (GameData.AimBot.Type == EntityType::Wheel) {
                    if (!IsWheelKey)
                    {
                        StopAiming();
                        continue;
                    }

                    if (CanAim) GameData.AimBot.Lock = true;
                    FRotator VehicleRotator;
                    VehicleWheelInfo Wheel = Data::GetVehicleWheelsItem(GameData.AimBot.Target);
                    mem.AddScatterReadRequest(hScatter, Wheel.Wheel + GameData.Offset["WheelLocation"], &Wheel.Location);
                    mem.AddScatterReadRequest(hScatter, Wheel.Wheel + GameData.Offset["DampingRate"], &Wheel.DampingRate);
                    mem.AddScatterReadRequest(hScatter, Wheel.Vehicle + GameData.Offset["ReplicatedMovement"], (FVector*)&TargetVelocity);
                    mem.AddScatterReadRequest(hScatter, Wheel.Vehicle + GameData.Offset["ReplicatedMovement"] + 0x24, (FRotator*)&VehicleRotator);
                    mem.ExecuteReadScatter(hScatter);

                    TargetPos = Wheel.Location;

                    auto UpdatePost = VectorHelper::RotateVector(VehicleRotator.GetMatrix(), TargetPos);

                    if (Config.AimWheelBone == 0)
                    {
                        TargetPos.Z += VectorHelper::RotateVector(VehicleRotator.GetMatrix(), { 0, 0, Wheel.ShapeRadius - 5 }).Z;
                    }
                    else if (Config.AimWheelBone == 1)
                    {
                        TargetPos.Z += VectorHelper::RotateVector(VehicleRotator.GetMatrix(), { 0, 0, 10 }).Z;
                    }
                    else if (Config.AimWheelBone == 3)
                    {
                        TargetPos.Z -= VectorHelper::RotateVector(VehicleRotator.GetMatrix(), { 0, 0, 10 }).Z;
                    }
                    else if (Config.AimWheelBone == 4)
                    {
                        TargetPos.Z -= VectorHelper::RotateVector(VehicleRotator.GetMatrix(), { 0, 0, Wheel.ShapeRadius - 5 }).Z;
                    }

                    TargetDistance = GameData.Camera.Location.Distance(Wheel.Location) / 100.0f;

                    if (Wheel.DampingRate > 2.f || Wheel.DampingRate == 0.1f || Wheel.DampingRate == 0.0f)
                    {
                        Wheel.State = WheelState::FlatTire;
                    }

                    if (Wheel.State == WheelState::FlatTire) {
                        StopAiming();
                        continue;
                    }
                }

                SHORT BulletNumber = 1.f;

                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["bIsScoping_CP"], (bool*)&IsScoping);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["bIsReloading_CP"], (bool*)&IsReloading);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["RecoilADSRotation_CP"], (FRotator*)&Recoil);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["ControlRotation_CP"], (FRotator*)&ControlRotation);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["LeanLeftAlpha_CP"], (float*)&LeanLeftAlpha_CP);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.AnimScriptInstance + GameData.Offset["LeanRightAlpha_CP"], (float*)&LeanRightAlpha_CP);
                mem.AddScatterReadRequest(hScatter, CurrentWeaponData.Mesh3P + GameData.Offset["ComponentToWorld"], (FTransform*)&CurrentWeaponData.ComponentToWorld);
                mem.AddScatterReadRequest(hScatter, GameData.LocalPlayerInfo.CurrentWeapon + GameData.Offset["CurrentAmmoData"], (SHORT*)&BulletNumber);

                mem.ExecuteReadScatter(hScatter);

                if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SR) {
                    if (IsScoping && !g_prevIsScoping) {
                        GameData.AimBot.Lock = false;
                        GameData.AimBot.Target = 0;
                        LineTraceSingleRecoilLocation = FVector(0.f, 0.f, 0.f);
                        LineTraceSingleRecoilTimeStartTime = 0;
                    }
                    g_prevIsScoping = IsScoping;
                }

                if (IsReloading || (Config.NoBulletNotAim && BulletNumber == 0))
                {

                    if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::Grenade && GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::PanzerFaust100M1)
                    {
                        StopAiming();
                        continue;
                    }

                }

                if (Config.IsScopeandAim && !IsScoping)
                {

                    StopAiming();
                    continue;

                }

                float ScopingAttachPointRelativeZ = 0.f;
                FTransform SocketWorldTransform;
                FTransform ScopeMeshComponentToWorld;
                GetScopingAttachPointRelativeZ(hScatter, CurrentWeaponData.ComponentToWorld, ScopingAttachPointRelativeZ, SocketWorldTransform, ScopeMeshComponentToWorld);
                float BulletDropAdd = ScopingAttachPointRelativeZ - CurrentWeaponData.ComponentToWorld.GetRelativeTransform(SocketWorldTransform).Translation.Z;

                FTransform GunTransform = CurrentWeaponData.ComponentToWorld;
                FVector GunLocation = GunTransform.Translation;
                FRotator GunRotation = GunTransform.Rotation;
                FVector AimLocation = GunLocation.Length() > 0.0f ? GunLocation : GameData.LocalPlayerInfo.ComponentToWorld.Translation;
                FRotator AimRotation = (IsScoping) ? GunRotation : ControlRotation;

                if (!Config.NoRecoil && !Config.OriginalRecoil)
                {
                    AimRotation = AimRotation - Recoil;
                }
                else if (!Config.NoRecoil && Config.OriginalRecoil)
                {
                    AimRotation = ControlRotation;
                }

                TargetDistance = GameData.Camera.Location.Distance(TargetPos) / 100.0f;

                if (TargetDistance > Config.AimDistance)
                {
                    StopAiming();
                    continue;
                }
                FVector PredictedPos;
                if (isMelee) {
                    float meleeLeadTime = 0.1f;
                    if (TargetDistance > 10.0f) meleeLeadTime += (TargetDistance - 10.0f) * 0.01f;
                    if (meleeLeadTime > 0.35f) meleeLeadTime = 0.35f;
                    PredictedPos = Config.Prediction ? (TargetPos + TargetVelocity * meleeLeadTime) : TargetPos;
                    PredictedPos.Z += TargetVelocity.Z * 0.1f;
                }
                else {
                    if (GameData.LocalPlayerInfo.CurrentWeaponIndex != 255 && GameData.LocalPlayerInfo.CurrentWeaponIndex != 4)
                    {
                        float Aimxx = 0.f;
                        float Aimyy = 0.f;
                        auto ZeroingDistance = 100.0f;
                        float BulletDrop = 0;
                        float TravelTime = 0;
                        float BallisticDragScale = 1;//1
                        float BallisticDropScale = 1;//1
                        float TimeToReach = TargetDistance / CurrentWeaponData.TrajectoryConfigs.InitialSpeed;

                        auto Result = GetBulletDropAndTravelTime(
                            AimLocation,
                            AimRotation,
                            TargetPos,
                            ZeroingDistance,
                            BulletDropAdd,
                            CurrentWeaponData.TrajectoryConfigs.InitialSpeed,
                            CurrentWeaponData.TrajectoryGravityZ,
                            BallisticDragScale, BallisticDropScale,
                            CurrentWeaponData.TrajectoryConfigs.BDS,
                            CurrentWeaponData.TrajectoryConfigs.SimulationSubstepTime,
                            CurrentWeaponData.TrajectoryConfigs.VDragCoefficient,
                            CurrentWeaponData.FloatCurves,
                            CurrentWeaponData.RichCurveKeys.size(),
                            CurrentWeaponData.RichCurveKeys
                        );

                        float  Drop = GetPredicted(AimLocation, TargetPos, TargetVelocity, CurrentWeaponData.TrajectoryConfigs);
                        BulletDrop = Result.first;
                        TravelTime = Result.second;
                        PredictedPos = FVector(TargetPos.X, TargetPos.Y, TargetPos.Z + Abs(Drop)) + TargetVelocity * (TravelTime / 0.885f);
                    }
                    else
                    {
                        PredictedPos = TargetPos;
                    }
                    if (!Config.Prediction)
                    {
                        PredictedPos = TargetPos;
                    }
                }

                if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::Grenade)//手雷预测
                {
                    // 这里是雷的高度计算需要时间 还有高度
                    if (TargetDistance > 16.0f && TargetDistance < 100.0f)
                    {
                        float ProjectHinght = 0;//  这是高度
                        float Project = 0;//  这是时间
                        GrenadeHwind(TargetDistance, ProjectHinght, Project);
                        PredictedPos = { TargetPos.X, TargetPos.Y, TargetPos.Z + ProjectHinght };
                        // 最大恰雷时间  5.1
                        if (GameData.LocalPlayerInfo.ElapsedCookingTime >= Project - 0.1f)
                        {
                            simulateClick();
                        }

                    }
                }


                if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::PanzerFaust100M1)//迫击炮预测
                {
                    // 火箭筒专用自瞄逻辑
                    float ProjectHeight = 0.0f;
                    float ProjectTime = 0.0f;

                    // 获取目标距离（单位：米）
                    float TargetDistance = GameData.Camera.Location.Distance(TargetPos) / 100.0f;

                    // 计算火箭筒高低抬动参数
                    RocketHwind(TargetDistance, ProjectHeight, ProjectTime);

                    // 调整预测位置（增加高度补偿下坠）
                    PredictedPos = FVector(TargetPos.X, TargetPos.Y, TargetPos.Z + ProjectHeight);




                }


                float CameraFOV = GameData.Camera.FOV;
                float DefaultFOV = 0.0f;
                //新添加的
                float MouseXSensitivity = 0.02f;
                float MouseYSensitivity = 0.02f;
                float AimSpeedMaxFactor = Config.AimSpeedMaxFactor;


                if (IsNearlyZero(CameraFOV))
                    continue;
                if (IsNearlyZero(DefaultFOV))
                    DefaultFOV = 90.0f;

                const float FOVRatio = DefaultFOV / CameraFOV;
                auto GetMouseXY = [&](FRotator RotationInput) {
                    RotationInput.Clamp();
                    return FVector2D{
                        float(RotationInput.Yaw / MouseXSensitivity * 0.4f * FOVRatio),
                        float(-RotationInput.Pitch / MouseYSensitivity * 0.4f * FOVRatio) };
                    };




                FRotator RotationInput = (PredictedPos - GameData.Camera.Location).GetDirectionRotator() - AimRotation;
                RotationInput.Clamp();

                FVector2D MoveXY = GetMouseXY(RotationInput * AimSpeedMaxFactor);


                GameData.AimBot.PredictedPos = PredictedPos;



                if (CanAim)
                {


                    if (Config.AutomaticShooting && GameData.AimBot.Type == EntityType::Player)//自动扳机仅对玩家目标
                    {
                        // AutomaticShootingFOV 范围 AutomaticShootingTime 开启时间
                        if (abs(MoveXY.X) < Config.AutomaticShootingFOV && abs(MoveXY.Y) < Config.AutomaticShootingFOV)
                        {
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::AR ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SMG ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::DMR ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::LMG ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::HG)
                            {
                                // 只有在未开镜状态下才自动开枪
                                if ((GetTickCount64() - AutomaticShootingTime) >= Config.AutomaticShootingTime)
                                {
                                    simulateClick();// 鼠标按住
                                    AutomaticShootingTime = 0;
                                }
                            }
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SR)
                            {
                                if ((GetTickCount64() - AutomaticShootingTime) >= Config.AutomaticShootingTime)// 大于多少毫秒
                                {
                                    simulateClick();  // 鼠标点击
                                    AutomaticShootingTime = 0;
                                }
                            }
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SG)
                            {
                                if ((GetTickCount64() - AutomaticShootingTime) >= Config.AutomaticShootingTime)// 大于多少毫秒
                                {
                                    simulateClick();// 鼠标点击
                                    AutomaticShootingTime = 0;
                                }
                            }
                        }
                    }

                    if (Config.AimAndShot) {

                        if (TargetDistance < Config.banjiAimDistance)
                        {
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::AR ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SMG ||

                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::LMG ||
                                GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::HG)
                            {
                                AimBotAPI_SG(MoveXY, Config);
                            }
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::DMR
                                || GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SR)
                            {
                                AimBotAPI_SG(MoveXY, Config);
                            }
                            if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::SG)
                            {
                                AimBotAPI_SG(MoveXY, Config);
                            }


                        }

                    }
                    AimBotAPI(MoveXY, Config);

                }
            }
            else {
                FistAim = false;
                StopAiming();
                //if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType == WeaponType::AR && Config.AutomaticShooting)PopUpTheLeft();// 松开左键
            }


        }
        mem.CloseScatterHandle(hScatter);
        mem.CloseScatterHandle(hWriteScatter);
    }
};
