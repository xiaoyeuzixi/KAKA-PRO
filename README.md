# KAKA-PRO · PUBG DMA 辅助（kaka 二改）

> ## ⚠️ 版权与版本标注（重要）
>
> - **原始项目 / 原版作者：`kaka`** —— 本项目的整体框架、偏移体系、DMA 读取方案、ImGui 覆盖层均来自 **kaka 原版**。
> - **本仓库是 `kaka 二改`（二次修改版）**，在原版基础上做了功能增补、界面改造与性能优化。
> - 本仓库**不是原版仓库**，也**不代表原作者立场**；转载、分发请保留 `kaka` 原作者署名与本二改标注。
> - 仅供逆向工程学习、DMA 硬件调试、图形渲染与 UE4 内存结构研究使用。
>
> **一句话：KAKA-PRO —— kaka 原版，kaka 二改。**

| 项目 | 信息 |
| --- | --- |
| 程序名 / 目标名 | `KAKA-PRO.exe` |
| 旧工程名 | `Jax Kane` / `Media.vcxproj` |
| 版本文件 | `version.txt` → `1.2` |
| 偏移版本 | `2605.x.x.x`（见 `config/offset_cache.txt`） |
| 平台 / 工具集 | Windows x64 / MSVC `v143`（Visual Studio 2022） |
| 界面技术 | ImGui（DX11 后端）+ freetype + RmlUi 后端样本 |
| 内存读取 | DMA 硬件（FTDI FT601 / `FTD3XX.dll`）+ MemProcFS（`vmm.dll` / `leechcore.dll`） |
| 输入设备 | KMBox / MoBox / KmBoxNet 硬件级鼠标（免驱动注入） |

---

## 💬 QQ 交流群

> **QQ 群号：`835775657`**
>
> 使用交流 · 偏移更新 · 二改讨论 · 问题反馈，欢迎加群。

---

## 一、项目简介

`KAKA-PRO` 是一个基于 **DMA 硬件读取 + 外部覆盖层（Overlay）** 的 PUBG（PUBG: BATTLEGROUNDS）辅助程序。

与传统的进程注入式外挂不同，本项目：

1. **不向游戏进程写入任何代码**：通过 DMA 硬件卡（FTDI FT601 芯片）从物理内存侧读取游戏内存；
2. **不依赖游戏进程句柄**：使用 MemProcFS（`vmm.dll` + `leechcore.dll`）在用户态完成虚拟地址翻译；
3. **鼠标操作由硬件盒子完成**：支持 KMBox / MoBox / KmBoxNet 硬件鼠标，移动指令不经过游戏进程；
4. **渲染完全独立**：使用 ImGui + DirectX 11 绘制透明覆盖层窗口，游戏画面仅作为背景。

数据流大致为：

```
物理内存 ──DMA卡(FTD3XX)──> MemProcFS(vmm/leechcore) ──> Memory 抽象层
        ──> Offset/Decrypt 解密 UE4 对象 ──> Hack 逻辑层(ESP/Aimbot/Radar)
        ──> ImGui Overlay(DX11) ──> 屏幕绘制
        ──> 硬件鼠标(KMBox/MoBox/KmBoxNet) ──> 实际输入
```

---

## 二、二改说明（kaka 二改）

本仓库基于 **kaka 原版** 二次修改，与原始版本的差异主要在：

| 分类 | 二改内容 |
| --- | --- |
| 界面 UI | 重构菜单主题系统（`Source/Overlay/MenuTheme.h`）、新增 Pluto 风格面板（`MenuPlutoEsp.inl`）、侧边栏与动画切换 |
| 偏移体系 | 适配 `2605.x.x.x` 版本偏移，新增偏移缓存文件 `config/offset_cache.txt`，解密函数 `Decrypt::CIndex` 内联生成 |
| 透视 ESP | 物品透视分组重做（子弹 / 投掷物 / 钥匙 / 其他），新增图标+文字混合显示、射线显示、距离阈值 |
| 自瞄 Aimbot | 分武器类别（AR / DMR / HG / LMG / MELEE / SG / SMG / SR）独立参数，支持随机部位、随机速度、动态 FOV、预判、无后座 |
| 雷达 | 大地图 / 小地图雷达、密室标注、空投与载具显示、网页共享雷达（WebRadar） |
| 载具 / 投掷物 | 载具血量油量、手雷/火箭弹道预测与爆炸范围、迫击炮落点 |
| 稳定性 | 修复资源释放崩溃，优化线程渲染（`MThreadRenderer.h`）、渲染缓存（`RenderCache.h`）、骨骼平滑（`BoneSmoother.h`） |
| 防护 | 集成 SProtect / SPCloud 云端卡密验证（`Source/SProtect`）与登录校验流程 |

> **二改标注要求**：任何基于本仓库的再分发，必须在 README 与程序关于页保留 **`kaka`（原版）+ `kaka 二改`（二次修改）** 两处署名。

---

## 三、功能特性

### 1. 主要透视（ESP）
- 方框 / 骨骼 / 血量条 / 名字 / 距离 / 队伍 / 武器 / 段位图标
- 玩家类型过滤：全部、玩家、合作者、队友、本人、人机、菜逼、挂逼、老挂逼
- 血量条样式：彩虹血条 / 单色血条 / 单色缩放 / 彩虹缩放，支持顶部或左侧显示
- 倒地（Groggy）状态区分、击倒图标、背包容纳信息

### 2. 自瞄 / 无后座（Aimbot）
- 按武器类别分别配置：`AR` `DMR` `HG` `LMG` `MELEE` `SG` `SMG` `SR` 以及 `Other` / `PanzerFaust100M1`
- 每类武器可见：自瞄开关、热键、瞄准距离、瞄准速度因子、骨骼选择（头/胸/骨盆等 17 个骨骼点）
- 高级选项：动态 FOV、FOV 圈显示、随机部位（部位数量 / 随机间隔 / 随机速度）、预判（Prediction）、无子弹不自瞄、可见性检查（VisibleCheck）、LineTraceSingle
- 自动开火（AutomaticShooting）：触发键、开火时间、FOV 限制、延迟
- 滚轮自瞄（AimWheel）+ 滚轮速度 + 绑定骨骼
- 多套自瞄配置（ConfigIndex），一键切换
- 自动压枪（`Source/Hack/autoRecoil.h`）

### 3. 物品透视
- 按分组显示：`子弹 / 投掷物 / 钥匙 / 其他`
- 支持图标、图标+文字、纯文字、射线显示，分组过滤与距离阈值（ThresholdX / ThresholdY）
- 物品图标资源见 `Assets/image/All/*.png`

### 4. 雷达
- 大地图雷达（`Radar.Main`）：玩家 / 载具 / 空投 / 密室
- 小地图雷达（`Radar.Mini`）
- 网页共享雷达（`WebRadar`）：可自建 HTTP 服务，把雷达数据推送到网页
- 游戏内段位 / 战绩查询（`Source/Hack/Segment.h`，走外部 HTTP 接口）

### 5. 载具与投掷物
- 载具血条 / 油量条 / 距离 / 颜色
- 手雷预测线、爆炸范围与倒计时
- 迫击炮落点（`Source/Hack/Mortar.h`）、投掷/火箭自动锁定

### 6. 其他
- 玩家列表（12 列数据，含昵称、段位、KD、距离、武器、背包等）
- 多语言切换（中文 / English）
- 菜单热键：**`Home`(0x24)** 显示菜单、**`End`(0x23)** 安全退出、**`Insert`(0x2D)** 融合模式
- 自动更新：读取 `version.txt`，下载 `update_package.bin` 并通过 `update_program.bat` 热替换

---

## 四、目录结构

只保留**参与编译的文件**与**发布目录 `x64/Release` 的运行文件**，编译中间产物已通过 `.gitignore` 排除。

```
KAKA-PRO/
├─ Main.cpp                      # 程序入口：DMA 初始化、更新器、主循环、窗口
├─ resource.h / Jax Kane.rc      # 资源脚本（图标）
├─ version.txt                   # 版本号，程序自动更新用
├─ Jax Kane.sln                  # VS2022 解决方案（工程名 Jax Kane，目标 KAKA-PRO）
├─ Media.vcxproj                 # 工程文件（v143 / x64 / Release）
├─ Media.vcxproj.filters         # VS 文件过滤器
├─ Media.vcxproj.user            # VS 用户设置
├─ app.rc/                       # 图标资源：logo.ico、PaoX.png
│
├─ Source/                       # ★ 项目源码（二改核心）
│  ├─ Common/                    # 数据模型、配置、偏移、骨骼、可见性
│  │   ├─ Data.h                 # FGameData 全局数据结构
│  │   ├─ Config.h               # 配置项：读写 kakaConfig.bak
│  │   ├─ Offset.h               # UE4 偏移表
│  │   ├─ Entitys.cpp/.h         # 物品/武器名称与分组表
│  │   ├─ Bone.h / BoneSmoother.h
│  │   └─ VisibleScene.h         # PhysX 可见场景判断
│  ├─ Hack/                      # ★ 逻辑层
│  │   ├─ Hack.h                 # 主线程：遍历 Actor、更新数据
│  │   ├─ AimBot.h               # 自瞄实现（分武器类别）
│  │   ├─ autoRecoil.h           # 自动压枪
│  │   ├─ Players.h / PlayerLists.h / Actors.h / Items.h
│  │   ├─ Vehicles.h / Radar.h / WebRadar.h / Webpageradar.h
│  │   ├─ Segment.h              # 战绩/段位查询
│  │   ├─ Physx.h / MeshPatcher.h / VisibleCheck.h / LineTrace.h
│  │   ├─ Decrypt.cpp/.h         # 解密与偏移解析
│  │   ├─ GNames.h / Process.cpp/.h / KeyState.h
│  │   ├─ WebSocketClient.h      # 网页雷达通信
│  │   └─ Projects.h / Mortar.h
│  ├─ Overlay/                   # ★ 渲染层
│  │   ├─ Overlay.cpp            # DX11 + ImGui 覆盖层、登录窗口、主循环
│  │   ├─ Menu.h                 # 主菜单（体量最大的界面文件）
│  │   ├─ MenuTheme.h            # 菜单主题/尺寸系统
│  │   ├─ MenuPlutoEsp.inl       # Pluto 风格面板
│  │   ├─ ESP.h                  # 绘制方框/骨骼/血条/距离
│  │   ├─ weapon.h / image.h 相关资源
│  │   ├─ HealthBar.h / RenderCache.h / RenderHelper.h / Style.h
│  │   ├─ Texture.cpp/.h         # 贴图加载
│  │   └─ MThreadRenderer.h
│  ├─ Utils/                     # 工具层
│  │   ├─ Utils.h / string.cpp / Throttler.cpp / Timer.cpp
│  │   ├─ KmBox* / MoBox* / Lurker* / NetConfig/kmboxNet.cpp
│  │   ├─ ue4math/               # FVector / FRotator / FMatrix / FQuat
│  │   ├─ w3c/                   # 时间与编码工具
│  │   ├─ COM.h / Engine.h / Sha.h / QPVerify.h / MachineCodeGenerator.h
│  │   └─ image.h / stb_image.h / weapon.h（图标数据）
│  ├─ cJSON/                     # JSON 解析（配置读写）
│  ├─ Gui/                       # RmlUi 后端与资源（备用 UI 方案）
│  └─ SProtect/                  # 云端卡密验证 SDK
│
├─ Include/                      # ★ 第三方头文件与编译单元
│  ├─ DMALibrary/                # MemProcFS 封装（Memory.cpp 等）
│  ├─ imgui/                     # ImGui 全套源码 + freetype/knobs 扩展
│  ├─ embree4/                   # Embree4 光线追踪（视线检测加速）
│  ├─ Resources/                 # 内嵌资源头文件与 PNG
│  ├─ boost/ nlohmann/ openssl/ websocketpp/ Mosquitto/ TD/ Font/
│  └─ DMARender/                 # DMA 渲染相关
│
├─ lib/                          # ★ 链接库：embree4 / tbb12 / zstd / zlib / boost_random / QPLibX64_MT / SPCloud64_MT / SProtectSDK64
├─ freetype/                     # ★ FreeType 头文件、win64/win32 的 dll+lib
├─ ThirdParty/                   # ★ Microsoft.DXSDK.D3DX（d3dx9/10/11 头与库）、DXSDK
├─ Assets/                       # ★ 运行时资源：物品/武器/载具/地图/段位图标、主题
│
└─ x64/Release/                  # ★★ 编译产物（可直接运行的发布目录，已全部上传）
   ├─ KAKA-PRO.exe
   ├─ vmm.dll / leechcore.dll / FTD3XX.dll / ddll64.dll
   ├─ embree4.dll / tbb12.dll / zstd.dll / zlib1.dll / freetype.dll / boost_random-*.dll
   ├─ D3DCompiler_43.dll / d3dx9_43.dll / d3dx10_43.dll / d3dx11_43.dll
   ├─ VMProtectSDK64.dll
   ├─ KAKA-PRO.lib / KAKA-PRO.exp
   ├─ Key.ini / lic.ini / version.txt / imgui.ini
   ├─ config/      # 运行时配置（见第八节）
   └─ Assets/      # 运行时资源（与根目录 Assets 一致）
```

---

## 五、编译环境与步骤

### 环境要求

| 依赖 | 版本 |
| --- | --- |
| Visual Studio | 2022（MSVC v143） |
| Windows SDK | 10.0（`WindowsTargetPlatformVersion=10.0`） |
| 平台 | **x64**（Win32 配置存在但未维护） |
| 配置 | **Release**（Debug 需要 `MultiThreadedDebug`，体积巨大） |

### 步骤

1. 克隆仓库（**必须保留目录结构**，因为工程使用 `$(SolutionDir)` / `$(ProjectDir)` 相对路径）：

   ```powershell
   git clone https://github.com/xiaoyeuzixi/KAKA-PRO.git
   cd KAKA-PRO
   ```

2. 用 Visual Studio 2022 打开 `Jax Kane.sln`。

3. 选择配置：**Release | x64**。

4. 确认工程属性中的关键路径（工程内已配置好，无需手动改）：

   - 头文件目录：`$(SolutionDir)Include\embree4`、`$(ProjectDir)Include\Resources`、`$(SolutionDir)ThirdParty\Microsoft.DXSDK.D3DX\build\native\include`
   - 库目录：`$(SolutionDir)Include\DMALibrary\libs`、`$(SolutionDir)lib`、`$(SolutionDir)freetype\win64`、`$(SolutionDir)ThirdParty\Microsoft.DXSDK.D3DX\build\native\release\lib\x64`
   - 输出目录：`$(SolutionDir)x64\$(Configuration)\`

5. 生成：

   ```powershell
   msbuild "Jax Kane.sln" /p:Configuration=Release /p:Platform=x64 /m
   ```

6. 产物位于 `x64\Release\KAKA-PRO.exe`，可直接与 `x64\Release` 下已有 DLL、`config`、`Assets` 一起运行。

> **已知问题**：`Media.vcxproj` 中登记的头文件为 `Source\Utils\QPModX64_507276699.h`，
> 而仓库内实际存在的是 `Source\Utils\QPModX64_289700860.h`（版本号不同的同名生成文件，未被任何源码 `#include`）。
> 如需在 IDE 中彻底干净地打开工程，可把该工程项指向仓库内实际存在的那个文件。

---

## 六、运行说明

### 前置条件

1. **DMA 硬件卡**（FTDI FT601）—— 未插卡时程序无法读取游戏内存；
2. 安装 DMA 官方驱动（提供 `FTD3XX.dll`）；
3. `vmm.dll` / `leechcore.dll`（MemProcFS）需与 `KAKA-PRO.exe` 同目录，仓库已附带；
4. 如需硬件鼠标，连接 KMBox / MoBox 设备。

### 启动

1. 以管理员身份运行 `x64\Release\KAKA-PRO.exe`（建议与游戏同盘、同目录结构）；
2. 进入卡密登录界面，输入账号密码（配置保存在 `Key.ini` / `lic.ini`）；
3. 进入游戏后按 **`Home`** 呼出菜单，**`End`** 安全退出，**`Insert`** 切换融合模式；
4. 首次运行会自动生成 / 更新：
   - `config/offset_cache.txt` —— 偏移缓存（版本 `2605.x.x.x`）
   - `config/kakaConfig.bak` —— 全部菜单配置的 JSON 备份
   - `imgui.ini` —— 窗口布局

---

## 七、编译产物 `x64/Release` 文件清单

| 文件 | 作用 |
| --- | --- |
| `KAKA-PRO.exe` | 主程序（约 9.6 MB） |
| `vmm.dll` | MemProcFS 虚拟内存管理核心，负责物理内存 → 虚拟地址翻译 |
| `leechcore.dll` | MemProcFS 底层设备访问层（DMA / FPGA 通信） |
| `FTD3XX.dll` | FTDI FT601 USB3.0 驱动接口，DMA 卡通信 |
| `ddll64.dll` | DMA 卡厂商驱动配套库 |
| `embree4.dll` | Intel Embree 4 光线追踪库，用于视线遮挡（VisibleCheck）加速 |
| `tbb12.dll` | Intel TBB 线程库（Embree 依赖） |
| `zstd.dll` / `zlib1.dll` | 压缩库（内存/数据压缩，MemProcFS 依赖） |
| `boost_random-vc143-mt-x64-1_86.dll` | Boost.Random（KMBoxNet 随机化输入） |
| `freetype.dll` | 字体渲染（ImGui_freetype，中文与自定义字体） |
| `D3DCompiler_43.dll` / `d3dx9_43.dll` / `d3dx10_43.dll` / `d3dx11_43.dll` | DirectX 运行库（覆盖层渲染与纹理） |
| `VMProtectSDK64.dll` | VMProtect 保护 SDK 运行库 |
| `KAKA-PRO.lib` / `KAKA-PRO.exp` | 链接产物（导入库 / 导出表） |
| `version.txt` | 版本号 `1.2`，自动更新比对用 |
| `Key.ini` | 账号配置（`user` / `pass`） |
| `lic.ini` | 授权配置（`save` / `user` / `pass`） |
| `imgui.ini` | ImGui 窗口布局与尺寸 |
| `vmm.dll.debug.bak` / `leechcore.dll.bak` / `leechcore2.dll.debug.bak` | 依赖库备份文件（原样保留） |
| `config/` | 运行时配置目录，见下节 |
| `Assets/` | 运行时图标资源（物品、武器、载具、地图、段位、主题） |

> 发布目录中**不包含** `obj/`（编译中间文件）、`*.pdb`（调试符号）、`run*.txt` / `*.log`（运行日志），
> 这些已由 `.gitignore` 排除。

---

## 八、配置文件说明

`config/` 目录（根目录与 `x64/Release/config/` 各一份，内容一致）：

| 文件 | 说明 |
| --- | --- |
| `kakaConfig.bak` | **核心配置**：完整菜单配置的 JSON 备份。包含 `AimBot`（8 类武器 × 多套配置：骨骼、距离、速度、预判、无后座、随机部位）、`ESP`（透视项目与物品分组）、`Overlay`（热键）、`Radar`（大/小地图）、`Project`（投掷物预测）、`Vehicle`、`PlayerList`、`WebRadar`（网页雷达服务地址）等 |
| `offset_cache.txt` | **偏移缓存**（编译/运行期产物）：由 `Decrypt::CIndex` 与 `namespace Offset` 组成，记录当前版本（`2605.x.x.x`）的 `UWorld`、`GNames`、`XenuineDecrypt` 等偏移 |
| `ESP.txt` | 透视自定义列表（空文件 = 使用默认） |
| `WhiteLists.txt` | 白名单（空文件 = 不启用） |
| `BlackLists.txt` | 黑名单（空文件 = 不启用） |

其他配置：

| 文件 | 说明 |
| --- | --- |
| `version.txt` | 当前版本号，用于自动更新比对 |
| `Key.ini` | 登录账号保存位置 |
| `lic.ini` | 授权/卡密保存位置（`save=1` 表示记住） |
| `imgui.ini` | 覆盖层窗口布局（位置、尺寸、折叠状态） |

> **提示**：`kakaConfig.bak` 中保存了示例用户的 `WebRadar` 服务地址与 PIN 码。若是公开仓库，
> 建议先把 `WebRadar.Server IP` / `Server PIN` 替换为你自己的服务地址或占位符后再提交。

---

## 九、常见问题

**Q1：为什么编译报错找不到 `QPModX64_507276699.h`？**
见第五节"已知问题"，仓库内实际文件为 `QPModX64_289700860.h`，且没有源码 `#include` 它，删除或改名该工程项即可。

**Q2：为什么 Debug 配置体积巨大？**
Debug 使用 `MultiThreadedDebug`，`Overlay.cpp` 与 `Main.cpp` 体量大（`Overlay.obj` 可达 80 MB+），建议只编译 Release。

**Q3：为什么运行后读不到数据？**
检查 DMA 卡驱动（`FTD3XX.dll`）是否正确安装、`vmm.dll` 是否与 exe 同目录、游戏是否已进入大厅后对局。

**Q4：为什么偏移不对？**
游戏更新后需重新适配 `Source/Common/Offset.h` 与 `config/offset_cache.txt`，版本号在 `offset_cache.txt` 顶部标注。

---

## 十、免责声明与致谢

### 免责声明

本项目及其全部源码、二进制、配置**仅供逆向工程学习、DMA 硬件调试、图形渲染技术与 UE4 内存结构研究使用**。
使用者应自行确认所在地区的法律法规与目标软件的用户协议，任何因使用本项目产生的后果由使用者自行承担。

### 致谢

- **原版作者：`kaka`** —— 本项目的全部基础工作归功于 kaka。
- **本仓库：`kaka 二改`** —— 二次修改、界面重构与优化。
- **QQ 交流群：`835775657`** —— 二改版使用与更新交流。
- **第三方组件**：`Dear ImGui`、`MemProcFS (vmm/leechcore)`、`Intel Embree`、`Intel TBB`、`FreeType`、`cJSON`、`RmlUi`、`Boost`、`zlib/zstd`、`OpenSSL`、`websocketpp`、`nlohmann/json`、`Microsoft DirectX SDK`。
