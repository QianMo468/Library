#pragma once
#include <ctime>
#include <string>

class DateUtils {
public:
    static std::string formatTime(std::time_t time);
    static int daysBetween(std::time_t start, std::time_t end);
    static std::time_t getCurrentTime();
};