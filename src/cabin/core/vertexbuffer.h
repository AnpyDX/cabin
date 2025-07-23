/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <vector>
#include <optional>

#include <glad/glad.h>

namespace cabin::core {
    
    class VertexBuffer {
    public:
        class Builder {
        private:
            struct AttributeInfo {
                GLuint index;
                GLuint count;
                GLuint storageSize;
                GLenum storageType;
                bool normalized;
            };

        public:
            Builder();
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            /** Allocate vertex buffer and set buffer data.
             * 
             * @param data  Pointer to vertices data.
             * @param size  Size of vertices data (in byte).
             * @param usage Buffer usage.
             */
            Builder& setBuffer(const void* data, GLsizeiptr size, GLenum usage);

            /** Add a vertex array attribute.
             * 
             * @tparam T         Attribute component type.
             * @param index      Attribute location index.
             * @param count      Attribute component count.
             * @param normalized Whether normalize attribute components.
             */
            template <typename T>
                requires std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>
            Builder& addAttribute(GLuint index, GLuint count, bool normalized = false) {
                AttributeInfo attrInfo {};
                attrInfo.index = index;
                attrInfo.count = count;
                attrInfo.normalized = normalized;
                attrInfo.storageSize = sizeof(T) * count;
                
                if constexpr (std::is_same_v<T, int>)
                    attrInfo.storageType = GL_INT;
                else if constexpr (std::is_same_v<T, float>)
                    attrInfo.storageType = GL_FLOAT;
                else if constexpr (std::is_same_v<T, double>)
                    attrInfo.storageType = GL_DOUBLE;

                attributes.push_back(attrInfo);
                
                return *this;
            }

            VertexBuffer build();

        private:
            GLuint vertexBufferID, vertexArrayID;
            std::vector<AttributeInfo> attributes {};
        };

    public:
        VertexBuffer() = default;
        VertexBuffer(GLuint VBO, GLuint VAO);

        VertexBuffer(VertexBuffer&& right) noexcept;
        VertexBuffer& operator=(VertexBuffer&& right) noexcept;

        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        ~VertexBuffer();

        //! Bind to this vertex buffer. (wrapper of `glBindVertexArray`)
        void bind() const;

    public:
        std::optional<GLuint> VBO, VAO;
    };
}