#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include <format>
#include <stdexcept>

namespace cabin::core {
    Texture::Builder::Builder() {
        glGenTextures(1, &id);
    }

    Texture::Builder& Texture::Builder::asEmpty2D(GLsizei width, GLsizei height, GLenum internalFormat) {
        target = GL_TEXTURE_2D;
        format = internalFormat;
        this->width = width;
        this->height = height;
        
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

        return *this;
    }

    Texture::Builder& Texture::Builder::asEmpty3D(GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat) {
        target = GL_TEXTURE_3D;
        format = internalFormat;
        this->width = width;
        this->height = height;
        this->depth = depth;

        glBindTexture(GL_TEXTURE_3D, id);
        glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, GL_RGB, GL_FLOAT, nullptr);

        return *this;
    }

    Texture::Builder& Texture::Builder::asEmptyCubeMap(GLsizei length, GLenum internalFormat) {
        target = GL_TEXTURE_CUBE_MAP;
        format = internalFormat;
        this->width = length;
        this->height = length;

        glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        for (int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
                         length, length, 0, GL_RGB, GL_FLOAT, nullptr);
        }

        return *this;
    }

    Texture::Builder& Texture::Builder::fromFile2D(const std::string& path, GLenum internalFormat, bool flip) {
        void* data;
        GLenum compType;
        int width, height, nrChannals;

        stbi_set_flip_vertically_on_load(flip);

        if (path.ends_with(".hdr")) {
            compType = GL_FLOAT;
            data = stbi_loadf(path.c_str(), &width, &height, &nrChannals, 0);
        } else {
            compType = GL_UNSIGNED_BYTE;
            data = stbi_load(path.c_str(), &width, &height, &nrChannals, 0);
        }

        if (!data)
            throw std::runtime_error(std::format("failed to open image: {}", path));

        GLenum srcFormat;
        if (nrChannals == 3)
            srcFormat = GL_RGB;
        else if (nrChannals == 4)
            srcFormat = GL_RGBA;
        else
            throw std::runtime_error(std::format("image owns an unsupported format: \"{}\"", path));
        
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, srcFormat, compType, data);

        stbi_image_free(data);

        target = GL_TEXTURE_2D;
        format = internalFormat;
        this->width = width;
        this->height = height;

        return *this;
    }

    Texture::Builder& Texture::Builder::setFilter(GLenum minify, GLenum magnify) {
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, minify);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, magnify);
        return *this;
    }

    Texture::Builder& Texture::Builder::genMipmap() {
        glBindTexture(target, id);
        glGenerateMipmap(target);
        return *this;
    }

    Texture::Builder& Texture::Builder::setWrap(GLenum s, GLenum t) {
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, s);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, t);
        return *this;
    }

    Texture::Builder& Texture::Builder::setWrap(GLenum s, GLenum t, GLenum r) {
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, s);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, t);
        glTextureParameteri(id, GL_TEXTURE_WRAP_R, r);
        return *this;
    }

    Texture::Builder& Texture::Builder::setBorderColor(const glm::vec4& color) {
        glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, &color[0]);
        return *this;
    }

    Texture Texture::Builder::build() {
        return Texture { id, target, format, width, height, depth };
    }

    Texture::Texture(GLuint id, GLenum target, GLenum format, GLsizei width, GLsizei height, GLsizei depth)
    : id(id), target(target), format(format), width(width), height(height), depth(depth) {}

    Texture::Texture(Texture&& right) noexcept {
        if (id.has_value()) {
            glDeleteTextures(1, &id.value());
        }

        id = right.id;
        target = right.target;
        format = right.format;
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
        target = right.target;
        format = right.format;
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
        glBindTexture(target, id.value());
    }
}