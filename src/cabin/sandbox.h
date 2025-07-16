#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "core/console.h"

namespace cabin {
    class Sandbox;

    /**
     * @brief Sandbox derived application launcher,
     *        which provides exceptions handling.
     */
    template <typename T>
        requires std::derived_from<T, Sandbox>
    class SandboxApp {
    public:
        template <typename ...Args>
        static int run(Args ...args) {
            try {
                T app { args... };
                app.launch();
            } catch (const std::exception& e) {
                utils::Console::error(e.what());
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
    };

    /**
     * @brief Sandbox base class, providing window and OpenGL context. 
     */
    class Sandbox {
    public:
        Sandbox() = delete;
        Sandbox(const std::string& title, int width, int height);

        ~Sandbox();

        /**
         * @brief Launch the sandbox mainloop.
         */
        void launch();

        /**
         * @brief Custom function in render loop.
         */
        virtual void renderFrame() = 0;

        /**
         * @brief 
         */
        virtual void interfaceFrame() {};

        std::tuple<int, int> getWindowSize();

        /**
         * @brief Set whether window is resizable.
         * 
         * @param enable 
         */
        void setWindowResizable(bool enable);

        /**
         * @brief Enable ImGui.
         */
        void enableImGui();

    public:
        GLFWwindow* window { nullptr };
        bool hasImGuiContext { false };
    };

}