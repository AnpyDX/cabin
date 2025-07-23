#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

namespace cabin::core {
    Texture::Texture(GLuint id, GLenum type, GLenum format, GLsizei width, GLsizei height, GLsizei depth)
    : id(id), type(type), format(format), width(width), height(height), depth(depth) {}

    Texture::Texture(Texture&& right) noexcept {
        if (id.has_value()) {
            glDeleteTextures(1, &id.value());
        }

        id = right.id;
        type = right.type;
        width = right.width;
        height = right.height;
        depth = right.depth;

        right.id.reset();
    }

    Texture& Texture::operator=(Texture&& right) noexcept {
        if (id.has_value()) {
            glDeleteTextures(1, &id.value());
        }

        id = right.id;
        type = right.type;
        width = right.width;
        height = right.height;
        depth = right.depth;
        right.id.reset();
        
        return *this;
    }

    Texture::~Texture() {
        if (id.has_value()) {
            glDeleteTextures(1, &id.value());
            id.reset();
        }
    }

    void Texture::active(GLuint index) const {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(type, id.value());
    }
}