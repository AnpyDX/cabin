/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <optional>
#include <glad/glad.h>

namespace cabin::core {

    class RenderBuffer {
    public:
        class Builder {
        public:
            Builder() = default;
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            //! Set the renderbuffer format.
            Builder& setFormat(GLenum format);

            /** Set the size of renderbuffer.
             * 
             * @note The size of renderbuffer is usually 
             *       the same as that of the viewport.
             */
            Builder& setSize(GLsizei width, GLsizei height);

            RenderBuffer build();

        private:
            GLenum internalFormat;
            GLsizei width, height;
        };

    public:
        RenderBuffer(GLuint id, GLenum format, GLsizei width, GLsizei height);
        RenderBuffer(RenderBuffer&& right) noexcept;
        RenderBuffer(const RenderBuffer&) = delete;
        RenderBuffer& operator=(const RenderBuffer&) = delete;

        ~RenderBuffer();

    public:
        std::optional<GLuint> id;
        GLenum format;
        GLsizei width, height;
    };
}