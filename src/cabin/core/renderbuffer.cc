#include "renderbuffer.h"

namespace cabin::core {

    RenderBuffer::Builder& RenderBuffer::Builder::setFormat(GLenum format) {
        internalFormat = format;
        return *this;
    }

    RenderBuffer::Builder& RenderBuffer::Builder::setSize(GLsizei width, GLsizei height) {
        this->width = width;
        this->height = height;
        return *this;
    }

    RenderBuffer RenderBuffer::Builder::build() {
        GLuint id;
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
        
        return RenderBuffer { id, internalFormat, width, height };
    }

    RenderBuffer::RenderBuffer(GLuint id, GLenum format, GLsizei width, GLsizei height)
    : id(id), format(format), width(width), height(height) {}

    RenderBuffer::RenderBuffer(RenderBuffer&& right) noexcept {
        if (id.has_value()) {
            glDeleteRenderbuffers(1, &id.value());
        }
        id = right.id;
        right.id.reset();
    }

    RenderBuffer::~RenderBuffer() {
        if (id.has_value()) {
            glDeleteRenderbuffers(1, &id.value());
            id.reset();
        }
    }
}