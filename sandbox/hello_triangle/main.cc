/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 *
 *
 * `Hello Triangle` is a sample application showing 
 *  how to build a basic cabin sandbox-app with
 *  ImGui and FPS-style camera integrated.
 */

#include <memory>
#include <vector>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "cabin/sandbox.h"
#include "cabin/utils/camera.h"
#include "cabin/core/shader.h"
#include "cabin/core/vertexbuffer.h"
#include "cabin/core/indexbuffer.h"
using namespace cabin;


struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

static std::vector<Vertex> vertices = {
    { { 0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
};

static std::vector<unsigned int> indices = { 0, 1, 2 };

class HelloTriangle: public Sandbox {
public:
    HelloTriangle()
    : Sandbox("Hello Triangle", 800, 600) {
        m_shader = std::make_unique<core::Shader>(
            core::Shader::Builder()
                        .fromFile("hello_triangle/main.shader")
                        .build()
        );
        m_vertexBuffer = std::make_unique<core::VertexBuffer>(
            core::VertexBuffer::Builder()
                                .setBuffer(static_cast<void*>(vertices.data()), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .build()
        );
        m_indexBuffer = std::make_unique<core::IndexBuffer>(
            core::IndexBuffer::Builder()
                            .setBuffer(indices.data(), indices.size(), GL_STATIC_DRAW)
                            .build()
        );

        // Enable and initialize ImGui.
        enableImGui();

        // Prevent `imgui.ini` generation.
        ImGui::GetIO().IniFilename = nullptr;

        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));

        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            // Pass mouse button input to ImGui (as we have called `glfwSetMouseButtonCallback` manully, which
            //                                    overwrite the ImGui callback setted in `enableImGui`).
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

            // Determine whether the cursor is within ImGui windows.
            if (ImGui::GetIO().WantCaptureMouse) return;

            auto app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));

            // Pass mouse button input to camera.
            app->m_camera.mouseButtonCallback(window, button, action);
        });
    }

    void renderFrame() override {
        /* Write Render Logic Here ... */

        processInput();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->bind();
        m_vertexBuffer->bind();
        m_indexBuffer->bind();

        glm::mat4 model { 1.0f };
        model = glm::translate(model, glm::vec3(0.0, 0.0, -1.0));
        model = glm::scale(model, glm::vec3(0.5, 0.5, 1.0));
        
        glm::mat4 view = m_camera.getLookAt();

        glm::mat4 projection;
        auto [width, height] = getWindowSize();
        // Prevent `height == 0` when window is minimized.
        float aspect = static_cast<float>(width) / (height != 0.0f ? height : 1.0f);
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 50.0f);

        m_shader->setMat4("model", model);
        m_shader->setMat4("view", view);
        m_shader->setMat4("projection", projection);

        glDrawElements(GL_TRIANGLES, m_indexBuffer->count, m_indexBuffer->storageType, nullptr);
    }

    void interfaceFrame() override {
        /** Write UI Logic Here ... */

        if (ImGui::Begin("Hello Triangle")) {
            ImGui::SeparatorText("Helps");
            ImGui::BulletText("\"Left / Right-click\" the mouse to control/exit camera.");
            ImGui::BulletText("Press \"W / S / A / D\" to move the camera.");
            ImGui::BulletText("Press \"Escape\" to exit the application.");

            ImGui::SeparatorText("Camera");
            ImGui::Text("Speed: %.2f", glm::length(m_camera.getSpeed()));
            ImGui::Text("Position: ( %.2f, %.2f, %.2f )", m_camera.position.x, m_camera.position.y, m_camera.position.z);
        }
        ImGui::End();
    }

    void processInput() {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Update keyboard input and cursor position for camera.
        m_camera.updateInput(window);
    }

private:
    utils::Camera m_camera { { 0.0f, 0.0f, 0.0f }, { 0.0, 0.0, -1.0 }, { 0.0, 1.0, 0.0 } };
    std::unique_ptr<core::Shader> m_shader;
    std::unique_ptr<core::VertexBuffer> m_vertexBuffer;
    std::unique_ptr<core::IndexBuffer> m_indexBuffer;
};


int main() {
    return SandboxApp<HelloTriangle>::run();
}