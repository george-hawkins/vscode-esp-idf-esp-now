#include <array>
#include <cmath>

#include "esp_log.h"

#include "utils.hpp"

static const char* TAG = "util";

void merkur::_check_failed(const char *file, int line)
{
    ESP_LOGE(TAG, "check failed at file: \"%s\" line %d\n", file, line);
    abort();
}

static void insert_thousands_separator(std::string& number) {
    CHECK(!number.empty());

    // Comparing a negative signed value with an unsigned value has surprising results.
    std::ptrdiff_t start = (number[0] == '-') ? 1 : 0;
    std::ptrdiff_t insert_position = number.size() - 3;
    
    while (insert_position > start) {
        number.insert(insert_position, ",");
        insert_position -= 3;
    }
}

std::string merkur::fmt::thousands(std::intmax_t value) {
    char c[16];

    std::snprintf(c, std::size(c), "%lld", value);

    std::string s{c};

    insert_thousands_separator(s);

    return s;
}

std::string merkur::fmt::byte_count(std::intmax_t size) {
    constexpr std::array<const char*, 7> suffixes{"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    int suffixIndex = 0;
    bool is_negative = size < 0;
    double value = static_cast<double>(std::abs(size));

    while (value >= 1024 && suffixIndex < suffixes.size() - 1) {
        value /= 1024;
        ++suffixIndex;
    }

    double intPart;
    auto fracPart = std::modf(value, &intPart);
    auto places = fracPart < 0.1 ? 0 : 1;

    char c[16];

    const auto sign = is_negative ? "-" : "";
    std::snprintf(c, std::size(c), "%s%.*f %s", sign, places, value, suffixes[suffixIndex]);

    return std::string{c};

    // Using std::format balloons the image size by +360KiB.
    // All the formatting pulled in by memory.cpp doesn't move the dial.
    // So that makes me think the ESP-IDF function I use already pull all that in.
    // return std::format("{:.{}f} {}", value, places, suffixes[suffixIndex]);
}
