#include "sandbox.h"

#include <stdexcept>
#include <GLFW/glfw3.h>


namespace cabin {

    Sandbox::Sandbox(const std::string& title, int width, int height) {

        // Window creation.
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window);

        // OpenGL context creation.
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwTerminate();
            throw std::runtime_error("failed to initialize OpenGL context!");
        }

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        });
    }

    Sandbox::~Sandbox() {
        if (window) {
            glfwDestroyWindow(window);
            glfwTerminate();
            window = nullptr;
        }
    }

    void Sandbox::launch() {
        while (!glfwWindowShouldClose(window)) {
            inLoop();
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void Sandbox::setResizable(bool enable) {
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, enable);
    }
}