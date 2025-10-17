#pragma once

#include <vector>
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

            /** Create the shape as a cube.
             * @note Default parameters:
             *  - The size is 2 x 2 x 2.
             *  - Center at (0, 0, 0).
             */
            Builder& asCube();

            /** Create the shape as a square.
             * @note Default parameters:
             *  - The size is 2 x 2.
             *  - Center at (0, 0, 0).
             *  - At z = 0 plane.
             */
            Builder& asSquare();

            /** Create the shape as a plane.
             * @note Default parameters:
             *  - Center at (0, 0, 0).
             *
             *  @param size     The size of plane.
             *  @param division The number of divided parts.
             *
             *  @note To make sure the divided part is a square,
             *        `division.x * size.y == division.y * size.x`
             *        should be guaranteed.
             */
            Builder& asPlane(glm::vec2 size, glm::vec<2, int> division);

            /** Create the shape as a plane.
             * @note Default parameters:
             * - Ceneter at (0, 0, 0).
             * 
             * @param radius   The radius of sphere.
             * @param division The division level, or the number of divided parts.
             */
            Builder& asShpere(float radius, uint32_t division = 32);

            Shape build();

        private:
            core::VertexBuffer m_vertices {};
            std::vector<unsigned int> m_indices {};
        };

    public:
        Shape() = default;
        Shape(Shape&& right) noexcept;
        Shape& operator=(Shape&& right) noexcept;

        Shape(const Shape&) = delete;
        Shape& operator=(const Shape&) = delete;

        void draw();

    public:
        core::VertexBuffer vertices {};
        std::vector<unsigned int> indices {};
    };
}