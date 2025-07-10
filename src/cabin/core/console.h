#pragma once
#include <string>
#include <format>
#include <iostream>

namespace cabin::utils {
    struct Console {
    public:
        static void debug(const std::string& msg) {
#ifndef NDEBUG
            std::cerr
                << std::format("\033[32m[debug]\033[0m {} \n", msg);
#endif
        }

        static void info(const std::string& msg) {
            std::cerr
                << std::format("\033[32m[info]\033[0m {} \n", msg);
        }

        static void error(const std::string& msg) {
            std::cerr
                << std::format("\033[31m[error]\033[0m {} \n", msg);
        }
    };
}