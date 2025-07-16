#pragma once
#include <format>
#include <string>
#include <optional>
#include <stdexcept>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

namespace cabin::core {

    class Texture {
    public:
        using TextureType = GLenum;
        
        template <TextureType T>
            requires (T == GL_TEXTURE_2D || T == GL_TEXTURE_3D)
        class Builder {
        public:
            Builder() {
                glGenTextures(1, &id);
            }

            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            Builder& fromBuffer(const uint8_t* data, GLsizei width, GLsizei height, GLenum srcFormat, GLenum dstFormat) {
                static_assert(T == GL_TEXTURE_2D);
                
                this->width = width;
                this->height = height;
                this->format = dstFormat;

                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, dstFormat, width, height, 0, srcFormat, GL_UNSIGNED_BYTE, static_cast<const void*>(data));
                
                return *this;
            }
            Builder& fromBuffer(const uint8_t* data, GLsizei width, GLsizei height, GLsizei depth, GLenum srcFormat, GLenum dstFormat) {
                static_assert(T == GL_TEXTURE_3D);

                this->width = width;
                this->height = height;
                this->depth = depth;
                this->format = dstFormat;

                glBindTexture(GL_TEXTURE_3D, id);
                glTexImage3D(GL_TEXTURE_3D, 0, dstFormat, width, height, depth, 0, srcFormat, GL_UNSIGNED_INT, static_cast<const void*>(data));
                
                return *this;
            }

            Builder& fromFile(const std::string& path, bool flip = true) {
                static_assert(T == GL_TEXTURE_2D);

                if (flip)
                    stbi_set_flip_vertically_on_load(true);

                int width, height, channels;
                auto data = stbi_load(path.c_str(), &width, &height, &channels, 0);

                if (!data)
                    throw std::runtime_error(std::format("failed to open image: \"{}\"", path));

                GLenum format;
                if (channels == 3)
                    format = GL_RGB;
                else if (channels == 4)
                    format = GL_RGBA;
                else
                    throw std::runtime_error(std::format("not supported format in image: \"{}\"", path));
                
                this->width = width;
                this->height = height;
                this->format = format;
                
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, static_cast<const void*>(data));
                
                stbi_image_free(data);

                return *this;
            }

            Builder& setFilter(GLenum minify, GLenum magify) {
                glBindTexture(T, id);
                glTexParameteri(T, GL_TEXTURE_MIN_FILTER, minify);
                glTexParameteri(T, GL_TEXTURE_MAG_FILTER, magify);
                return *this;
            }

            Builder& genMipmap() {
                glBindTexture(T, id);
                glGenerateMipmap(T);
                return *this;
            }
            
            Builder& setWrap(GLenum s, GLenum t) {
                glBindTexture(T, id);
                glTextureParameteri(T, GL_TEXTURE_WRAP_S, s);
                glTextureParameteri(T, GL_TEXTURE_WRAP_T, t);
                return *this;
            }

            Builder& setWrap(GLenum s, GLenum t, GLenum r) {
                glBindTexture(T, id);
                glTextureParameteri(T, GL_TEXTURE_WRAP_S, s);
                glTextureParameteri(T, GL_TEXTURE_WRAP_T, t);
                glTextureParameteri(T, GL_TEXTURE_WRAP_R, r);
                return *this;
            }

            Builder& setBorderColor(const glm::vec4& color) {
                glBindTexture(T, id);
                glTextureParameterfv(T, GL_TEXTURE_BORDER_COLOR, &color[0]);
                return *this;
            }

            Texture build() {
                return Texture { id, T, format, width, height, depth };
            }

        private:
            GLuint id;
            GLenum format;
            GLsizei width, height, depth { 0 };
        };

    public:
        Texture(GLuint id, GLenum type, GLenum format, GLsizei width, GLsizei height, GLsizei depth);
        Texture(Texture&& right) noexcept;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        ~Texture();

        void active(GLuint index) const;

    public:
        std::optional<GLuint> id;
        GLenum type, format;
        GLsizei width, height, depth;
    };
}