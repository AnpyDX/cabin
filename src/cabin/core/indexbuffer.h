/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <optional>
#include <type_traits>
#include <glad/glad.h>

namespace cabin::core {

    class IndexBuffer {
    public:
        class Builder {
        public:
            Builder();
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            /** Allocate index buffer and set buffer data.
             * 
             * @tparam T    Index storage type.
             *              Only accept `unsigned int` or `int`.
             *
             * @param data  Pointer to indices data buffer.
             * @param count The number of indices.
             * @param usage The usage of buffer (e.g. `GL_STATIC_DRAW`).
             */
            template <typename T>
                requires std::is_same_v<T, unsigned int> || std::is_same_v<T, int>
            Builder& setBuffer(T* data, GLsizeiptr count, GLenum usage) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(T) * count, static_cast<void*>(data), usage);

                if constexpr (std::is_same_v<T, unsigned int>)
                    storageType = GL_UNSIGNED_INT;
                else
                    storageType = GL_INT;

                return *this;
            }

            IndexBuffer build();

        private:
            GLuint id;
            GLsizei count;
            GLenum storageType;
        };

    public:
        IndexBuffer(GLuint id, GLsizei count, GLenum storageType);
        IndexBuffer(IndexBuffer&& right) noexcept;
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        ~IndexBuffer();

        //! Bind to this index buffer. (wrapper of `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)`)
        void bind() const;

    public:
        std::optional<GLuint> id;
        GLsizei count;
        GLenum storageType;
    };
}