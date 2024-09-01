#include <array>
#include <cmath>

#include "esp_log.h"

#include "utils.hpp"

static const char* TAG = "util";

std::string human_readable_byte_count(std::uintmax_t bytes) {
    constexpr std::array<const char*, 7> suffixes{"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    int suffixIndex = 0;
    double value = static_cast<double>(bytes);

    while (value >= 1024 && suffixIndex < suffixes.size() - 1) {
        value /= 1024;
        ++suffixIndex;
    }

    double intPart;
    auto fracPart = std::modf(value, &intPart);
    auto places = fracPart < 0.1 ? 0 : 1;

    // 1023.0 KiB
    // 12345678901
    char s[16];

    std::snprintf(s, sizeof(s), "%.*f %s", places, value, suffixes[suffixIndex]);

    return std::string{s};

    // Using std::format balloons the image size by +360KiB.
    // All the formatting pulled in by memory.cpp doesn't move the dial.
    // So that makes me thing the ESP-IDF function I use already pull all that in.
    // return std::format("{:.{}f} {}", value, places, suffixes[suffixIndex]);
}

void _check_failed(const char *file, int line)
{
    ESP_LOGE(TAG, "check failed at file: \"%s\" line %d\n", file, line);
    abort();
}