#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
#include <winhttp.h>
#include "cJSON/cJSON.h"
#include <memory>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <Common/Data.h>
#include <Utils/Utils.h>
#include <functional>
#include <queue>
#include <condition_variable>

// 线程池类
class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
                });
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// 调试开关：设置为 true 时输出API返回的数据结构
static bool DEBUG_API_RESPONSE = false;

#pragma comment(lib, "winhttp.lib")

class HttpHandle {
public:
    explicit HttpHandle(HINTERNET handle) : handle_(handle) {}
    ~HttpHandle() { if (handle_) WinHttpCloseHandle(handle_); }
    HINTERNET get() const { return handle_; }
    bool isValid() const { return handle_ != nullptr; }
private:
    HINTERNET handle_;
};

class JsonHandle {
public:
    explicit JsonHandle(cJSON* json) : json_(json) {}
    ~JsonHandle() { if (json_) cJSON_Delete(json_); }
    cJSON* get() const { return json_; }
    bool isValid() const { return json_ != nullptr; }
private:
    cJSON* json_;
};

class Segment {
public:
    static std::unordered_map<std::string, std::string> ChineseToEnglishTier;

    static std::string SendHttpPostRequest(const std::string& url, const std::string& postData) {
        HttpHandle hSession(WinHttpOpen(L"WinHTTP Example/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0));

        if (!hSession.isValid()) {
            std::cerr << "Failed to open HTTP session." << std::endl;
            return "";
        }

        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256], urlPath[256];
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = _countof(hostName);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = _countof(urlPath);

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            std::cerr << "Failed to crack URL." << std::endl;
            return "";
        }

        HttpHandle hConnect(WinHttpConnect(hSession.get(), urlComp.lpszHostName, urlComp.nPort, 0));
        if (!hConnect.isValid()) {
            std::cerr << "Failed to connect to server." << std::endl;
            return "";
        }

        HttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), L"POST", urlComp.lpszUrlPath,
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT ? WINHTTP_FLAG_SECURE : 0));

        if (!hRequest.isValid()) {
            std::cerr << "Failed to open HTTP request." << std::endl;
            return "";
        }

        DWORD timeout = 8000;//超时时间8秒
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

        const wchar_t* headers = L"Content-Type: application/x-www-form-urlencoded";
        if (!WinHttpSendRequest(hRequest.get(), headers, -1L,
            (LPVOID)postData.c_str(), (DWORD)postData.size(),
            (DWORD)postData.size(), 0)) {
            std::cerr << "Error in WinHttpSendRequest: " << GetLastError() << std::endl;
            return "";
        }

        if (!WinHttpReceiveResponse(hRequest.get(), NULL)) {
            std::cerr << "Error in WinHttpReceiveResponse: " << GetLastError() << std::endl;
            return "";
        }

        std::string response;
        DWORD dwSize = 0;
        do {
            if (WinHttpQueryDataAvailable(hRequest.get(), &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest.get(), buffer.data(), dwSize, &dwDownloaded)) {
                    response.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);

        return response;
    }

    static std::string GetResponse(const std::string& userName) {

        std::string url = "http://pubg.000881.xyz/getRankByName?name=" + userName;
        return SendHttpGetRequest(url);
    }

    static std::string GetMatchResponse(const std::string& userName) {
        std::string url = "http://pubg.000881.xyz/getMatch?name=" + userName;
        return SendHttpGetRequest(url);
    }

    // 新增 GET 请求方法
    static std::string SendHttpGetRequest(const std::string& url) {
        HttpHandle hSession(WinHttpOpen(L"WinHTTP Example/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0));

        if (!hSession.isValid()) {
            std::cerr << "Failed to open HTTP session." << std::endl;
            return "";
        }

        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256], urlPath[256];
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = _countof(hostName);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = _countof(urlPath);

        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
            std::cerr << "Failed to crack URL." << std::endl;
            return "";
        }

        HttpHandle hConnect(WinHttpConnect(hSession.get(), urlComp.lpszHostName, urlComp.nPort, 0));
        if (!hConnect.isValid()) {
            std::cerr << "Failed to connect to server." << std::endl;
            return "";
        }

        HttpHandle hRequest(WinHttpOpenRequest(hConnect.get(), L"GET", urlComp.lpszUrlPath,
            NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            urlComp.nPort == INTERNET_DEFAULT_HTTPS_PORT ? WINHTTP_FLAG_SECURE : 0));

        if (!hRequest.isValid()) {
            std::cerr << "Failed to open HTTP request." << std::endl;
            return "";
        }

        DWORD timeout = 8000;//超时时间8秒
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        WinHttpSetOption(hRequest.get(), WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

        const wchar_t* headers = L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
        if (!WinHttpSendRequest(hRequest.get(), headers, -1L, NULL, 0, 0, 0)) {
            std::cerr << "Error in WinHttpSendRequest: " << GetLastError() << std::endl;
            return "";
        }

        if (!WinHttpReceiveResponse(hRequest.get(), NULL)) {
            std::cerr << "Error in WinHttpReceiveResponse: " << GetLastError() << std::endl;
            return "";
        }

        std::string response;
        DWORD dwSize = 0;
        do {
            if (WinHttpQueryDataAvailable(hRequest.get(), &dwSize) && dwSize > 0) {
                std::vector<char> buffer(dwSize + 1, 0);
                DWORD dwDownloaded = 0;
                if (WinHttpReadData(hRequest.get(), buffer.data(), dwSize, &dwDownloaded)) {
                    response.append(buffer.data(), dwDownloaded);
                }
            }
        } while (dwSize > 0);

        return response;
    }

    static std::string DoubleToString(double value) {
        value *= 100;  // 转换为百分比
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;  // 保留两位小数
        return stream.str() + "%";  // 加上百分号
    }

    static std::string DoubleToString2(double value) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << value;
        return stream.str();
    }

    static void Update() {
        InitializeChineseToEnglishMapping();

        std::unordered_map<std::string, int> requestCountMap;
        std::mutex requestCountMutex;
        std::mutex dataMutex;

        while (true) {
            if (GameData.Scene != Scene::Gaming) {
                requestCountMap.clear();
                if (GameData.Scene == Scene::FindProcess) {
                    Data::SetPlayerRankLists({});
                    Data::SetPlayerSegmentLists({});
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(GameData.ThreadSleep));
                continue;
            }

            ThreadPool pool(20);
            auto retrievedPlayerRankLists = Data::GetPlayerRankLists();

            for (const auto& pair : retrievedPlayerRankLists) {
                const auto& detail = pair.second;
                if (detail.Tem >= 0 && detail.Tem < 100) {
                    pool.enqueue([playerName = detail.PlayerName, &requestCountMap, &requestCountMutex, &dataMutex] {
                        ProcessPlayer(playerName, requestCountMap, requestCountMutex, dataMutex);
                        });
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    }

private:
    static void ProcessPlayer(const std::string& playerName, std::unordered_map<std::string, int>& requestCountMap, std::mutex& requestCountMutex, std::mutex& dataMutex) {
        PlayerRankList temp;
        {
            auto playerSegmentLists = Data::GetPlayerSegmentLists();
            auto it = playerSegmentLists.find(playerName);
            if (it != playerSegmentLists.end()) {
                temp = it->second;
            }
        }

        bool rankedUpdated = temp.TPP.Updated && temp.FPP.Updated && temp.SquadTPP.Updated && temp.SquadFPP.Updated;
        bool normalUpdated = temp.NormalSquadTPP.Updated && temp.NormalSoloTPP.Updated &&
            temp.NormalDuoTPP.Updated && temp.NormalSquadFPP.Updated &&
            temp.NormalSoloFPP.Updated && temp.NormalDuoFPP.Updated;

        if (rankedUpdated && normalUpdated) {
            return;
        }

        int currentRequestCount = 0;
        {
            std::lock_guard<std::mutex> lock(requestCountMutex);
            currentRequestCount = requestCountMap[playerName];
        }

        if (currentRequestCount >= 5) {
            SetDefaultUnranked(temp);
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                Data::SetPlayerSegmentListsItem(playerName, temp);
            }
            return;
        }

        if (currentRequestCount > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200 * currentRequestCount));
        }

        {
            std::lock_guard<std::mutex> lock(requestCountMutex);
            requestCountMap[playerName]++;
        }

        std::string response = GetResponse(playerName);
        if (!response.empty()) {
            JsonHandle root(cJSON_Parse(response.c_str()));
            if (root.isValid()) {
                cJSON* data = cJSON_GetObjectItem(root.get(), "data");
                if (data) {
                    cJSON* attributes = cJSON_GetObjectItem(data, "attributes");
                    if (attributes) {
                        cJSON* rankedGameModeStats = cJSON_GetObjectItem(attributes, "rankedGameModeStats");
                        if (rankedGameModeStats) {
                            ParseGameModeStats(temp, rankedGameModeStats, "squad", 0);
                            ParseGameModeStats(temp, rankedGameModeStats, "solo", 1);
                            ParseGameModeStats(temp, rankedGameModeStats, "squad-fpp", 2);
                            ParseGameModeStats(temp, rankedGameModeStats, "solo-fpp", 3);
                        }
                        cJSON* normalGameModeStats = cJSON_GetObjectItem(attributes, "normalGameModeStats");
                        if (!normalGameModeStats) normalGameModeStats = cJSON_GetObjectItem(attributes, "normalStats");
                        if (!normalGameModeStats) normalGameModeStats = cJSON_GetObjectItem(attributes, "matchStats");
                        if (!normalGameModeStats) normalGameModeStats = cJSON_GetObjectItem(attributes, "casualStats");
                        if (!normalGameModeStats) normalGameModeStats = cJSON_GetObjectItem(attributes, "normalModeStats");
                        if (normalGameModeStats) {
                            ParseNormalGameModeStats(temp, normalGameModeStats, "squad", 0);
                            ParseNormalGameModeStats(temp, normalGameModeStats, "solo", 1);
                            ParseNormalGameModeStats(temp, normalGameModeStats, "squad-fpp", 2);
                            ParseNormalGameModeStats(temp, normalGameModeStats, "solo-fpp", 3);
                        }
                    }
                }
            }
        }

        if (!temp.NormalSquadTPP.Updated || !temp.NormalSoloTPP.Updated || !temp.NormalDuoTPP.Updated ||
            !temp.NormalSquadFPP.Updated || !temp.NormalSoloFPP.Updated || !temp.NormalDuoFPP.Updated) {
            std::string matchResponse = GetMatchResponse(playerName);
            if (!matchResponse.empty()) {
                JsonHandle matchRoot(cJSON_Parse(matchResponse.c_str()));
                if (matchRoot.isValid()) {
                    cJSON* matchData = cJSON_GetObjectItem(matchRoot.get(), "data");
                    if (matchData) {
                        cJSON* matchAttributes = cJSON_GetObjectItem(matchData, "attributes");
                        if (matchAttributes) {
                            cJSON* gameModeStats = cJSON_GetObjectItem(matchAttributes, "gameModeStats");
                            if (gameModeStats) {
                                ParseMatchModeStats(temp, gameModeStats, "squad", 0);
                                ParseMatchModeStats(temp, gameModeStats, "solo", 1);
                                ParseMatchModeStats(temp, gameModeStats, "duo", 2);
                                ParseMatchModeStats(temp, gameModeStats, "squad-fpp", 3);
                                ParseMatchModeStats(temp, gameModeStats, "solo-fpp", 4);
                                ParseMatchModeStats(temp, gameModeStats, "duo-fpp", 5);
                            }
                        }
                    }
                }
            }
        }

        SetDefaultUnranked(temp);

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            Data::SetPlayerSegmentListsItem(playerName, temp);
        }
    }
    // 初始化中文到英文段位映射
    static void InitializeChineseToEnglishMapping() {
        if (ChineseToEnglishTier.empty()) {
            ChineseToEnglishTier[U8("青铜")] = "Bronze";
            ChineseToEnglishTier[U8("白银")] = "Silver";
            ChineseToEnglishTier[U8("黄金")] = "Gold";
            ChineseToEnglishTier[U8("白金")] = "Platinum";
            ChineseToEnglishTier[U8("水晶")] = "Crystal";
            ChineseToEnglishTier[U8("钻石")] = "Diamond";
            ChineseToEnglishTier[U8("大师")] = "Master";
            ChineseToEnglishTier[U8("生存者")] = "Survivor";//生存者
            ChineseToEnglishTier[U8("未定级")] = "Unranked";//未定级
        }
    }


    // 设置默认未定级状态
    static void SetDefaultUnranked(PlayerRankList& temp) {
        if (!temp.TPP.Updated) {
            temp.TPP.Tier = "";
            temp.TPP.SubTier = "";
            temp.TPP.TierToString = U8("没玩此模式");
            temp.TPP.Updated = true;
        }
        if (!temp.FPP.Updated) {
            temp.FPP.Tier = "";
            temp.FPP.SubTier = "";
            temp.FPP.TierToString = U8("没玩此模式");
            temp.FPP.Updated = true;
        }
        if (!temp.SquadTPP.Updated) {
            temp.SquadTPP.Tier = "";
            temp.SquadTPP.SubTier = "";
            temp.SquadTPP.TierToString = U8("没玩此模式");
            temp.SquadTPP.Updated = true;
        }
        if (!temp.SquadFPP.Updated) {
            temp.SquadFPP.Tier = "";
            temp.SquadFPP.SubTier = "";
            temp.SquadFPP.TierToString = U8("没玩此模式");
            temp.SquadFPP.Updated = true;
        }
        // 设置匹配模式的默认状态
        if (!temp.NormalSquadTPP.Updated) {
            temp.NormalSquadTPP.Tier = "";
            temp.NormalSquadTPP.SubTier = "";
            temp.NormalSquadTPP.TierToString = U8("没玩此模式");
            temp.NormalSquadTPP.Updated = true;
        }
        if (!temp.NormalSoloTPP.Updated) {
            temp.NormalSoloTPP.Tier = "";
            temp.NormalSoloTPP.SubTier = "";
            temp.NormalSoloTPP.TierToString = U8("没玩此模式");
            temp.NormalSoloTPP.Updated = true;
        }
        if (!temp.NormalDuoTPP.Updated) {
            temp.NormalDuoTPP.Tier = "";
            temp.NormalDuoTPP.SubTier = "";
            temp.NormalDuoTPP.TierToString = U8("没玩此模式");
            temp.NormalDuoTPP.Updated = true;
        }
        if (!temp.NormalSquadFPP.Updated) {
            temp.NormalSquadFPP.Tier = "";
            temp.NormalSquadFPP.SubTier = "";
            temp.NormalSquadFPP.TierToString = U8("没玩此模式");
            temp.NormalSquadFPP.Updated = true;
        }
        if (!temp.NormalSoloFPP.Updated) {
            temp.NormalSoloFPP.Tier = "";
            temp.NormalSoloFPP.SubTier = "";
            temp.NormalSoloFPP.TierToString = U8("没玩此模式");
            temp.NormalSoloFPP.Updated = true;
        }
        if (!temp.NormalDuoFPP.Updated) {
            temp.NormalDuoFPP.Tier = "";
            temp.NormalDuoFPP.SubTier = "";
            temp.NormalDuoFPP.TierToString = U8("没玩此模式");
            temp.NormalDuoFPP.Updated = true;
        }
    }

    static void SetRankedDefaultOnly(PlayerRankList& temp) {
        if (!temp.TPP.Updated) {
            temp.TPP.Tier = "";
            temp.TPP.SubTier = "";
            temp.TPP.TierToString = U8("没玩此模式");
            temp.TPP.Updated = true;
        }
        if (!temp.FPP.Updated) {
            temp.FPP.Tier = "";
            temp.FPP.SubTier = "";
            temp.FPP.TierToString = U8("没玩此模式");
            temp.FPP.Updated = true;
        }
        if (!temp.SquadTPP.Updated) {
            temp.SquadTPP.Tier = "";
            temp.SquadTPP.SubTier = "";
            temp.SquadTPP.TierToString = U8("没玩此模式");
            temp.SquadTPP.Updated = true;
        }
        if (!temp.SquadFPP.Updated) {
            temp.SquadFPP.Tier = "";
            temp.SquadFPP.SubTier = "";
            temp.SquadFPP.TierToString = U8("没玩此模式");
            temp.SquadFPP.Updated = true;
        }
        // 不触碰匹配模式字段，后续仍可从匹配接口填充KD
    }

    static void ParseGameModeStats(PlayerRankList& temp, cJSON* stats, const char* mode, int Issquad) {
        cJSON* modeData = cJSON_GetObjectItem(stats, mode);
        if (!modeData) return;

        PlayerRankInfo& rank = Issquad == 0 ? temp.SquadTPP : Issquad == 1 ? temp.TPP : Issquad == 2 ? temp.SquadFPP : temp.FPP;
        if (rank.Updated) return;

        cJSON* currentTier = cJSON_GetObjectItem(modeData, "currentTier");
        if (currentTier) {
            cJSON* tier = cJSON_GetObjectItem(currentTier, "tier");
            cJSON* subTier = cJSON_GetObjectItem(currentTier, "subTier");
            if (cJSON_IsString(tier) && cJSON_IsString(subTier)) {
                std::string chineseTier = tier->valuestring;// "黄金"
                std::string subTierStr = subTier->valuestring;// "3"

                if (ChineseToEnglishTier.count(chineseTier)) {
                    rank.Tier = ChineseToEnglishTier[chineseTier];
                }
                else {
                    rank.Tier = "Unranked";
                }

                rank.SubTier = subTierStr;
                rank.TierToString = chineseTier + subTierStr;
            }
        }
        else {
            rank.Tier = "";
            rank.SubTier = "";
            rank.TierToString = U8("\u6ca1\u73a9\u8fc7");
        }

        cJSON* KDA = cJSON_GetObjectItem(modeData, "kd");
        if (!KDA) KDA = cJSON_GetObjectItem(modeData, "kda");
        if (KDA && cJSON_IsNumber(KDA)) {
            rank.KDA = KDA->valuedouble;
            rank.KDAToString = DoubleToString2(KDA->valuedouble);
        }

        cJSON* winRatio = cJSON_GetObjectItem(modeData, "winRatio");
        if (winRatio && cJSON_IsNumber(winRatio)) {
            rank.WinRatio = static_cast<float>(winRatio->valuedouble);
            rank.WinRatioToString = DoubleToString(winRatio->valuedouble);
        }

        cJSON* score = cJSON_GetObjectItem(modeData, "currentRankPoint");
        if (score && cJSON_IsNumber(score)) {
            rank.RankPoint = score->valuedouble;
        }

        // 标记为已更新
        rank.Updated = true;
    }

    // 解析匹配模式统计数据（从 getMatch API）
    static void ParseMatchModeStats(PlayerRankList& temp, cJSON* stats, const char* mode, int modeIndex) {
        cJSON* modeData = cJSON_GetObjectItem(stats, mode);
        if (!modeData) return;

        PlayerRankInfo* rank = nullptr;
        // 根据模式索引选择对应的数据结构
        switch (modeIndex) {
        case 0: // squad (TPP四人)
            rank = &temp.NormalSquadTPP;
            break;
        case 1: // solo (TPP单人)
            rank = &temp.NormalSoloTPP;
            break;
        case 2: // duo (TPP双人)
            rank = &temp.NormalDuoTPP;
            break;
        case 3: // squad-fpp (FPP四人)
            rank = &temp.NormalSquadFPP;
            break;
        case 4: // solo-fpp (FPP单人)
            rank = &temp.NormalSoloFPP;
            break;
        case 5: // duo-fpp (FPP双人)
            rank = &temp.NormalDuoFPP;
            break;
        }

        if (!rank || rank->Updated) return;

        //  kills  roundsPlayed  KD
        cJSON* kills = cJSON_GetObjectItem(modeData, "kills");
        cJSON* roundsPlayed = cJSON_GetObjectItem(modeData, "roundsPlayed");
        cJSON* wins = cJSON_GetObjectItem(modeData, "wins");

        bool hasData = false;
        if (kills && cJSON_IsNumber(kills) && roundsPlayed && cJSON_IsNumber(roundsPlayed)) {
            double killCount = kills->valuedouble;
            double roundCount = roundsPlayed->valuedouble;

            if (roundCount > 0) {
                rank->KDA = static_cast<float>(killCount / roundCount);
                rank->KDAToString = DoubleToString2(rank->KDA);
                hasData = true;
            }
        }

        // 计算吃鸡率
        if (wins && cJSON_IsNumber(wins) && roundsPlayed && cJSON_IsNumber(roundsPlayed)) {
            double winCount = wins->valuedouble;
            double roundCount = roundsPlayed->valuedouble;

            if (roundCount > 0) {
                rank->WinRatio = static_cast<float>(winCount / roundCount);   // 修复：不乘100，DoubleToString会自动乘100
                rank->WinRatioToString = DoubleToString(rank->WinRatio);
            }
        }

        // 匹配模式显示为"匹配模式"
        rank->Tier = "";
        rank->SubTier = "";
        rank->TierToString = hasData ? U8("匹配模式") : U8("没玩此模式");

        // 标记为已更新
        rank->Updated = true;
    }

    // 解析匹配模式统计数据
    static void ParseNormalGameModeStats(PlayerRankList& temp, cJSON* stats, const char* mode, int Issquad) {
        cJSON* modeData = cJSON_GetObjectItem(stats, mode);
        if (!modeData) return;

        PlayerRankInfo& rank = Issquad == 0 ? temp.NormalSquadTPP : Issquad == 1 ? temp.NormalSoloTPP : Issquad == 2 ? temp.NormalSquadFPP : temp.NormalSoloFPP;
        if (rank.Updated) return;

        // 匹配模式没有段位信息，直接解析KD和其他数据
        cJSON* KDA = cJSON_GetObjectItem(modeData, "kd");
        if (!KDA) KDA = cJSON_GetObjectItem(modeData, "kda");
        bool hasData = false;
        if (KDA && cJSON_IsNumber(KDA)) {
            rank.KDA = KDA->valuedouble;
            rank.KDAToString = DoubleToString2(KDA->valuedouble);
            hasData = true;
        }

        cJSON* winRatio = cJSON_GetObjectItem(modeData, "winRatio");
        if (winRatio && cJSON_IsNumber(winRatio)) {
            rank.WinRatio = static_cast<float>(winRatio->valuedouble);
            rank.WinRatioToString = DoubleToString(winRatio->valuedouble);
        }

        cJSON* score = cJSON_GetObjectItem(modeData, "currentRankPoint");
        if (score && cJSON_IsNumber(score)) {
            rank.RankPoint = score->valuedouble;
        }

        // 匹配模式显示为"匹配模式"
        rank.Tier = "";
        rank.SubTier = "";
        rank.TierToString = hasData ? U8("匹配模式") : U8("没玩此模式");

        // 标记为已更新
        rank.Updated = true;
    }
};

// 静态成员变量定义
std::unordered_map<std::string, std::string> Segment::ChineseToEnglishTier;