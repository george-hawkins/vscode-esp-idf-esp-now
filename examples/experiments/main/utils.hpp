#pragma once

#include <string>
#include <cstdint>
#include <bitset>

namespace merkur {

namespace fmt {

extern std::string byte_count(std::intmax_t size);

extern std::string thousands(std::intmax_t value);

template <typename T>
std::string bits(T value) {
    return std::bitset<sizeof(T) * 8>{value}.to_string();
}

}

#define CHECK(cond) do {                               \
        if (!(cond)) {                                 \
            merkur::_check_failed(__FILE__, __LINE__); \
        }                                              \
    } while(0)

extern void _check_failed(const char *file, int line);

}