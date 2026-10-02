#include "Decrypt.h"
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"
#include "common/Offset.h"
#include <string>
#include <intrin.h>
#include <cctype>
#include <cstdint>
#include <array>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <Utils/Utils.h>

namespace
{
    using XeDecryptFunction = uint64_t(*)(uint64_t key, uint64_t base);

    std::mutex g_xeMutex;
    XeDecryptFunction g_decFunction = nullptr;
    void* g_decFunctionMemory = nullptr;
    uint64_t g_xeKey = 0;

    bool IsCanonicalUserAddress(uint64_t address)
    {
        return address >= 0x10000 && address <= 0x00007FFFFFFFFFFFULL;
    }
}

// ============================================================
// 动态表达式解析器 - 支持运行时解析并执行 CIndex 表达式
// ============================================================

class ExprParser {
public:
    ExprParser(const std::string& expr, DWORD val) : m_expr(expr), m_pos(0), m_value(val) {}

    DWORD Parse() {
        DWORD result = ParseBitwiseOr();
        return result;
    }

private:
    std::string m_expr;
    size_t m_pos;
    DWORD m_value;

    void SkipSpaces() {
        while (m_pos < m_expr.size() && std::isspace((unsigned char)m_expr[m_pos]))
            m_pos++;
    }

    char Peek() {
        SkipSpaces();
        return m_pos < m_expr.size() ? m_expr[m_pos] : '\0';
    }

    char Get() {
        SkipSpaces();
        return m_pos < m_expr.size() ? m_expr[m_pos++] : '\0';
    }

    // 优先级从低到高: ^ -> | -> & -> << >> -> + - -> * /
    // 实际C优先级: | < ^ < & < shift < +- < */
    // 但这里按C语言真实优先级来

    // 最低优先级: |
    DWORD ParseBitwiseOr() {
        DWORD left = ParseBitwiseXor();
        while (Peek() == '|' && (m_pos + 1 >= m_expr.size() || m_expr[m_pos + 1] != '|')) {
            Get(); // consume '|'
            left |= ParseBitwiseXor();
        }
        return left;
    }

    // ^
    DWORD ParseBitwiseXor() {
        DWORD left = ParseBitwiseAnd();
        while (Peek() == '^') {
            Get(); // consume '^'
            left ^= ParseBitwiseAnd();
        }
        return left;
    }

    // &
    DWORD ParseBitwiseAnd() {
        DWORD left = ParseShift();
        while (Peek() == '&' && (m_pos + 1 >= m_expr.size() || m_expr[m_pos + 1] != '&')) {
            Get(); // consume '&'
            left &= ParseShift();
        }
        return left;
    }

    // << >>
    DWORD ParseShift() {
        DWORD left = ParseAddSub();
        while (true) {
            SkipSpaces();
            if (m_pos + 1 < m_expr.size() && m_expr[m_pos] == '<' && m_expr[m_pos + 1] == '<') {
                m_pos += 2;
                left <<= ParseAddSub();
            }
            else if (m_pos + 1 < m_expr.size() && m_expr[m_pos] == '>' && m_expr[m_pos + 1] == '>') {
                m_pos += 2;
                left >>= ParseAddSub();
            }
            else {
                break;
            }
        }
        return left;
    }

    // + -
    DWORD ParseAddSub() {
        DWORD left = ParseMulDiv();
        while (Peek() == '+' || Peek() == '-') {
            char op = Get();
            DWORD right = ParseMulDiv();
            if (op == '+') left += right;
            else left -= right;
        }
        return left;
    }

    // * /
    DWORD ParseMulDiv() {
        DWORD left = ParseUnary();
        while (Peek() == '*' || Peek() == '/') {
            char op = Get();
            DWORD right = ParseUnary();
            if (op == '*') left *= right;
            else if (right != 0) left /= right;
        }
        return left;
    }

    // ~ (按位取反) 和一元 -
    DWORD ParseUnary() {
        if (Peek() == '~') {
            Get();
            return ~ParseUnary();
        }
        if (Peek() == '-') {
            Get();
            return (DWORD)(-(int32_t)ParseUnary());
        }
        return ParsePrimary();
    }

    // 检查当前位置是否匹配指定字符串
    bool MatchStr(const std::string& s) {
        SkipSpaces();
        if (m_pos + s.size() > m_expr.size()) return false;
        for (size_t i = 0; i < s.size(); i++) {
            if (m_expr[m_pos + i] != s[i]) return false;
        }
        // 确保不是更长标识符的前缀
        size_t endPos = m_pos + s.size();
        if (endPos < m_expr.size() && (std::isalnum((unsigned char)m_expr[endPos]) || m_expr[endPos] == '_'))
            return false;
        return true;
    }

    // 基本元素: 数字, value, _rotr(), _rotl(), 括号
    DWORD ParsePrimary() {
        SkipSpaces();

        // 括号
        if (Peek() == '(') {
            Get(); // consume '('
            DWORD result = ParseBitwiseOr();
            if (Peek() == ')') Get(); // consume ')'
            return result;
        }

        // _rotr(expr, shift)
        if (MatchStr("_rotr")) {
            m_pos += 5;
            if (Peek() == '(') Get();
            DWORD val = ParseBitwiseOr();
            if (Peek() == ',') Get();
            DWORD shift = ParseBitwiseOr();
            if (Peek() == ')') Get();
            return _rotr(val, (int)shift);
        }

        // _rotl(expr, shift)
        if (MatchStr("_rotl")) {
            m_pos += 5;
            if (Peek() == '(') Get();
            DWORD val = ParseBitwiseOr();
            if (Peek() == ',') Get();
            DWORD shift = ParseBitwiseOr();
            if (Peek() == ')') Get();
            return _rotl(val, (int)shift);
        }

        // value 变量
        if (MatchStr("value")) {
            m_pos += 5;
            return m_value;
        }

        // 十六进制或十进制数字
        return ParseNumber();
    }

    DWORD ParseNumber() {
        SkipSpaces();
        DWORD result = 0;

        if (m_pos + 1 < m_expr.size() && m_expr[m_pos] == '0' &&
            (m_expr[m_pos + 1] == 'x' || m_expr[m_pos + 1] == 'X')) {
            // 十六进制
            m_pos += 2;
            while (m_pos < m_expr.size() && std::isxdigit((unsigned char)m_expr[m_pos])) {
                char c = m_expr[m_pos++];
                DWORD digit;
                if (c >= '0' && c <= '9') digit = c - '0';
                else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
                else digit = c - 'A' + 10;
                result = result * 16 + digit;
            }
        }
        else {
            // 十进制
            while (m_pos < m_expr.size() && std::isdigit((unsigned char)m_expr[m_pos])) {
                result = result * 10 + (m_expr[m_pos++] - '0');
            }
        }
        // 跳过类型后缀 u U l L
        while (m_pos < m_expr.size() && (m_expr[m_pos] == 'u' || m_expr[m_pos] == 'U' ||
            m_expr[m_pos] == 'l' || m_expr[m_pos] == 'L')) {
            m_pos++;
        }
        return result;
    }
};

// ============================================================
//  CIndex - hardcoded local, no cloud delivery
//  expr: ((((v ^ 0x7360F24) << 25) | (((v ^ 0x7360F24) >> 7) & 0x1FF0000)) ^ _rotr(v ^ 0x7360F24, 23) ^ 0xB621EC05)
//  ver: 2605.1.2.1  date: 2026-05-27
// ============================================================

DWORD Decrypt::CIndex(DWORD value)
{
    return ((value << 23) | ((value >> 9) & 0x7F0000)) ^
        _rotr(value ^ 0xBBEEADB7, 25) ^ 0xC5238AB3;
}

// 其他 Decrypt 方法
// ============================================================

void Decrypt::DestroyXe()
{
    std::lock_guard<std::mutex> lock(g_xeMutex);
    g_decFunction = nullptr;
    g_xeKey = 0;
    if (g_decFunctionMemory != nullptr)
    {
        VirtualFree(g_decFunctionMemory, 0, MEM_RELEASE);
        g_decFunctionMemory = nullptr;
    }
}

uint64_t Decrypt::Xe(uint64_t addr)
{
    if (addr == 0)
        return 0;

    std::lock_guard<std::mutex> lock(g_xeMutex);
    if (g_decFunction == nullptr)
    {
        const uint64_t gameBase = GameData.GameBase;
        const uint64_t decryptOffset = GameData.Offset["XenuineDecrypt"];
        if (!IsCanonicalUserAddress(gameBase) || decryptOffset == 0)
            return 0;

        uint64_t decryptPtr = 0;
        if (!mem.Read(gameBase + decryptOffset, &decryptPtr, sizeof(decryptPtr)) ||
            !IsCanonicalUserAddress(decryptPtr))
            return 0;

        std::array<unsigned char, 32> entry{};
        for (int depth = 0; depth < 4; ++depth)
        {
            if (!mem.Read(decryptPtr, entry.data(), static_cast<DWORD>(entry.size())))
                return 0;

            if (entry[0] == 0xE9)
            {
                int32_t displacement = 0;
                memcpy(&displacement, entry.data() + 1, sizeof(displacement));
                decryptPtr = decryptPtr + 5 + static_cast<int64_t>(displacement);
                continue;
            }

            if (entry[0] == 0xFF && entry[1] == 0x25)
            {
                int32_t displacement = 0;
                memcpy(&displacement, entry.data() + 2, sizeof(displacement));
                const uint64_t slot = decryptPtr + 6 + static_cast<int64_t>(displacement);
                if (!mem.Read(slot, &decryptPtr, sizeof(decryptPtr)))
                    return 0;
                continue;
            }

            // Current Xenuine builds use a small guard wrapper before the real
            // decrypt routine: test r8b,r8b; ...; ret; jmp rel32.
            if (entry[0] == 0x45 && entry[1] == 0x84 && entry[2] == 0xC0 &&
                entry[17] == 0xC3 && entry[18] == 0xE9)
            {
                int32_t displacement = 0;
                memcpy(&displacement, entry.data() + 19, sizeof(displacement));
                decryptPtr = decryptPtr + 23 + static_cast<int64_t>(displacement);
                continue;
            }

            break;
        }

        if (!mem.Read(decryptPtr, entry.data(), static_cast<DWORD>(entry.size())) ||
            entry[0] != 0x48 || (entry[1] != 0x8D && entry[1] != 0x8B) || entry[2] != 0x05)
        {
            if (Utils::IsDiagnosticLoggingEnabled())
            {
                static bool dumpedUnsupportedEntry = false;
                if (!dumpedUnsupportedEntry)
                {
                    dumpedUnsupportedEntry = true;
                    std::array<unsigned char, 1024> code{};
                    if (mem.Read(decryptPtr, code.data(), static_cast<DWORD>(code.size())))
                    {
                        std::ostringstream dump;
                        dump << std::hex << std::setfill('0');
                        for (const unsigned char byte : code)
                            dump << std::setw(2) << static_cast<unsigned int>(byte);
                        Utils::Log(1, "[XE] decryptPtr=0x%llX input=0x%llX code=%s",
                            decryptPtr, addr, dump.str().c_str());
                    }
                }
            }
            return addr;
        }

        int32_t relative = 0;
        memcpy(&relative, entry.data() + 3, sizeof(relative));
        const uint64_t target = decryptPtr + 7 + static_cast<int64_t>(relative);
        uint64_t key = target;
        if (entry[1] == 0x8B && !mem.Read(target, &key, sizeof(key)))
            return 0;
        if (!IsCanonicalUserAddress(key))
            return 0;

        std::array<unsigned char, 1024> shellcode{};
        if (!mem.Read(decryptPtr, shellcode.data() + 2,
            static_cast<DWORD>(shellcode.size() - 2)))
            return 0;

        shellcode[0] = 0x90;
        shellcode[1] = 0x90;
        shellcode[2] = 0x48;
        shellcode[3] = 0x8B;
        shellcode[4] = 0xC1;
        shellcode[5] = 0x90;
        shellcode[6] = 0x90;
        shellcode[7] = 0x90;
        shellcode[8] = 0x90;

        void* executable = VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (executable == nullptr)
            return 0;

        RtlCopyMemory(executable, shellcode.data(), shellcode.size());
        DWORD oldProtect = 0;
        if (!VirtualProtect(executable, 4096, PAGE_EXECUTE_READ, &oldProtect))
        {
            VirtualFree(executable, 0, MEM_RELEASE);
            return 0;
        }

        FlushInstructionCache(GetCurrentProcess(), executable, shellcode.size());
        g_decFunctionMemory = executable;
        g_decFunction = reinterpret_cast<XeDecryptFunction>(executable);
        g_xeKey = key;
    }

    return g_decFunction(g_xeKey, addr);
}
