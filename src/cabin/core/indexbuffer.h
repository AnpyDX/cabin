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

            /**
             * @brief Allocate index buffer and set buffer data.
             * 
             * @tparam T Index storage type.
             * @param data Pointer to indices data.
             * @param count Indices number.
             * @param usage Buffer usage.
             */
            template <typename T>
                requires std::is_same_v<T, unsigned int> || std::is_same_v<T, int>
            Builder& setBuffer(T* data, GLsizei count, GLenum usage) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(T) * count, static_cast<void*>(data), usage);

                if constexpr (std::is_same_v<T, unsigned int>)
                    storageType = GL_UNSIGNED_INT;
                else
                    storageType = GL_INT;

                return *this;
            }

            IndexBuffer build();

        private:
            GLuint EBO;
            GLsizei count;
            GLenum storageType;
        };

    public:
        IndexBuffer(GLuint id, GLsizei count, GLenum storageType);
        IndexBuffer(IndexBuffer&& right) noexcept;
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        ~IndexBuffer();

        /**
         * @brief Bind to the index array. (wrapper of `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)`)
         */
        void bind() const;

    public:
        std::optional<GLuint> id;
        GLsizei count;
        GLenum storageType;
    };
}