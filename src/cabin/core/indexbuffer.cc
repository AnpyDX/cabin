#include "indexbuffer.h"

namespace cabin::core {
    IndexBuffer::Builder::Builder() {
        glGenBuffers(1, &id);
    }

    IndexBuffer IndexBuffer::Builder::build() {
        return IndexBuffer { id, count, componentType };
    }

    IndexBuffer::IndexBuffer(GLuint id, GLsizei count, GLenum componentType)
    : id(id), count(count), componentType(componentType) {}

    IndexBuffer::IndexBuffer(IndexBuffer&& right) noexcept {
        if (id.has_value()) {
            glDeleteBuffers(1, &id.value());
        }

        id = right.id;
        count = right.count;
        componentType = right.componentType;
        right.id.reset();
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& right) noexcept {
        if (id.has_value()) {
            glDeleteBuffers(1, &id.value());
        }

        id = right.id;
        count = right.count;
        componentType = right.componentType;
        right.id.reset();
        
        return *this;
    }

    IndexBuffer::~IndexBuffer() {
        if (id.has_value()) {
            glDeleteBuffers(1, &id.value());
            id.reset();
        }
    }

    void IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id.value());
    }
}