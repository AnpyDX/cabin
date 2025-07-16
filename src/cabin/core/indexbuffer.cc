#include "indexbuffer.h"

namespace cabin::core {
    IndexBuffer::Builder::Builder() {
        glGenBuffers(1, &EBO);
    }

    IndexBuffer IndexBuffer::Builder::build() {
        return IndexBuffer { EBO, count, storageType };
    }

    IndexBuffer::IndexBuffer(GLuint id, GLsizei count, GLenum storageType)
    : id(id), count(count), storageType(storageType) {}

    IndexBuffer::IndexBuffer(IndexBuffer&& right) noexcept {
        if (id.has_value()) {
            glDeleteBuffers(1, &id.value());
        }
        id = right.id;
        count = right.count;
        storageType = right.storageType;
        right.id.reset();
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