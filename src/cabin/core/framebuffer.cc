#include "framebuffer.h"

namespace cabin::core {
    FrameBuffer::FrameBuffer() {
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        id = fbo;
    }

    FrameBuffer::FrameBuffer(GLuint id)
    : id(id) {}

    FrameBuffer::FrameBuffer(FrameBuffer&& right) noexcept{
        if (id.has_value()) {
            glDeleteFramebuffers(1, &id.value());
        }
        id = right.id;
        right.id.reset();
    }

    FrameBuffer::~FrameBuffer() {
        if (id.has_value()) {
            glDeleteFramebuffers(1, &id.value());
            id.reset();
        }
    }

    void FrameBuffer::attachTexture(GLenum target, GLuint id) {
        glBindFramebuffer(GL_FRAMEBUFFER, this->id.value());
        glFramebufferTexture(GL_FRAMEBUFFER, target, id, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::attachRenderbuffer(GLenum target, GLuint id) {
        glBindFramebuffer(GL_FRAMEBUFFER, this->id.value());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, id);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, id.value());
    }
}