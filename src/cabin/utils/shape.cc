#include "shape.h"
#include "cabin/core/vertexbuffer.h"

#include <cmath>
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

    float SHAPE_PLANE_VERTICES[] = {
        // Position       Normal          TexCoord
         1.0,  1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0,
        -1.0,  1.0, 0.0,  0.0, 0.0, 1.0,  0.0, 1.0,
        -1.0, -1.0, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0,

         1.0,  1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 1.0,
        -1.0, -1.0, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0,
         1.0, -1.0, 0.0,  0.0, 0.0, 1.0,  1.0, 0.0
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
        m_count = 36;

        return *this;
    }

    Shape::Builder& Shape::Builder::asPlane() {
        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(SHAPE_PLANE_VERTICES, sizeof(SHAPE_PLANE_VERTICES), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();
        m_count = 6;
        
        return *this;
    }

    Shape::Builder& Shape::Builder::asShpere(float radius, uint32_t division) {
        constexpr float PI = glm::pi<float>();

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

        struct alignas(4) Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
        };
        std::vector<Vertex> vertices(6 * (2 * division - 1) * (division - 1));
        
        for (size_t i = 0; i < 6 * (2 * division - 1) * (division - 1); i += 3) {
            for (size_t j = 0; j < 3; j++) {
                size_t index = indices[i + j];
                vertices[i + j].position = positions[index];
                vertices[i + j].normal = positions[index];
                vertices[i + j].texCoord = texCoords[index];
            }
        }

        m_vertices = core::VertexBuffer::Builder()
                                .setBuffer(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW)
                                .addAttribute<float>(0, 3)
                                .addAttribute<float>(1, 3)
                                .addAttribute<float>(2, 2)
                                .build();
        
        m_count = static_cast<GLsizei>(vertices.size());
        
        return *this;
    }

    Shape Shape::Builder::build() {
        Shape result {};
        result.vertices = std::move(m_vertices);
        result.count = m_count;

        return result;
    }

    Shape::Shape(Shape&& right) noexcept {
        count = right.count;
        vertices = std::move(right.vertices);
    }

    Shape& Shape::operator=(Shape&& right) noexcept {
        count = right.count;
        vertices = std::move(right.vertices);
        return *this;
    }

    void Shape::draw() {
        vertices.bind();
        glDrawArrays(GL_TRIANGLES, 0, count);
    }
}