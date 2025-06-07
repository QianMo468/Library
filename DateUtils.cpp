#include "DateUtils.h"

std::string DateUtils::formatTime(std::time_t time) {
    char buffer[26];
    ctime_s(buffer, sizeof(buffer), &time);
    std::string result(buffer);
    if (!result.empty() && result[result.length() - 1] == '\n') {
        result.erase(result.length() - 1);
    }
    return result;
}

int DateUtils::daysBetween(std::time_t start, std::time_t end) {
    return static_cast<int>((end - start) / (24 * 60 * 60));
}

std::time_t DateUtils::getCurrentTime() {
    return std::time(nullptr);
}