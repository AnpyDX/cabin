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
    class FrameBuffer {
    public:
        //! Construct an empty `FrameBuffer` object.
        FrameBuffer();

        //! Construct a `FrameBuffer` from an existing FBO.
        FrameBuffer(GLuint id);

        FrameBuffer(FrameBuffer&& right) noexcept;
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        ~FrameBuffer();

        /** Attach a texture to the framebuffer.
         * 
         * @param target The target attachment type.
         * @param id     Texture's object ID.
         */
        void attachTexture(GLenum target, GLuint id);

        /** Attach a renderbuffer to the framebuffer.
         * 
         * @param target The target attachment type.
         * @param id     The renderbuffer's object ID.
         */
        void attachRenderbuffer(GLenum target, GLuint id);

        //! Bind to this framebuffer object. (wrapper of `glBindFramebuffer`)
        void bind() const;
    public:
        std::optional<GLuint> id;
    };
}