/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "utils/console.h"

namespace cabin {
    class Sandbox;

    /** Launcher for Sandbox derived application class, 
     *  providing exceptions handling function.
     *
     * @see Usage example: 
     *       sandbox/hello_triangle/main.cc
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

    /** Sandbox Application Base
     *
     * --------------------------
     * @note Sandbox Base does the stuffs below:
     *
     *          - GLFW window creation
     *          - OpenGL Context creation
     *          - ImGui initialization (`enableImGui` needed)
     *
     *
     *      `Sandbox` and objects in `cabin::core` might throw 
     *      `std::exception`, so you are required to handle these 
     *       exceptions while creation and running in some ways:
     *
     *          - Use `try...catch` block
     *          - Use `SandboxApp<T>::run()` (recommanded)
     *
     * @see Usage example: 
     *       sandbox/hello_triangle/main.cc
     */
    class Sandbox {
    public:
        Sandbox() = delete;
        Sandbox(const std::string& title, int width, int height);

        ~Sandbox();

        //! Launch the sandbox mainloop.
        void launch();

        /** Render logic frame loop.
         *  All rendering logic should be written here.
         *
         * @note Executed before `interfaceFrame`.
         *
         *       Framebuffer can be switched freely inside `renderFrame`.
         */
        virtual void renderFrame() = 0;

        /** Interface logic frame loop.
         *  All ImGui widgets logic should be written here.
         *
         * @note Executed after `renderFrame`.
         *
         *       Framebuffer will be switch to `0`, therefore all
         *       ImGui contents will be render to FRAMEBUFFER0.
         */
        virtual void interfaceFrame() {};

        /** Get the size of window.
         *
         * @note e.g. `auto [w, h] = getWindowSize();`
         */
        std::tuple<int, int> getWindowSize();

        //! Set whether window is resizable.
        void setWindowResizable(bool enable);

        //! Enable and initialize ImGui context.
        void enableImGui();

    public:
        GLFWwindow* window { nullptr };
        bool hasImGuiContext { false };
    };

}