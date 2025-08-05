/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <vector>
#include <string>
#include <optional>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb/stb_image.h>

namespace cabin::core {

    class Texture {
    public:
        class Builder {
        public:
            Builder();

            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            Builder& asEmpty2D(GLsizei width, GLsizei height, GLenum internalFormat);
            Builder& asEmpty3D(GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat);
            Builder& asEmptyCubeMap(GLsizei length, GLenum internalFormat);

            Builder& fromFile2D(const std::string& path, GLenum internalFormat = GL_RGBA, bool flip = true);
            
            template <typename T>
                requires (std::is_same_v<T, unsigned char> || std::is_same_v<T, float>)
            Builder& fromBuffer2D(const T* data, GLsizei width, GLsizei height, GLenum srcFormat, GLenum internalFormat) {
                target = GL_TEXTURE_2D;
                format = internalFormat;
                this->width = width;
                this->height = height;

                GLenum compType;
                if constexpr (std::is_same_v<T, unsigned char>)
                    compType = GL_UNSIGNED_BYTE;
                else
                    compType = GL_FLOAT;

                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, srcFormat, compType, data);

                return *this;
            }

            template <typename T>
                requires (std::is_same_v<T, unsigned char> || std::is_same_v<T, float>)
            Builder& fromBuffer3D(const T* data, GLsizei width, GLsizei height, GLsizei depth, GLenum srcFormat, GLenum internalFormat) {
                target = GL_TEXTURE_3D;
                format = internalFormat;
                this->width = width;
                this->height = height;
                this->depth = depth;

                GLenum compType;
                if constexpr (std::is_same_v<T, unsigned char>)
                    compType = GL_UNSIGNED_BYTE;
                else
                    compType = GL_FLOAT;

                glBindTexture(GL_TEXTURE_3D, id);
                glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, srcFormat, compType, data);

                return *this;
            }

            /** Set the texture filtering way when minify and magify.
             *
             * @note `magify` should not be set to `GL_*_MIPMAP_*`, 
             *        since texture magnification doesn't use mipmaps.
             * 
             * @see https://learnopengl.com/Getting-started/Textures
             */
            Builder& setFilter(GLenum minify, GLenum magnify);

            //! Generate mipmap for texture.
            Builder& genMipmap();
            
            //! Set the wrapping way for Texture2D.
            Builder& setWrap(GLenum s, GLenum t);

            //! Set the wrapping way for the texture3D and Cubemap.
            Builder& setWrap(GLenum s, GLenum t, GLenum r);

            /** Set the border color of texture.
             *
             * @note Only work when wrapping way set to `GL_CLAMP_TO_BORDER`.
             */
            Builder& setBorderColor(const glm::vec4& color);

            Texture build();

        private:
            GLuint id {};
            GLenum target {};
            GLenum format {};
            GLsizei width {0}, height {0}, depth {0};
        };

    public:
        Texture() = default;
        Texture(GLuint id, GLenum target, GLenum format, GLsizei width, GLsizei height, GLsizei depth);

        Texture(Texture&& right) noexcept;
        Texture& operator=(Texture&&) noexcept;

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        ~Texture();

        

        /** Activate a texture unit, and assign current texture to it.
         * 
         * @param index The index of the texture unit to be activated.
         */
        void active(GLuint index) const;

    public:
        std::optional<GLuint> id;
        GLenum target, format;
        GLsizei width, height, depth;
    };
}