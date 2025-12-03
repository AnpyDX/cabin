#include "sandbox.h"

#include <string>
#include <stdexcept>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "misc/RobotoMonoBoldTTF.h"

namespace {
    static void applyImGuiStyle() {
        ImGuiStyle& style = ImGui::GetStyle();

        /* Main */
        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding  = ImVec2(4, 3);
        style.ItemSpacing   = ImVec2(8, 4);
        style.ItemInnerSpacing  = ImVec2(4, 4);
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = 20;
        style.ScrollbarSize = 14;
        style.GrabMinSize   = 12;

        /* Borders */
        style.WindowBorderSize = 1;
        style.ChildBorderSize  = 1;
        style.PopupBorderSize  = 1;
        style.FrameBorderSize  = 0;

        /* Roudding */
        style.WindowRounding = 3;
        style.ChildRounding  = 1;
        style.FrameRounding  = 1;
        style.PopupRounding  = 1;
        style.GrabRounding   = 1;
        style.ScrollbarRounding = 9;

        /* Tabs */
        style.TabBorderSize      = 1;
        style.TabBarBorderSize   = 1;
        style.TabBarOverlineSize = 1;
        style.TabRounding        = 2;

        /* Tables */
        style.CellPadding = ImVec2(4, 2);
        style.TableAngledHeadersTextAlign = ImVec2(0.5f, 0.0f);

        /* Trees */
        style.TreeLinesFlags    = ImGuiTreeNodeFlags_DrawLinesFull;
        style.TreeLinesSize     = 1;
        style.TreeLinesRounding = 0;

        /* Windows */
        style.WindowTitleAlign = ImVec2(0.0f, 0.4f);
        style.WindowBorderHoverPadding = 4;
        style.WindowMenuButtonPosition = ImGuiDir_Left;

        /* Widgets */
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign     = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0, 0);
        style.SeparatorTextBorderSize = 2;
        style.SeparatorTextAlign   = ImVec2(0.0f, 0.5f);
        style.SeparatorTextPadding = ImVec2(16, 2);
        style.LogSliderDeadzone = 4;
        style.ImageBorderSize   = 0;

        /* Colors */
        ImGui::StyleColorsClassic();
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        style.Colors[ImGuiCol_TitleBgCollapsed].w = 128.0f;
    }
}

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
            if (m_registerdCamera)
                m_registerdCamera->updateInput(window);

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

        /* Disable `imgui.ini` by default */
        ImGui::GetIO().IniFilename = nullptr;

        /* Customize ImGui's Styles and Font */
        applyImGuiStyle();

        float baseFontSize = 18.0f;
        std::string fontName = "Roboto Mono Bold";

        ImFontConfig fontCfg {};
        fontCfg.FontDataOwnedByAtlas = false;
        std::copy_n(fontName.c_str(), fontName.size(), fontCfg.Name);
        
        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            static_cast<void*>(RobotoMonoBoldTTF),
            sizeof(RobotoMonoBoldTTF),
            baseFontSize, &fontCfg
        );
    }

    void Sandbox::registerCamera(utils::Camera* camera) {
        if (!camera)
            throw std::runtime_error("failed to register the camera with a nullptr");
        
        m_registerdCamera = camera;

        // Use raw input to prevent twitchy view motion.
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

        glfwSetWindowUserPointer(window, static_cast<void*>(this));
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            auto app = reinterpret_cast<Sandbox*>(glfwGetWindowUserPointer(window));

            if (app->hasImGuiContext && ImGui::GetIO().WantCaptureMouse) {
                ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
                return;
            }

            if (app->m_registerdCamera)
                app->m_registerdCamera->mouseButtonCallback(window, button, action);
        });
    }
}