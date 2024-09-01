#pragma once

#include <string>
#include <cstdint>

extern std::string human_readable_byte_count(std::uintmax_t bytes);

#define CHECK(cond) do {                         \
        if (!(cond)) {                           \
            _check_failed(__FILE__, __LINE__);   \
        }                                        \
    } while(0)

extern void _check_failed(const char *file, int line);