#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "cabin/sandbox.h"
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

static std::vector<unsigned int> indices = {
    0, 1, 2
};

class HelloTriangle: public Sandbox {
public:
    HelloTriangle()
    : Sandbox("Hello Triangle", 800, 600) {
        m_shader = std::make_unique<core::Shader>(
            core::Shader::Builder()
                        .setSourceFile("hello_triangle/main.shader")
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
    }

    void inLoop() override {
        processInput();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_shader->bind();
        m_vertexBuffer->bind();
        m_indexBuffer->bind();
        glDrawElements(GL_TRIANGLES, m_indexBuffer->count, m_indexBuffer->storageType, nullptr);
    }

    void processInput() {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

private:
    std::unique_ptr<core::Shader> m_shader;
    std::unique_ptr<core::VertexBuffer> m_vertexBuffer;
    std::unique_ptr<core::IndexBuffer> m_indexBuffer;
};


int main() {
    return SandboxApp<HelloTriangle>::run();
}