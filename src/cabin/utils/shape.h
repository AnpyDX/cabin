#pragma once

#include <glm/glm.hpp>
#include "cabin/core/vertexbuffer.h"


namespace cabin::utils {
    class Shape {
    public:
        class Builder {
        public:
            Builder() = default;
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            Builder& asCube();
            Builder& asPlane();
            Builder& asShpere(float radius, uint32_t division = 32);

            Shape build();

        private:
            GLsizei m_count;
            core::VertexBuffer m_vertices {};
        };

    public:
        Shape() = default;
        Shape(Shape&& right) noexcept;
        Shape& operator=(Shape&& right) noexcept;

        Shape(const Shape&) = delete;
        Shape& operator=(const Shape&) = delete;

        void draw();

    public:
        GLsizei count;
        core::VertexBuffer vertices {};
    };
}