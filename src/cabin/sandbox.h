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
#include "utils/camera.h"
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

        /** Register a camera for current app.
         *
         *  This function will handle inputs and ImGui mouse conflict.
         *
         * @note Using this method has the following side-effects:
         
                1. `glfwMouseButtonCallback` occupied.
                2. `glfwUserPointer` will be set to `this`.
                3. `GLFW_RAW_MOUSE_MOTION` will be set, if supported.
                
                If you need to add custom logic within the above methods, 
                please register manually instead of calling this function.
         */
        void registerCamera(utils::Camera* camera);

    public:
        GLFWwindow* window { nullptr };
        bool hasImGuiContext { false };

    private:
        utils::Camera* m_registerdCamera { nullptr };
    };

}