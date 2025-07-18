#pragma once
#include <format>
#include <iostream>

namespace cabin::utils {
    struct Console {
    public:
        template <typename T>
        static void debug(const T& msg) {
#ifndef NDEBUG
            std::cerr
                << std::format("\033[32m[debug]\033[0m {} \n", msg);
#endif
        }

        template <typename T>
        static void info(const T& msg) {
            std::cerr
                << std::format("\033[32m[info]\033[0m {} \n", msg);
        }

        template <typename T>
        static void error(const T& msg) {
            std::cerr
                << std::format("\033[31m[error]\033[0m {} \n", msg);
        }
    };
}