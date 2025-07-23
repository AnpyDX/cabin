#pragma once
#include <map>
#include <string>
#include <vector>
#include <optional>

#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "../core/shader.h"
#include "../core/texture.h"
#include "../core/indexbuffer.h"
#include "../core/vertexbuffer.h"

namespace cabin::utils {
    class Model {
    public:
        struct alignas(4) Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec2 texCoord;
        };

        struct Mesh {
            core::IndexBuffer indices;
            core::VertexBuffer vertices;

            struct Material {
                std::optional<size_t> baseColorTexture;
                std::optional<size_t> metallicRoughnessTexture;
                std::optional<size_t> normalTexture;
                std::optional<size_t> emissiveTexture;
                std::optional<size_t> occulsionTexture;
            } material;
        };

        class Builder {
        public:
            Builder() = default;
            Builder(Builder&&) = delete;
            Builder(const Builder&) = delete;

            Builder& fromFile(const std::string& path);

            Model build();

        private:
            void processNode(aiNode* node);
            void processMesh(aiMesh* mesh);
            std::optional<size_t> loadTexture(aiMaterial* material, aiTextureType type);

        private:
            std::string basePath;
            Assimp::Importer importer;
            const aiScene* scene;

            std::vector<Mesh> meshes {};
            std::vector<core::Texture> textures {};
            std::map<std::string, size_t> loadedTextures {};
        };

    public:
        Model() = default;
        Model(Model&& right) noexcept;
        Model& operator=(Model&& right) noexcept;

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void draw(core::Shader& shader);

    public:
        std::vector<Mesh> meshes {};
        std::vector<core::Texture> textures {};
    };
}