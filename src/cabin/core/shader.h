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

            /**
             * @brief Set shader source file path.
             * 
             * @param path 
             */
            Builder& setSourceFile(const std::string& path);

            /**
             * @brief Set shader source string (move; string's ownership taken).
             * 
             * @param str Source code string.
             */
            Builder& setSourceString(std::string&& str);

            /**
             * @brief Set shader source string (copy).
             * 
             * @param str Source code string.
             */
            Builder& setSourceString(const std::string& str);

            Shader build();

        private:
            GLuint id;
            std::stringstream source {};
        };

    public:
        Shader(GLuint id);
        Shader(Shader&&) noexcept;
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        ~Shader();

        /**
         * @brief Bind to the shader program. (wrapper of `glBindShaderProgram`)
         */
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