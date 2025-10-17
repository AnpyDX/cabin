#include "shape.h"
#include "cabin/core/vertexbuffer.h"

#include <cmath>
#include <utility>
#include <glad/glad.h>
#include <glm/geometric.hpp>
#include <glm/ext/scalar_constants.hpp>

namespace {
    static float SHAPE_CUBE_VERTICES[] = {
        /* Position           Normal              TexCoord */
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,

        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f 
    };

    float SHAPE_SQUARE_VERTICES[] = {
        /* Position          Normal             TexCoord */
         1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f
    };
}

namespace cabin::utils {

    Shape::Builder& Shape::Builder::asCube() {
        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(SHAPE_CUBE_VERTICES, sizeof(SHAPE_CUBE_VERTICES), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();
        m_indices.resize(36);
        for (int i = 0; i < 36; i++) m_indices[i] = i;

        return *this;
    }

    Shape::Builder& Shape::Builder::asSquare() {
        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(SHAPE_SQUARE_VERTICES, sizeof(SHAPE_SQUARE_VERTICES), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();

        m_indices = { 0, 1, 2, 0, 2, 3 };
        
        return *this;
    }

    Shape::Builder& Shape::Builder::asPlane(glm::vec2 size, glm::vec<2, int> division) {

        /* Vertices */
        std::vector<glm::vec3> positions((division.x + 1) * (division.y + 1));
        std::vector<glm::vec2> texCoords((division.x + 1) * (division.y + 1));

        for (int i = 0; i <= division.y; i++) {
            for (int j = 0; j <= division.x; j++) {
                float k_x = static_cast<float>(j) / division.x;
                float k_y = static_cast<float>(i) / division.y;

                glm::vec3 pos {};
                pos.x = size.x * (k_x - 0.5f);
                pos.y = size.y * (0.5f - k_y);
                pos.z = 0.0f;

                glm::vec2 texCoord { k_x, 1.0 - k_y };

                int index = i * (division.x + 1) + j;
                positions[index] = pos;
                texCoords[index] = texCoord;
            }
        }

        struct alignas(4) Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
        };
        std::vector<Vertex> vertices((division.x + 1) * (division.y + 1));
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i] = {
                positions[i], { 0.0f, 0.0f, 1.0f }, texCoords[i]
            };
        }

        /* Indices */
        std::vector<unsigned int> indices(6 * division.x * division.y);

        for (int i = 0; i < division.y; i++) {
            int base = (division.x + 1) * i;
            int baseNext = (division.x + 1) * (i + 1);
            int indexBase = division.x * i;

            for (int j = 0; j < division.x; j++) {
                int index = 6 * (indexBase + j);

                indices[index] = base + j;
                indices[index + 1] = baseNext + j;
                indices[index + 2] = base + j + 1;

                indices[index + 3] = base + j + 1;
                indices[index + 4] = baseNext + j;
                indices[index + 5] = baseNext + j + 1;
            }
        }

        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();
        
        m_indices = std::move(indices);
        
        return *this;
    }

    Shape::Builder& Shape::Builder::asShpere(float radius, uint32_t division) {
        constexpr float PI = glm::pi<float>();

        /* Vertices */
        std::vector<glm::vec2> texCoords(2 * division * division);
        std::vector<glm::vec3> positions(2 * division * division);
        for (size_t i = 0; i < division; i++) {
            for (size_t j = 0; j < 2 * division; j++) {
                float theta = (PI * i) / (division - 1);
                float alpha = (2.0f * PI * j) / (2 * division - 1);

                glm::vec3 pos {};
                pos.x = std::cos(alpha);
                pos.z = std::sin(alpha);
                pos = pos * radius * std::sin(theta);
                pos.y = radius * std::cos(theta);

                glm::vec2 texCoord;
                texCoord.x = j / (2.0f * division - 1.0f);
                texCoord.y = 1.0f - i / (division - 1.0f);

                size_t index = i * 2 * division + j;

                positions[index] = pos;
                texCoords[index] = texCoord;
            }
        }

        struct alignas(4) Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
        };

        std::vector<Vertex> vertices(2 * division * division);
        for (int i = 0; i < vertices.size(); i++) {
            vertices[i] = {
                positions[i], positions[i], texCoords[i]
            };
        }

        /* Indices */
        std::vector<unsigned int> indices(6 * (2 * division - 1) * (division - 1));
        for (unsigned int i = 0; i < division - 1; i++) {
            for (unsigned int j = 0; j < 2 * division - 1; j++) {
                int doubleDiv = 2 * division;
                size_t index = 6 * (i * (doubleDiv - 1)  + j);

                unsigned int baseHigh = i * doubleDiv;
                unsigned int baseLow = (i + 1) * doubleDiv;

                indices[index] = baseHigh + j;
                indices[index + 1] = baseHigh + j + 1;
                indices[index + 2] = baseLow + j;

                indices[index + 3] = baseLow + j + 1;
                indices[index + 4] = baseLow + j;
                indices[index + 5] = baseHigh + j + 1;
            }
        }

        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();
        
        m_indices = std::move(indices);
        
        return *this;
    }

    Shape Shape::Builder::build() {
        Shape result {};
        result.vertices = std::move(m_vertices);
        result.indices = std::move(m_indices);

        return result;
    }

    Shape::Shape(Shape&& right) noexcept {
        vertices = std::move(right.vertices);
        indices = std::move(right.indices);
    }

    Shape& Shape::operator=(Shape&& right) noexcept {
        vertices = std::move(right.vertices);
        indices = std::move(right.indices);
        return *this;
    }

    void Shape::draw() {
        vertices.bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), 
                       GL_UNSIGNED_INT, indices.data());
    }
}