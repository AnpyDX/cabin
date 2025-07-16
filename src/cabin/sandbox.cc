#include "sandbox.h"

#include <stdexcept>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

        if (hasImGuiContext) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            hasImGuiContext = false;
        }
    }

    void Sandbox::launch() {
        while (!glfwWindowShouldClose(window)) {
            renderFrame();

            if (hasImGuiContext) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                interfaceFrame();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    std::tuple<int, int> Sandbox::getWindowSize() {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        return { width, height };
    }

    void Sandbox::setWindowResizable(bool enable) {
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, enable);
    }

    void Sandbox::enableImGui() {
        hasImGuiContext = true;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 430");

        // Config ImGui styles and font
        ImGui::StyleColorsLight();
        ImGuiStyle* style = &ImGui::GetStyle();
        style->WindowRounding = 3;
        style->ChildRounding = 1;
        style->FrameRounding = 1;
        style->PopupRounding = 1;
        style->GrabRounding = 1;
        style->Colors[ImGuiCol_WindowBg].w = 0.95f;

        ImGuiIO& io = ImGui::GetIO();
        float baseFontSize = 19.0f;
        io.Fonts->AddFontFromFileTTF("assets/fonts/SourceSans3-Bold.ttf", baseFontSize, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    }
}