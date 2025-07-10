#include "shader.h"

#include <format>
#include <fstream>
#include <stdexcept>
#include "console.h"

namespace {
    std::string trimStr(const std::string& str) {
        size_t begin = str.find_first_not_of(' ');
        if (begin == std::string::npos)
            return "";
        size_t end = str.find_last_not_of(' ');
        return str.substr(begin, end - begin + 1);
    }
}

namespace cabin::core {

    Shader::Builder& Shader::Builder::setSourceFile(const std::string& path) {
        utils::Console::info(std::format("loading shader \"{}\"", path));

        std::ifstream sourceFile { path };
        if (!sourceFile.is_open()) {
            throw std::runtime_error("failed to open shader file!");
        }

        source << sourceFile.rdbuf();
        sourceFile.close();

        return *this;
    }

    Shader::Builder& Shader::Builder::setSourceString(std::string&& str) {
        source = std::stringstream(str);
        return *this;
    }

    Shader::Builder& Shader::Builder::setSourceString(const std::string& str) {
        source = std::stringstream(str);
        return *this;
    }

    Shader Shader::Builder::build() {
        // 1. Parse source into different stages
        std::string version, vertex, geometory, fragment;

        size_t lineNumber = 0;
        std::string line {};
        std::string* target = nullptr;

        try {
            while (std::getline(source, line)) {
                lineNumber += 1;
                std::string trimed = trimStr(line);

                if (trimed.starts_with("#version")) {
                    if (version.size())
                        throw "version re-decleration";
                    version = trimed;
                    version.append("\n");
                }
                else if (trimed.starts_with("#!")) {
                    if (trimed == "#![vertex]") {
                        if (vertex.size())
                            throw "vertex block re-decleration";
                        target = &vertex;
                    }
                    else if (trimed == "#![geometory]") {
                        if (geometory.size())
                            throw "geometory block re-decleration";
                        target = &geometory;
                    }
                    else if (trimed == "#![fragment]") {
                        if (fragment.size())
                            throw "fragment block re-decleration";
                        target = &fragment;
                    }
                    else {
                        throw "unrecognized block";
                    }
                }
                else {
                    if (target) {
                        target->append(line);
                        target->append("\n");
                    }
                }
            }
        } catch (const char* errorInfo) {
            throw std::runtime_error(
                std::format(
                    "failed to parse shader at line {}: \n    \"{}\"\n  error: {}.", 
                    lineNumber, line, errorInfo
                )
            );
        }

        // 2. Check shader integrity
        try {
            if (!version.size())
                throw "missing necessary version decleration";
            if (!vertex.size())
                throw "missing necessary vertex block";
            if (!fragment.size())
                throw "missing necessary fragment block";
        } catch (const char* errorInfo) {
            throw std::runtime_error(
                std::format("incomplete shader: {}.", errorInfo)
            );
        }
        
        // 3. Create shader program object
        auto checkCompileStatus = [&](GLuint id, const char* stage) {
            int isSuccess;
            char statusLog[2048];
            
            glGetShaderiv(id, GL_COMPILE_STATUS, &isSuccess);
            if (!isSuccess) {
                glGetShaderInfoLog(id, 2048, nullptr, statusLog);
                throw std::runtime_error(
                    std::format(
                        "failed to compile shader's {} block!\n{}",
                        stage, statusLog
                    )
                );
            }
        };

        std::string vertexSrc = version + vertex;
        std::string fragmentSrc = version + fragment;

        const char* vertexSrcPtr = vertexSrc.c_str();
        const char* fragmentSrcPtr = fragmentSrc.c_str();

        GLuint vertexShader, fragmentShader, geometoryShader;

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrcPtr, nullptr);
        glCompileShader(vertexShader);
        checkCompileStatus(vertexShader, "vertex");

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrcPtr, nullptr);
        glCompileShader(fragmentShader);
        checkCompileStatus(fragmentShader, "fragment");

        if (geometory.size()) {
            std::string geometorySrc = version + geometory;
            const char* geometorySrcPtr = geometorySrc.c_str();
            
            geometoryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometoryShader, 1, &geometorySrcPtr, nullptr);
            glCompileShader(geometoryShader);
            checkCompileStatus(geometoryShader, "geometory");
        }

        id = glCreateProgram();
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);
        if (geometory.size())
            glAttachShader(id, geometoryShader);
        glLinkProgram(id);

        int isSuccess;
        char statusLog[2048];
        glGetProgramiv(id, GL_LINK_STATUS, &isSuccess);
        if (!isSuccess) {
            glGetProgramInfoLog(id, 2048, nullptr, statusLog);
            throw std::runtime_error(
                std::format("failed to link shader!\n{}", statusLog)
            );
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (geometory.size())
            glDeleteShader(geometoryShader);

        return Shader { id };
    }

    Shader::Shader(GLuint id)
    : id(id) {}

    Shader::~Shader() {
        if (id.has_value()) {
            glDeleteProgram(id.value());
            id.reset();
        }
    }

    Shader::Shader(Shader&& right) noexcept {
        this->id = right.id;
        right.id.reset();
    }

    void Shader::bind() const {
        glUseProgram(id.value());
    }

    void Shader::setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(id.value(), name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(id.value(), name.c_str()), value);
    }

    void Shader::setVec2(const std::string& name, glm::vec2 value) const {
        glUniform2fv(glGetUniformLocation(id.value(), name.c_str()), 1, &value[0]);
    }

    void Shader::setVec3(const std::string& name, glm::vec3 value) const {
        glUniform3fv(glGetUniformLocation(id.value(), name.c_str()), 1, &value[0]);
    }

    void Shader::setVec4(const std::string& name, glm::vec4 value) const {
        glUniform4fv(glGetUniformLocation(id.value(), name.c_str()), 1, &value[0]);
    }

    void Shader::setMat2(const std::string& name, glm::mat2 value) const {
        glUniformMatrix2fv(glGetUniformLocation(id.value(), name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::setMat3(const std::string& name, glm::mat3 value) const {
        glUniformMatrix3fv(glGetUniformLocation(id.value(), name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::setMat4(const std::string& name, glm::mat4 value) const {
        glUniformMatrix4fv(glGetUniformLocation(id.value(), name.c_str()), 1, GL_FALSE, &value[0][0]);
    }
}