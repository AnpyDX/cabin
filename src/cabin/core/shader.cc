#include "shader.h"

#include <regex>
#include <format>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include "cabin/utils/console.h"

namespace {
    std::string getSourceView(const std::string& src) {
        std::string result {};
        std::stringstream content;
        content << src;

        std::string line;
        std::vector<std::string> lines {};
        while (std::getline(content, line)) {
            lines.push_back(line);
        }

        int lineNum = static_cast<int>(lines.size());
        std::string maxSpacing {};
        while (lineNum) {
            lineNum /= 10;
            maxSpacing.append(" ");
        }

        for (int i = 0; i < lines.size(); i++) {
            int num = i + 1;
            std::string spacing = maxSpacing;
            while (num > 9) { num /= 10; spacing.erase(spacing.size() - 1, 1); }
            result.append(std::format("{}{}| {}\n", i + 1, spacing, lines[i]));
        }

        return result;
    }

    class ShaderProcesser {
    public:
        struct Result {
            std::string version {};
            std::string vertex {};
            std::string geometory {};
            std::string fragment {};
        };

    public:
        ShaderProcesser(const std::string& path) {
            std::ifstream sourceFile(path);
            if (!sourceFile.is_open())
                throw std::runtime_error("failed to open shader file.");

            m_sourceContent << sourceFile.rdbuf();
            m_baseDirectory = std::filesystem::path(path).parent_path().string();
            m_entryFileName = std::filesystem::path(path).filename().string();
            m_blockFileStack = { std::format("{}/{}", m_baseDirectory, m_entryFileName) };
        }

        Result process() {
            std::string versionStr = "#version ";
            std::optional<size_t> versionMarker {};

            std::string vertexBlock {}, geometoryBlock {}, fragmentBlock {};
            std::optional<size_t> vertexMarker {}, geometoryMarker {}, fragmentMarker {};  


            std::string line {};
            size_t lineNumber = 0;
            std::string* targetBlock = nullptr;

            try {
                while (std::getline(m_sourceContent, line)) {
                    lineNumber += 1;

                    if (isMacro(line)) {
                        auto [name, param] = idMacro(line);
                        
                        if (name.empty()) {
                            throw std::runtime_error("macro syntax error");
                        }
                        else if (name == "version") {
                            if (versionMarker.has_value())
                                throw std::runtime_error(std::format(
                                "version re-decleration (v.s. line {})", versionMarker.value()
                            ));
                            
                            if (param.empty())
                                throw std::runtime_error("macro \"version\" requires a version string as parameter. (helps: \"#![version(\"...\")]\")");

                            versionStr.append(param);
                            versionMarker = lineNumber;
                        }
                        else if (name == "vertex") {
                            if (vertexMarker.has_value())
                                throw std::runtime_error(std::format(
                                "vertex block re-decleration (v.s. line {})", vertexMarker.value()
                            ));

                            if (!param.empty())
                                throw std::runtime_error("extra \"vertex\" macro parameter. (helps: \"#![vertex]\")");
                            
                            targetBlock = &vertexBlock;
                            vertexMarker = lineNumber;
                        }
                        else if (name == "geometory") {
                            if (geometoryMarker.has_value())
                                throw std::runtime_error(std::format(
                                "geometory block re-decleration (v.s. line {})", geometoryMarker.value()
                            ));

                            if (!param.empty())
                                throw std::runtime_error("extra \"geometory\" macro parameter. (helps: \"#![geometory]\")");
                            
                            targetBlock = &geometoryBlock;
                            geometoryMarker = lineNumber;
                        }
                        else if (name == "fragment") {
                            if (fragmentMarker.has_value())
                                throw std::runtime_error(std::format(
                                "fragment block re-decleration (v.s. line {})", fragmentMarker.value()
                            ));

                            if (!param.empty())
                                throw std::runtime_error("extra \"fragment\" macro parameter. (helps: \"#![fragment]\")");
                            
                            targetBlock = &fragmentBlock;
                            fragmentMarker = lineNumber;
                        }
                        else {
                            if (!targetBlock)
                                throw std::runtime_error("out-block macro detected");

                            targetBlock->append(line);
                            targetBlock->append("\n");
                        }
                    }
                    else if (targetBlock) {
                        targetBlock->append(line);
                        targetBlock->append("\n");
                    }
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::format(
                    "failed to parse shader \"{}\", at line {}: \n    \"{}\"\n  error: {}.",
                    m_entryFileName, lineNumber, line, e.what()
                ));
            }
            
            Result processResult {};

            if (versionMarker.has_value())
                processResult.version = versionStr;
            else
                throw std::runtime_error("necessary \"version\" decleration is missing.");

            m_blockFileStack.erase(m_blockFileStack.begin() + 1, m_blockFileStack.end());
            if (vertexMarker.has_value())
                processResult.vertex = processBlock(vertexBlock, m_entryFileName, vertexMarker.value());
            else
                throw std::runtime_error("necessary \"vertex\" block is missing.");

            m_blockFileStack.erase(m_blockFileStack.begin() + 1, m_blockFileStack.end());
            if (geometoryMarker.has_value())
                processResult.geometory = processBlock(geometoryBlock, m_entryFileName, geometoryMarker.value());

            m_blockFileStack.erase(m_blockFileStack.begin() + 1, m_blockFileStack.end());
            if (fragmentMarker.has_value())
                processResult.fragment = processBlock(fragmentBlock, m_entryFileName, fragmentMarker.value());
            else
                throw std::runtime_error("necessary \"fragment\" block is missing.");

            return processResult;
        }

    private:
        std::string processBlock(const std::string& raw, const std::string& fileName, size_t lineOffset) {
            std::string result {};

            std::stringstream rawStream {};
            rawStream << raw;

            std::string line {};
            size_t lineNumber = lineOffset;

            try {
                while (std::getline(rawStream, line)) {
                    lineNumber += 1;

                    if (isMacro(line)) {
                        auto [name, param] = idMacro(line);
                        
                        if (name.empty()) {
                            throw std::runtime_error("macro syntax error");
                        }
                        else if (name == "use") {
                            if (param.empty())
                                throw std::runtime_error("macro \"use\" requires a path as parameter. (helps: \"#![use(\"...\")]\")");

                            std::string absolutePath = std::format("{}/{}", m_baseDirectory, param);
                            absolutePath = std::filesystem::absolute(absolutePath).string();

                            // Prevent self-use
                            if (absolutePath == std::filesystem::absolute(std::format("{}/{}", m_baseDirectory, fileName))) {
                                throw std::runtime_error("self-use detected");
                            }

                            // Prevent multi-use
                            if (std::find(m_blockFileStack.begin(), m_blockFileStack.end(), absolutePath) != m_blockFileStack.end()) {
                                cabin::utils::Console::info(std::format("muti-use file detected in \"{}\", line {}; ignored.", fileName, lineNumber));\
                                continue;
                            }

                            std::ifstream usedFile(absolutePath);
                            if (!usedFile.is_open())
                                throw std::runtime_error(std::format("failed to open used file: \"{}\"", param));

                            std::stringstream usedStream {};
                            usedStream << usedFile.rdbuf();

                            m_blockFileStack.push_back(absolutePath);
                            result.append(processBlock(usedStream.str(), param, 0));
                        }
                        else {
                            for (auto& macro : std::vector<std::string> { "version", "vertex", "geometory", "fragment" }) {
                                if (name == macro)
                                    throw std::runtime_error(std::format("macro \"{}\" only allowed in entry shader", macro));
                            }

                            throw std::runtime_error(std::format("unrecognized macro \"{}\"", name));
                        }
                    }
                    else {
                        result.append(line);
                        result.append("\n");
                    }
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(std::format(
                    "failed to parse shader block, in \"{}\", at line {}: \n    \"{}\"\n  error: {}",
                    fileName, lineNumber, line, e.what()
                ));
            }

            return result;
        }

        bool isMacro(const std::string& line) {
            return std::regex_match(line, m_macroRe);
        }

        std::pair<std::string, std::string> idMacro(const std::string& line) {
            std::smatch matchRes {};

            if (std::regex_match(line, matchRes, m_hintMacroRe)) {
                return std::make_pair(matchRes[1], "");
            }
            else if (std::regex_match(line, matchRes, m_declMacroRe)) {
                return std::make_pair(matchRes[1], matchRes[2]);
            }
            else {
                return std::make_pair("", "");
            }
        }

    private:
        std::stringstream m_sourceContent {};
        std::string m_baseDirectory {};
        std::string m_entryFileName {};

        std::vector<std::string> m_blockFileStack {};

        std::regex m_macroRe { R"(^\s*#![\S\s]*$)" }; 
        std::regex m_hintMacroRe { R"(^\s*#!\[\s*(\w+)\s*\]$)" };
        std::regex m_declMacroRe { R"(^\s*#!\[\s*(\w+)\s*\(\s*\"([\w\s\.\\//]*)\"\s*\)\s*\]$)"};
    };
}

namespace cabin::core {

    Shader::Builder& Shader::Builder::fromFile(const std::string& path) {
        utils::Console::info(std::format("loading shader \"{}\"", path));

        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("shader file doesn't exist");
        }

        m_filePath = path;
        return *this;
    }

    Shader Shader::Builder::build() {
        // 1. Parse source into different stages
        std::string version, vertex, geometory, fragment;

        ShaderProcesser::Result processResult {};
        processResult = ShaderProcesser(m_filePath).process();
        
        version.swap(processResult.version);
        vertex.swap(processResult.vertex);
        geometory.swap(processResult.geometory);
        fragment.swap(processResult.fragment);
        version.append("\n");
        
        // 2. Create shader program object
        auto checkCompileStatus = [&](GLuint id, const char* stage, const std::string& source) {
            int isSuccess;
            char statusLog[2048];
            
            glGetShaderiv(id, GL_COMPILE_STATUS, &isSuccess);
            if (!isSuccess) {
                glGetShaderInfoLog(id, 2048, nullptr, statusLog);
                throw std::runtime_error(
                    std::format(
                        "failed to compile shader's {} block!\n{}\n{}",
                        stage, getSourceView(source), statusLog
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
        checkCompileStatus(vertexShader, "vertex", vertexSrc);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrcPtr, nullptr);
        glCompileShader(fragmentShader);
        checkCompileStatus(fragmentShader, "fragment", fragmentSrc);

        if (geometory.size()) {
            std::string geometorySrc = version + geometory;
            const char* geometorySrcPtr = geometorySrc.c_str();
            
            geometoryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometoryShader, 1, &geometorySrcPtr, nullptr);
            glCompileShader(geometoryShader);
            checkCompileStatus(geometoryShader, "geometory", geometorySrc);
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
            
            std::string vertexSrcView = "\n>>> Vertex Shader Source <<<\n" + getSourceView(vertexSrc);
            std::string fragSrcView = "\n>>> Fragment Shader Source <<<\n" + getSourceView(fragmentSrc);
            std::string geometorySrcView {};
            if (!geometory.empty())
                geometorySrcView = "\n>>> Geometory Shader Source <<<\n" + getSourceView(version + geometory);

            throw std::runtime_error(
                std::format(
                    "failed to link shader!\n{}{}{}\n{}", 
                    vertexSrcView, geometorySrcView, fragSrcView, statusLog
                )
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
        if (id.has_value()) {
            glDeleteProgram(id.value());
        }
        id = right.id;
        right.id.reset();
    }
    
    Shader& Shader::operator=(Shader&& right) noexcept {
        if (id.has_value()) {
            glDeleteProgram(id.value());
        }
        id = right.id;
        right.id.reset();

        return *this;
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