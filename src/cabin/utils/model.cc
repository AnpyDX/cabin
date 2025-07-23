#include "model.h"

#include <optional>
#include <stdexcept>
#include <filesystem>

#include <assimp/material.h>
#include <assimp/postprocess.h>

#include "console.h"


namespace cabin::utils {
    Model::Builder& Model::Builder::fromFile(const std::string& path) {

        Console::info("loading model: " + path);
        
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error(std::format("failed to load model file!"));
        }

        basePath = std::filesystem::path(path).parent_path().string();

        return *this;
    }

    Model Model::Builder::build() {
        processNode(scene->mRootNode);
        
        Model result {};
        result.meshes = std::move(meshes);
        result.textures = std::move(textures);

        return result;
    }

    void Model::Builder::processNode(aiNode* node) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i]);
        }
    }

    void Model::Builder::processMesh(aiMesh* mesh) {
        Mesh result {};

        /* Vertex and Index Buffers */
        std::vector<Vertex> vertices {};
        std::vector<unsigned int> indicies {};

        vertices.resize(mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {

            vertices[i].position.x = mesh->mVertices[i].x;
            vertices[i].position.y = mesh->mVertices[i].y;
            vertices[i].position.z = mesh->mVertices[i].z;

            vertices[i].normal.x = mesh->mNormals[i].x;
            vertices[i].normal.y = mesh->mNormals[i].y;
            vertices[i].normal.z = mesh->mNormals[i].z;

            vertices[i].tangent.x = mesh->mTangents[i].x;
            vertices[i].tangent.y = mesh->mTangents[i].y;
            vertices[i].tangent.z = mesh->mTangents[i].z;

            if (mesh->mTextureCoords[0]) {
                vertices[i].texCoord.x = mesh->mTextureCoords[0][i].x;
                vertices[i].texCoord.y = mesh->mTextureCoords[0][i].y;
            }
            else
                vertices[i].texCoord = { 0.0f, 0.0f };
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indicies.push_back(face.mIndices[j]);
            }
        }

        result.vertices = core::VertexBuffer::Builder()
                                    .setBuffer(reinterpret_cast<void*>(vertices.data()), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW)
                                    .addAttribute<float>(0, 3)
                                    .addAttribute<float>(1, 3)
                                    .addAttribute<float>(2, 3)
                                    .addAttribute<float>(3, 2)
                                    .build();

        result.indices = core::IndexBuffer::Builder()
                                    .setBuffer(indicies.data(), indicies.size(), GL_STATIC_DRAW)
                                    .build();

        /* Materials */
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        result.material.baseColorTexture = loadTexture(material, aiTextureType_BASE_COLOR);
        result.material.metallicRoughnessTexture = loadTexture(material, aiTextureType_GLTF_METALLIC_ROUGHNESS);
        result.material.normalTexture = loadTexture(material, aiTextureType_NORMALS);
        result.material.emissiveTexture = loadTexture(material, aiTextureType_EMISSIVE);
        result.material.occulsionTexture = loadTexture(material, aiTextureType_AMBIENT_OCCLUSION);

        if (!result.material.baseColorTexture.has_value())
            result.material.baseColorTexture = loadTexture(material, aiTextureType_DIFFUSE);
        if (!result.material.occulsionTexture.has_value())
            result.material.occulsionTexture = loadTexture(material, aiTextureType_LIGHTMAP);

        meshes.emplace_back(std::move(result));
    }

    std::optional<size_t> Model::Builder::loadTexture(aiMaterial* material, aiTextureType type) {
        if (material->GetTextureCount(type) == 0)
            return {};
        
        aiString assimpTexName;
        material->GetTexture(type, 0, &assimpTexName);
        std::string texName { assimpTexName.C_Str() };

        auto index = loadedTextures.find(texName);
        if (index == loadedTextures.end()) {
            auto texture = core::Texture::Builder<core::Texture::Tex2D>()
                                        .fromFile(std::format("{}/{}", basePath, texName))
                                        .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                                        .setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)
                                        .genMipmap()
                                        .build();
            textures.emplace_back(std::move(texture));
            loadedTextures[texName] = textures.size() - 1;
            return std::make_optional(textures.size() - 1);
        }
        
        return std::make_optional(index->second);
    }

    Model::Model(Model&& right) noexcept {
        meshes.swap(right.meshes);
        textures.swap(right.textures);
    }

    Model& Model::operator=(Model&& right) noexcept {
        meshes.swap(right.meshes);
        textures.swap(right.textures);
        return *this;
    }

    void Model::draw(core::Shader& shader) {
        for (auto& mesh : meshes) {
            shader.bind();

            if (mesh.material.baseColorTexture.has_value()) {
                textures[mesh.material.baseColorTexture.value()].active(0);
                shader.setInt("baseColorTexture", 0);
            }

            if (mesh.material.metallicRoughnessTexture.has_value()) {
                textures[mesh.material.metallicRoughnessTexture.value()].active(1);
                shader.setInt("metallicRoughnessTexture", 1);
            }

            if (mesh.material.normalTexture.has_value()) {
                textures[mesh.material.normalTexture.value()].active(2);
                shader.setInt("normalTexture", 2);
            }

            if (mesh.material.emissiveTexture.has_value()) {
                textures[mesh.material.emissiveTexture.value()].active(3);
                shader.setInt("emissiveTexture", 3);
            }

            if (mesh.material.occulsionTexture.has_value()) {
                textures[mesh.material.occulsionTexture.value()].active(4);
                shader.setInt("occulsionTexture", 4);
            }
            
            mesh.vertices.bind();
            glDrawElements(GL_TRIANGLES, mesh.indices.count, GL_UNSIGNED_INT, nullptr);
        }
    }
}