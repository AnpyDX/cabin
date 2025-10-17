/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 *
 * `Image Viewer` is a image preview application,
 *  showing a simple usage of `core::Framebuffer`.
 */

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <imgui.h>

#include "cabin/sandbox.h"
#include "cabin/utils/shape.h"
#include "cabin/core/shader.h"
#include "cabin/core/texture.h"
#include "cabin/core/framebuffer.h"
using namespace cabin;


class ImageViewer: public Sandbox {
public:
    ImageViewer(): Sandbox("Image Viewer", 800, 600) {
        m_square = utils::Shape::Builder().asSquare().build();

        m_imageShader = core::Shader::Builder()
                            .fromFile("assets/shaders/image.shader")
                            .build();

        m_pixelateShader = core::Shader::Builder()
                            .fromFile("assets/shaders/pixelate.shader")
                            .build();

        m_imageTexture = core::Texture::Builder()
                            .fromFile2D("assets/textures/awesomeface.png")
                            .setWrap(GL_REPEAT, GL_REPEAT)
                            .setFilter(GL_LINEAR, GL_LINEAR)
                            .genMipmap()
                            .build();

        m_subFramebuffer = core::FrameBuffer();

        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
        glfwSwapInterval(true);

        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto app = reinterpret_cast<ImageViewer*>(glfwGetWindowUserPointer(window));

            if (key == GLFW_KEY_M && action == GLFW_PRESS)
                app->m_showMenu = !app->m_showMenu;
            else if (key == GLFW_KEY_C && action == GLFW_PRESS)
                app->resetImageParameters();
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            if (ImGui::GetIO().WantCaptureMouse) return;
            
            auto app = reinterpret_cast<ImageViewer*>(glfwGetWindowUserPointer(window));

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS) {
                    app->m_underMove = true;
                    double x, y;
                    glfwGetCursorPos(window, &x, &y);
                    app->m_clickPosition = { x, y };
                }
                else if (action == GLFW_RELEASE) {
                    app->m_underMove = false;
                    app->m_imageLastPosition = app->m_imageCurrentPosition;
                }
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            auto app = reinterpret_cast<ImageViewer*>(glfwGetWindowUserPointer(window));
            app->m_imageScaleFactor = glm::clamp(app->m_imageScaleFactor + static_cast<float>(yoffset) * 0.1f, 0.1f, 10.0f);
        });

        glfwSetDropCallback(window, [](GLFWwindow* window, int count, const char** paths) {
            auto app = reinterpret_cast<ImageViewer*>(glfwGetWindowUserPointer(window));

            if (count > 1) {
                app->m_openErrorInfo = "Too many files! Drop single image at a time.";
                app->m_showError = true;
            }
            else {
                try {
                    auto inputImage = core::Texture::Builder()
                                        .fromFile2D(paths[0])
                                        .setWrap(GL_REPEAT, GL_REPEAT)
                                        .setFilter(GL_LINEAR, GL_LINEAR)
                                        .genMipmap()
                                        .build();
                    app->m_imageTexture = std::move(inputImage);
                    app->m_imagePath = paths[0];
                    app->resetImageParameters();
                } catch (const std::exception& e) {
                    app->m_openErrorInfo = e.what();
                    app->m_showError = true;
                }
            }
        });

        enableImGui();
        ImGui::GetIO().IniFilename = nullptr; // Disable imgui.ini

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        resetImageParameters();
    }

    void genPixelizedTexture() {
        int texWidth, texHeight;
        int scaleFactor = 4 * m_pixelateIntensity;
        texWidth = m_imageTexture.width / scaleFactor;
        texHeight = m_imageTexture.height / scaleFactor;

        m_pixelateTexture = core::Texture::Builder()
                                .asEmpty2D(texWidth, texHeight, GL_RGBA16F)
                                .setFilter(GL_NEAREST, GL_NEAREST)
                                .build();
        m_subFramebuffer.attachTexture(GL_COLOR_ATTACHMENT0, m_pixelateTexture.id.value());

        m_subFramebuffer.bind();
        m_pixelateShader.bind();
        m_pixelateShader.setInt("texture0", 0);
        m_imageTexture.active(0);

        glViewport(0, 0, texWidth, texHeight);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_square.draw();

        auto [width, height] = getWindowSize();
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resetImageParameters() {
        m_pixelateIntensity = 0;
        m_imageScaleFactor = 1.0f;
        m_imageLastPosition = glm::vec2 { 0.0f };
        m_imageCurrentPosition = glm::vec2 { 0.0f };

        if (m_imageTexture.width > m_imageTexture.height) {
            m_imageScale.x = 1.0f;
            m_imageScale.y = static_cast<float>(m_imageTexture.height) / m_imageTexture.width;
        }
        else {
            m_imageScale.y = 1.0f;
            m_imageScale.x = static_cast<float>(m_imageTexture.width) / m_imageTexture.height;
        }
    }

    void renderFrame() override {
        processInput();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto [width, height] = getWindowSize();
        float halfW, halfH;
        halfW = width / 2.0f;
        halfH = height / 2.0f;

        glm::mat4 model { 1.0f };
        model = glm::translate(model, glm::vec3 { m_imageCurrentPosition, 0.0f });
        model = glm::scale(model, glm::vec3(m_imageScale * m_imageScaleFactor * 300.0f , 1.0f));

        glm::mat4 projection = glm::ortho(-halfW, halfW, -halfH, halfH, -5.0f, 5.0f);
        
        m_imageShader.bind();
        m_imageShader.setInt("texture0", 0);
        m_imageShader.setMat4("model", model);
        m_imageShader.setMat4("projection", projection);

        if (m_pixelateIntensity)
            m_pixelateTexture.active(0);
        else
            m_imageTexture.active(0);

        m_square.draw();
    }

    void interfaceFrame() override {
        if (!m_showMenu) return;

        /* Menu Window */
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | 
                                       ImGuiWindowFlags_AlwaysAutoResize | 
                                       ImGuiWindowFlags_NoSavedSettings | 
                                       ImGuiWindowFlags_NoFocusOnAppearing | 
                                       ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoMove;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 workPos = viewport->WorkPos;
        ImVec2 workSize = viewport->WorkSize;
        ImVec2 windowPos = {
            workPos.x + 10.0f,
            workPos.y + workSize.y - 10.0f
        };
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, { 0.0f, 1.0f });
        ImGui::SetNextWindowBgAlpha(0.75f);

        if (ImGui::Begin("menu", nullptr, windowFlags))
        {
            ImGui::Text("%s", m_imagePath.c_str());
            ImGui::BulletText("Size: %d * %d", m_imageTexture.width, m_imageTexture.height);
            ImGui::BulletText("Format: %s", m_imageTexture.format == GL_RGB ? "RGB" : "RGBA");

            ImGui::SeparatorText("Pixelate");
            if (ImGui::SliderInt("", &m_pixelateIntensity, 0, 10)) {
                if (m_pixelateIntensity)
                    genPixelizedTexture();
            }

            ImGui::SeparatorText("Helps");
            ImGui::Text("- Press \"M\" to hide menu.");
            ImGui::Text("- Press \"C\" to reset image.");
            ImGui::Text("- Drag file into window to load.");

            if (ImGui::IsMousePosValid())
                ImGui::Text("Cursor Pos: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
            else
                ImGui::Text("Cursor Pos: <invalid>");
        }
        ImGui::End();

        /* Error Popup Window */
        if (m_showError) { ImGui::OpenPopup("Error"); m_showError = false; }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Text("%s", m_openErrorInfo.c_str());
            ImGui::Separator();

            float spaceOffset = (ImGui::GetContentRegionAvail().x - 130.0f) / 2.0f;
            ImGui::Dummy(ImVec2 { spaceOffset, 0.0f }); ImGui::SameLine();

            if (ImGui::Button("OK", ImVec2(120.0f, 0.0f))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
    }

    void processInput() {
        if (m_underMove) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            glm::vec2 pos { x, y };
            glm::vec2 offset = pos - m_clickPosition;
            offset.y *= -1.0f;
            m_imageCurrentPosition = m_imageLastPosition + offset;
        }
    }

private:
    bool m_showMenu = true;
    bool m_showError = false;
    std::string m_imagePath { "awesomeface.png" };
    std::string m_openErrorInfo {};
    
    bool m_underMove = false;
    glm::vec2 m_clickPosition {};

    int m_pixelateIntensity = 0;
    float m_imageScaleFactor = 1.0f;
    glm::vec2 m_imageScale {};
    glm::vec2 m_imageLastPosition { 0.0f };
    glm::vec2 m_imageCurrentPosition { 0.0f };
    
private:
    utils::Shape m_square;
    core::Shader m_imageShader;
    core::Shader m_pixelateShader;
    core::FrameBuffer m_subFramebuffer;
    core::Texture m_imageTexture;
    core::Texture m_pixelateTexture;
};

int main() {
    return SandboxApp<ImageViewer>::run();
}