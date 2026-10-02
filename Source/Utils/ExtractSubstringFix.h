#pragma once
#include <string>
#include <regex>

namespace Utils {
    // 增强版的ExtractSubstring函数，添加更多错误处理和调试信息
    inline std::string ExtractSubstringEnhanced(const std::string& input, const std::string& pattern) {
        try {
            std::regex re(pattern);
            std::smatch match;
            
            if (std::regex_search(input, match, re) && match.size() > 1) {
                return match[1].str();
            }
            
            // 如果使用正则表达式提取失败，尝试直接提取数字
            std::regex numRegex("\\d+");
            if (std::regex_search(input, match, numRegex)) {
                return match[0].str();
            }
            
            // 如果仍然失败，检查输入字符串是否包含"COM"
            size_t comPos = input.find("COM");
            if (comPos != std::string::npos) {
                // 找到"COM"后面的数字
                size_t numStart = comPos + 3; // "COM"长度为3
                size_t numEnd = input.find_first_not_of("0123456789", numStart);
                if (numEnd == std::string::npos) {
                    numEnd = input.length();
                }
                
                if (numStart < numEnd) {
                    return input.substr(numStart, numEnd - numStart);
                }
            }
        }
        catch (const std::exception& e) {
            // 异常处理，返回默认值
            return "1"; // 默认使用COM1
        }
        
        // 如果所有方法都失败，返回默认值
        return "1"; // 默认使用COM1
    }
}
