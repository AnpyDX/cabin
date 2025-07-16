#pragma once
#include <optional>
#include <glad/glad.h>

namespace cabin::core {
    class FrameBuffer {
    public:
        FrameBuffer();
        FrameBuffer(GLuint id);
        FrameBuffer(FrameBuffer&& right) noexcept;

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        ~FrameBuffer();

        void attachTexture(GLenum target, GLuint id);
        void attachRenderbuffer(GLenum target, GLuint id);

        void bind() const;
    public:
        std::optional<GLuint> id;
    };
}