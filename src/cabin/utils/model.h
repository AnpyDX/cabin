#pragma once
#include <map>
#include <string>
#include <vector>
#include <optional>

#include <glm/glm.hpp>

#include "cabin/core/shader.h"
#include "cabin/core/texture.h"
#include "cabin/core/vertexbuffer.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <tiny_gltf.h>

namespace cabin::utils {
    class Model {
    public:
        struct alignas(4) Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
        };

        struct Material {
            std::optional<size_t> baseColorTexture         {};
            std::optional<size_t> metallicRoughnessTexture {};
            std::optional<size_t> normalTexture            {};
            std::optional<size_t> emissiveTexture          {};
            std::optional<size_t> occlusionTexture         {};

            std::optional<glm::vec4> baseColorFactor {};
            std::optional<float> metallicFactor      {};
            std::optional<float> roughnessFactor     {};
            std::optional<glm::vec3> emissiveFactor  {};
        };

        struct Primitive {
            Material material {};
            core::VertexBuffer vertices {};
            std::vector<unsigned int> indices {};
        };

        using Mesh = std::vector<Primitive>;

    public:
        class Builder {
        public:
            Builder() = default;
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            Builder& fromGLB(const std::string& path);
            Builder& fromGLTF(const std::string& path);

            Model build();

        private:
            void loadModel();
            void loadNode(const tinygltf::Node& node);
            void loadMesh(const tinygltf::Mesh& mesh, const glm::mat4& transform);
            size_t loadTexture(int textureIndex);

        private:
            tinygltf::Model m_model {};
            std::vector<Mesh> m_meshes {};
            std::vector<core::Texture> m_textures {};
            std::map<size_t, size_t> m_loadedTextures {};
        };

    public:
        Model() = default;
        Model(Model&& right) noexcept;
        Model& operator=(Model&& right) noexcept;
        Model(const Model& right) = delete;
        Model& operator=(const Model& right) = delete;

        void draw(const core::Shader& shader) const;

    public:
        std::vector<Mesh> meshes {};
        std::vector<core::Texture> textures {};
    };
}