#include "vertexbuffer.h"

#include <stdexcept>

namespace cabin::core {
    VertexBuffer::Builder::Builder() {
        glGenBuffers(1, &vertexBufferID);
        glGenVertexArrays(1, &vertexArrayID);
    }

    VertexBuffer::Builder& VertexBuffer::Builder::setBuffer(const void* data, GLsizeiptr size, GLenum usage) {
        glBindVertexArray(vertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);

        return *this;
    }

    VertexBuffer VertexBuffer::Builder::build() {
        if (attributes.empty())
            throw std::runtime_error("failed to build VertexBuffer without any attribute!");

        GLuint strideSize = 0;
        size_t offsetRecord = 0;

        for (auto& attr : attributes) {
            strideSize += attr.storageSize;
        }

        glBindVertexArray(vertexArrayID);
        for (auto& attr : attributes) {
            glVertexAttribPointer(attr.index, attr.count, attr.storageType, attr.normalized, strideSize, reinterpret_cast<void*>(offsetRecord));
            glEnableVertexAttribArray(attr.index);
            offsetRecord += attr.storageSize;
        }

        return VertexBuffer { vertexBufferID, vertexArrayID };
    }

    VertexBuffer::VertexBuffer(GLuint VBO, GLuint VAO)
    : VBO(VBO), VAO(VAO) {}

    VertexBuffer::VertexBuffer(VertexBuffer&& right) noexcept {
        if (VAO.has_value() && VBO.has_value()) {
            glDeleteVertexArrays(1, &VAO.value());
            glDeleteBuffers(1, &VBO.value());
        }
        
        VAO = right.VAO;
        VBO = right.VBO;
        right.VAO.reset();
        right.VBO.reset();
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& right) noexcept {
        if (VAO.has_value() && VBO.has_value()) {
            glDeleteVertexArrays(1, &VAO.value());
            glDeleteBuffers(1, &VBO.value());
        }

        VAO = right.VAO;
        VBO = right.VBO;
        right.VAO.reset();
        right.VBO.reset();

        return *this;
    }

    VertexBuffer::~VertexBuffer() {
        if (VAO.has_value()) {
            glDeleteVertexArrays(1, &VAO.value());
            VAO.reset();
        }

        if (VBO.has_value()) {
            glDeleteBuffers(1, &VBO.value());
            VBO.reset();
        }
    }

    void VertexBuffer::bind() const {
        glBindVertexArray(VAO.value());
    }
}