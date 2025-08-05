/**
 * cabin-framework (https://github.com/anpydx/cabin)
 *
 * Copyright (c) 2025 anpyd, All Rights Reserved.
 * Licensed under the MIT License.
 */

#pragma once
#include <string>
#include <sstream>
#include <optional>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace cabin::core {

    class Shader {
    public:
        class Builder {
        public:
            Builder() = default;
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            /** Create the shader from source file.
             * 
             * @param path Path of shader source file.
             */
            Builder& fromFile(const std::string& path);

            Shader build();

        private:
            GLuint id;
            std::string m_filePath {};
        };

    public:
        Shader() = default;
        Shader(GLuint id);
        Shader(Shader&& right) noexcept;
        Shader& operator=(Shader&& right) noexcept;
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        ~Shader();

        //! Bind to this shader program. (wrapper of `glBindShaderProgram`)
        void bind() const;

        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec2(const std::string& name, glm::vec2 value) const;
        void setVec3(const std::string& name, glm::vec3 value) const;
        void setVec4(const std::string& name, glm::vec4 value) const;
        void setMat2(const std::string& name, glm::mat2 value) const;
        void setMat3(const std::string& name, glm::mat3 value) const;
        void setMat4(const std::string& name, glm::mat4 value) const;
    
    public:
        std::optional<GLuint> id;
    };
}