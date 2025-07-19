/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

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