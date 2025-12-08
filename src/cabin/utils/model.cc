#define TINYGLTF_IMPLEMENTATION
#include "model.h"
#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "cabin/utils/console.h"

namespace {
    template <typename T>
    constexpr void indexChecker(const std::vector<T>& container, int index) {
    #ifndef NDEBUG
        assert(container.size() > index && index >= 0);
    #endif
    }
}

namespace cabin::utils {
    Model::Builder::Builder() {
        m_loader.SetImageLoader([](
            tinygltf::Image* image, const int imageIndex,
            std::string* errorMsg, std::string* warnMsg,
            int requestWidth, int requestHeight,
            const unsigned char* bytes, int size, 
            void* userData
        ) -> bool {
            int bits = 8;
            int storageType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

            if (stbi_is_16_bit_from_memory(bytes, size)) {
                bits = 16;
                storageType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
            }

            uint8_t* data = nullptr;
            int width, height, channels;

            stbi_set_flip_vertically_on_load(false);

            if (bits == 16)
                data = reinterpret_cast<uint8_t*>(
                    stbi_load_16_from_memory(bytes, size, &width, &height, &channels, 0)
                );
            else
                data = stbi_load_from_memory(bytes, size, &width, &height, &channels, 0);

            if (!data) {
                errorMsg->append(std::format("stb_image failed to load: \"{}\"", image->name));
                return false;
            }

            if ((width < 1) || (height < 1)) {
                errorMsg->append(std::format("invalid size of image: \"{}\"", image->name));
                return false;
            }

            if ((requestWidth > 0) && (requestWidth != width)) {
                errorMsg->append(std::format("dismatched width of image: \"{}\"", image->name));
                return false;
            }

            if ((requestHeight > 0) && (requestHeight != height)) {
                errorMsg->append(std::format("dismatched height of image: \"{}\"", image->name));
                return false;
            }

            image->width = width;
            image->height = height;
            image->component = channels;
            image->bits = bits;
            image->pixel_type = storageType;
            image->as_is = false;

            image->image.resize(static_cast<size_t>(width * height * channels) * size_t(bits / 8));
            std::copy(data, data + image->image.size(), image->image.begin());

            return true;
        }, nullptr);
    }

    Model::Builder& Model::Builder::fromGLB(const std::string& path) {
        Console::info(std::format("loading glb model: \"{}\"", path));

        std::string loadError {}, loadWarn {};
        bool res = m_loader.LoadBinaryFromFile(&m_model, &loadError, &loadWarn, path);

        if (!loadWarn.empty())
            Console::info(std::format("warning: {}", loadWarn));
        if (!res)
            throw std::runtime_error(std::format("failed to load glb model: {}", loadError));

        loadModel();
        return *this;
    }

    Model::Builder& Model::Builder::fromGLTF(const std::string& path) {
        Console::info(std::format("loading glTF model: \"{}\"", path));

        std::string loadError {}, loadWarn {};
        bool res = m_loader.LoadASCIIFromFile(&m_model, &loadError, &loadWarn, path);

        if (!loadWarn.empty())
            Console::info(std::format("warning: {}", loadWarn));
        if (!res)
            throw std::runtime_error(std::format("failed to load glTF model: {}", loadError));

        loadModel();
        return *this;
    }

    Model Model::Builder::build() {
        Model result {};
        result.meshes.swap(m_meshes);
        result.textures.swap(m_textures);
        return result;
    }

    void Model::Builder::loadModel() {
        tinygltf::Scene& scene = m_model.scenes[m_model.defaultScene];
        for (auto& node : scene.nodes) {
            indexChecker(m_model.nodes, node);
            loadNode(m_model.nodes[node]);
        }
    }

    void Model::Builder::loadNode(const tinygltf::Node& node) {
        if (node.mesh >= 0) {
            indexChecker(m_model.meshes, node.mesh);

            glm::mat4 transform { 1.0f };
            if (!node.translation.empty() && node.translation.size() == 3) {
                transform = glm::translate(transform, glm::vec3 {
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2])
                });
            }
            if (!node.rotation.empty() && node.rotation.size() == 4) {
                glm::mat4 rotateMat = glm::mat4_cast(glm::quat {
                    static_cast<float>(node.rotation[3]),
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2])
                });
                transform = rotateMat * transform;
            }
            if (!node.scale.empty() && node.scale.size() == 3) {
                transform = glm::scale(transform, glm::vec3 {
                    static_cast<float>(node.scale[0]),
                    static_cast<float>(node.scale[1]),
                    static_cast<float>(node.scale[2])
                });
            }
            loadMesh(m_model.meshes[node.mesh], transform);
        }

        for (auto& child : node.children) {
            indexChecker(m_model.nodes, child);
            loadNode(m_model.nodes[child]);
        }
    }

    void Model::Builder::loadMesh(const tinygltf::Mesh& mesh, const glm::mat4& transform) {
        Mesh result (mesh.primitives.size());
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const std::string meshName = mesh.name.empty() ? "_" : mesh.name;
            
            const tinygltf::Primitive& primitive = mesh.primitives[i];

            // Check drawing mode
            if (primitive.mode != GL_TRIANGLES)
                throw std::runtime_error(std::format(
                                "unspported topology type in mesh \"{}\", which is not GL_TRIANGLES", meshName
                            ));

            /* Attributes */
            static const char* requireAttributes[] = {
                "POSITION", "NORMAL", "TEXCOORD_0"
            };
            for (int j = 0; j < 3; j++) {
                if (primitive.attributes.find(requireAttributes[j]) == primitive.attributes.end())
                    throw std::runtime_error(
                            std::format("found incomplete primitive, whose \"{}\" attribute is missing", requireAttributes[j])
                        );
            }

            auto fetchBuffer = [&](int accessorIndex, int type, int comp, size_t count) -> std::pair<uint8_t*, size_t> {
                indexChecker(m_model.accessors, accessorIndex);
                tinygltf::Accessor& accessor = m_model.accessors[accessorIndex];
                tinygltf::BufferView& bufferView = m_model.bufferViews[accessor.bufferView];
                tinygltf::Buffer& buffer = m_model.buffers[bufferView.buffer];

                if (accessor.type != type || accessor.componentType != comp || accessor.count != count)
                    return { nullptr, 0 };

                size_t stride = bufferView.byteStride;
                if (stride == 0) {
                    size_t compSize = 0, compCount = 0;

                    switch (type) {
                        case TINYGLTF_TYPE_SCALAR: compCount = 1; break;
                        case TINYGLTF_TYPE_VEC2: compCount = 2; break;
                        case TINYGLTF_TYPE_VEC3: compCount = 3; break;
                        case TINYGLTF_TYPE_VEC4: compCount = 4; break;
                        default: return { nullptr, 0 };
                    }

                    switch (comp) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: compSize = sizeof(unsigned int); break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: compSize = sizeof(unsigned char); break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: compSize = sizeof(unsigned short); break;
                        case TINYGLTF_COMPONENT_TYPE_FLOAT: compSize = sizeof(float); break;
                        default: return { nullptr, 0 };
                    }

                    stride = compSize * compCount;
                }

                return { buffer.data.data() + (bufferView.byteOffset + accessor.byteOffset), stride };
            };

            size_t vertexCount = m_model.accessors[primitive.attributes.at("POSITION")].count;
            
            // Position
            auto [positionData, positionStride] = \
                                fetchBuffer(primitive.attributes.at("POSITION"), 
                                            TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT, vertexCount);
            if (!positionData)
                throw std::runtime_error(std::format(
                                "found invalid \"primitive.attribute.POSITION\" in mesh \"{}\". Expected {{ type: vec<float, 3>, count: {} }}", 
                                meshName, vertexCount
                            ));
            
            // Normal
            auto [normalData, normalStride] = \
                                fetchBuffer(primitive.attributes.at("NORMAL"), 
                                            TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT, vertexCount);
            if (!normalData)
                throw std::runtime_error(std::format(
                                "found invalid \"primitive.attribute.NORMAL\" in mesh \"{}\". Expected {{ type: vec<float, 3>, count: {} }}", 
                                meshName, vertexCount
                            ));

            // TexCoord
            auto [texCoordData, texCoordStride] = \
                                fetchBuffer(primitive.attributes.at("TEXCOORD_0"), 
                                            TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT, vertexCount);
            if (!texCoordData)
                throw std::runtime_error(std::format(
                                "found invalid \"primitive.attribute.TEXCOORD_0\" in mesh \"{}\". Expected {{ type: vec<float, 2>, count: {} }}", 
                                meshName, vertexCount
                            ));

            std::vector<Vertex> vertices(vertexCount);
            for (size_t j = 0; j < vertexCount; j++) {
                vertices[j].position = *reinterpret_cast<glm::vec3*>(positionData + j * positionStride);
                vertices[j].position = glm::vec3(glm::vec4(vertices[j].position, 1.0) * transform);
                vertices[j].normal = *reinterpret_cast<glm::vec3*>(normalData + j * normalStride);
                vertices[j].texCoord = *reinterpret_cast<glm::vec2*>(texCoordData + j * texCoordStride);
            }
            result[i].vertices = core::VertexBuffer::Builder()
                                        .setBuffer(vertices.data(), vertices.size() * sizeof(Vertex), GL_STATIC_DRAW)
                                        .addAttribute<float>(0, 3)
                                        .addAttribute<float>(1, 3)
                                        .addAttribute<float>(2, 2)
                                        .build();

            /* Indices */
            size_t indexCount = m_model.accessors[primitive.indices].count;
            std::vector<unsigned int> indices (indexCount);

            static const int supportedIndexType[3] = {
                TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE,
                TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
                TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,
            };

            bool hasIndices = false;
            for (int j = 0; j < 3; j++) {
                auto [indicesData, indicesStride] = \
                                    fetchBuffer(primitive.indices, 
                                                TINYGLTF_TYPE_SCALAR, supportedIndexType[j], indexCount);
                if (indicesData) {
                    if (supportedIndexType[j] == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        for (size_t k = 0; k < indexCount; k++)
                            indices[k] = *(indicesData + k * indicesStride);
                    }
                    else if (supportedIndexType[j] == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        for (size_t k = 0; k < indexCount; k++)
                            indices[k] = *reinterpret_cast<unsigned short*>(indicesData + k * indicesStride);
                    }
                    else {
                        for (size_t k = 0; k < indexCount; k++)
                            indices[k] = *reinterpret_cast<unsigned int*>(indicesData + k * indicesStride);;
                    }
                    hasIndices = true;
                    break;
                }
            }

            if (!hasIndices)
                throw std::runtime_error(std::format(
                            "invalid \"primitive's indices\" in mesh \"{}\". Expected {{ type: uint8 | uint16 | uint32, count: {} }}",
                            meshName, indexCount
                        ));

            result[i].indices.swap(indices);

            /* Material */
            tinygltf::Material& material = m_model.materials[primitive.material];

            int textureIndex;

            textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
            if (textureIndex >= 0)
                result[i].material.baseColorTexture = loadTexture(textureIndex);
            textureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
            if (textureIndex >= 0)
                result[i].material.metallicRoughnessTexture = loadTexture(textureIndex);
            textureIndex = material.normalTexture.index;
            if (textureIndex >= 0)
                result[i].material.normalTexture = loadTexture(textureIndex);
            textureIndex = material.emissiveTexture.index;
            if (textureIndex >= 0)
                result[i].material.emissiveTexture = loadTexture(textureIndex);
            textureIndex = material.occlusionTexture.index;
            if (textureIndex >= 0)
                result[i].material.occlusionTexture = loadTexture(textureIndex);

            std::vector<double>& factorHD = material.pbrMetallicRoughness.baseColorFactor;
            if (factorHD.size() == 4)
                result[i].material.baseColorFactor = glm::vec4(
                    static_cast<float>(factorHD[0]),
                    static_cast<float>(factorHD[1]),
                    static_cast<float>(factorHD[2]),
                    static_cast<float>(factorHD[3])
                );
            factorHD = material.emissiveFactor;
            if (factorHD.size() == 3)
                result[i].material.emissiveFactor = glm::vec3(
                    static_cast<float>(factorHD[0]),
                    static_cast<float>(factorHD[1]),
                    static_cast<float>(factorHD[2])
                );
            
            double factor1D = material.pbrMetallicRoughness.metallicFactor;
            if (factor1D >= 0.0)
                result[i].material.metallicFactor = static_cast<float>(factor1D);
            factor1D = material.pbrMetallicRoughness.roughnessFactor;
            if (factor1D >= 0.0)
                result[i].material.roughnessFactor = static_cast<float>(factor1D);
        }

        m_meshes.push_back({});
        (m_meshes.end() - 1)->swap(result);
    }

    size_t Model::Builder::loadTexture(int textureIndex) {
        indexChecker(m_model.textures, textureIndex);

        if (m_loadedTextures.find(textureIndex) == m_loadedTextures.end()) {
            indexChecker(m_model.images, m_model.textures[textureIndex].source);

            tinygltf::Texture& texture = m_model.textures[textureIndex];
            tinygltf::Image& image = m_model.images[texture.source];

            GLenum format;
            if (image.component == 1)
                format = GL_RED;
            else if (image.component == 2)
                format = GL_RG;
            else if (image.component == 3)
                format = GL_RGB;
            else if (image.component == 4)
                format = GL_RGBA;
            else
                throw std::runtime_error(
                        std::format("unsupported image format, with comp({})", image.component)
                    );

            GLenum compType;
            if (image.bits == 8)
                compType = GL_UNSIGNED_BYTE;
            else if (image.bits == 16)
                compType = GL_UNSIGNED_SHORT;
            else
                throw std::runtime_error(
                        std::format("unsupported image storage type, with bits({})", image.bits)
                    );

            GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR;
            GLenum magFilter = GL_LINEAR;
            GLenum wrapS = GL_CLAMP_TO_EDGE;
            GLenum wrapT = GL_CLAMP_TO_EDGE;
            if (texture.sampler >= 0) {
                indexChecker(m_model.samplers, texture.sampler);
                tinygltf::Sampler& sampler = m_model.samplers[texture.sampler];
                if (sampler.minFilter != -1)
                    minFilter = sampler.minFilter;
                if (sampler.magFilter != -1)
                    magFilter = sampler.magFilter;

                wrapS = sampler.wrapS;
                wrapT = sampler.wrapT;
            }

            GLuint texID;
            glGenTextures(1, &texID);
            glBindTexture(GL_TEXTURE_2D, texID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, 
                                           format, compType, image.image.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
            glGenerateMipmap(GL_TEXTURE_2D);

            m_textures.emplace_back(texID, GL_TEXTURE_2D, format, image.width, image.height, 0);
            m_loadedTextures[textureIndex] = m_textures.size() - 1;
        }

        return m_loadedTextures[textureIndex];
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

    void Model::draw(const core::Shader& shader) const {
        for (auto& mesh : meshes) {
            for (auto& primitive : mesh) {
                shader.bind();

                shader.setInt("baseColorTexMarker", 0);
                shader.setVec4("baseColorFactor", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
                if (primitive.material.baseColorTexture.has_value()) {
                    textures[primitive.material.baseColorTexture.value()].active(0);
                    shader.setInt("baseColorTexture", 0);
                    shader.setInt("baseColorTexMarker", 1);
                }
                if (primitive.material.baseColorFactor.has_value()) {
                    shader.setVec4("baseColorFactor", primitive.material.baseColorFactor.value());
                }

                shader.setInt("metallicRoughnessTexMarker", 0);
                shader.setFloat("metallicFactor", 0.0f);
                shader.setFloat("roughnessFactor", 0.0f);
                if (primitive.material.metallicRoughnessTexture.has_value()) {
                    textures[primitive.material.metallicRoughnessTexture.value()].active(1);
                    shader.setInt("metallicRoughnessTexture", 1);
                    shader.setInt("metallicRoughnessTexMarker", 1);
                }
                if (primitive.material.metallicFactor.has_value())
                    shader.setFloat("metallicFactor", primitive.material.metallicFactor.value());
                if (primitive.material.roughnessFactor.has_value())
                    shader.setFloat("roughnessFactor", primitive.material.roughnessFactor.value());

                shader.setInt("normalTexMarker", 0);
                if (primitive.material.normalTexture.has_value()) {
                    textures[primitive.material.normalTexture.value()].active(2);
                    shader.setInt("normalTexture", 2);
                    shader.setInt("normalTexMarker", 1);
                }

                shader.setInt("emissiveTexMarker", 0);
                shader.setVec3("emissiveFactor", glm::vec3(0.0f));
                if (primitive.material.emissiveTexture.has_value()) {
                    textures[primitive.material.emissiveTexture.value()].active(3);
                    shader.setInt("emissiveTexture", 3);
                    shader.setInt("emissiveTexMarker", 1);
                }
                if (primitive.material.emissiveFactor.has_value())
                    shader.setVec3("emissiveFactor", primitive.material.emissiveFactor.value());

                shader.setInt("occlusionTexMarker", 0);
                if (primitive.material.occlusionTexture.has_value()) {
                    textures[primitive.material.occlusionTexture.value()].active(4);
                    shader.setInt("occlusionTexture", 4);
                    shader.setInt("occlusionTexMarker", 1);
                }

                primitive.vertices.bind();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(primitive.indices.size()), 
                               GL_UNSIGNED_INT, primitive.indices.data());
            }
        }
    }
}