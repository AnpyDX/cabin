#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "cabin/sandbox.h"
#include "cabin/utils/model.h"
#include "cabin/utils/shape.h"
#include "cabin/utils/camera.h"
#include "cabin/core/shader.h"
#include "cabin/core/texture.h"
#include "cabin/core/framebuffer.h"
using namespace cabin;

const int ENVIRONMENT_RESOLUTION = 2048;

class HelloPBR: public Sandbox {
public:
    HelloPBR() : Sandbox("Hello PBR", 800, 600) {
        m_et2cubeShader = core::Shader::Builder()
                            .fromFile("hello_pbr/et2cube.shader")
                            .build();
        
        m_irradianceShader = core::Shader::Builder()
                                .fromFile("hello_pbr/irradiance.shader")
                                .build();

        m_prefilterShader = core::Shader::Builder()
                                .fromFile("hello_pbr/prefilter.shader")
                                .build();

        m_BRDFLUTShader = core::Shader::Builder()
                            .fromFile("hello_pbr/brdf.shader")
                            .build();

        m_shapePBRShader = core::Shader::Builder()
                            .fromFile("hello_pbr/shapePBR.shader")
                            .build();

        m_modelPBRShader = core::Shader::Builder()
                            .fromFile("hello_pbr/modelPBR.shader")
                            .build();

        m_skyboxShader = core::Shader::Builder()
                            .fromFile("hello_pbr/skybox.shader")
                            .build();

        m_sphere = utils::Shape::Builder().asShpere(1.0, 64).build();

        m_cube = utils::Shape::Builder().asCube().build();

        m_sponzaModel = utils::Model::Builder().fromGLB("assets/models/Sponza.glb").build();

        m_coffeeCartModel = utils::Model::Builder().fromGLB("assets/models/CoffeeCart.glb").build();

        lightPositions = {
            { "lightPositions[0]", {} },
            { "lightPositions[1]", {} },
            { "lightPositions[2]", {} },
            { "lightPositions[3]", {} },
        };

        enableImGui();
        ImGui::GetIO().IniFilename = nullptr;

        glfwSetWindowAttrib(window, GL_SAMPLES, 4);
        glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
        
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
            if (ImGui::GetIO().WantCaptureMouse) return;

            auto app = reinterpret_cast<HelloPBR*>(glfwGetWindowUserPointer(window));
            app->m_camera.mouseButtonCallback(window, button, action);

        });

        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        generateIBLCubeMaps();
    }

    void generateIBLCubeMaps() {
        /* Get HDRI Path */
        if (HDRIIndex < 0 || HDRIIndex > 3)
            throw std::runtime_error("bad HDRImage index");

        static const char* HDRImagePaths[] = {
            "newport_loft.hdr",
            "gear_store_2k.hdr",
            "empty_play_room_2k.hdr",
            "billiard_hall_2k.hdr"
        };

        std::string HDRIPath = std::format("assets/textures/HDRI/{}", HDRImagePaths[HDRIIndex]);
        utils::Console::info(std::format("generating IBL for HDRI: \"{}\"", HDRIPath));

        /* IBL Generating Preparation */
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        core::FrameBuffer FBO {};
        FBO.bind();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        std::vector<glm::mat4> views = {
            glm::lookAt(glm::vec3(0.0), {  1.0,  0.0,  0.0 }, { 0.0, -1.0,  0.0 }),
            glm::lookAt(glm::vec3(0.0), { -1.0,  0.0,  0.0 }, { 0.0, -1.0,  0.0 }),
            glm::lookAt(glm::vec3(0.0), {  0.0,  1.0,  0.0 }, { 0.0,  0.0,  1.0 }),
            glm::lookAt(glm::vec3(0.0), {  0.0, -1.0,  0.0 }, { 0.0,  0.0, -1.0 }),
            glm::lookAt(glm::vec3(0.0), {  0.0,  0.0,  1.0 }, { 0.0, -1.0,  0.0 }),
            glm::lookAt(glm::vec3(0.0), {  0.0,  0.0, -1.0 }, { 0.0, -1.0,  0.0 })
        };

        int mapLength {};

        /* From Equirectangular To CubeMap */
        auto hdrTexture = core::Texture::Builder()
                                .fromFile2D(HDRIPath, GL_RGB32F)
                                .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                                .setFilter(GL_LINEAR, GL_LINEAR)
                                .build();
        
        mapLength = ENVIRONMENT_RESOLUTION;
        m_envCubeMap = core::Texture::Builder()
                            .asEmptyCubeMap(mapLength, GL_RGB32F)
                            .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                            .setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)
                            .build();
        
        hdrTexture.active(0);
        m_et2cubeShader.bind();
        m_et2cubeShader.setInt("etHDRTexture", 0);
        m_et2cubeShader.setMat4("projection", projection);

        glViewport(0, 0, mapLength, mapLength);
        for (unsigned int i = 0; i < 6; i++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubeMap.id.value(), 0);
            m_et2cubeShader.setMat4("view", views[i]);
            m_cube.draw();
        }

        glGenerateTextureMipmap(m_envCubeMap.id.value());
        
        /* Irradiance Map */
        mapLength = ENVIRONMENT_RESOLUTION / 8;
        m_irradianceMap = core::Texture::Builder()
                                    .asEmptyCubeMap(mapLength, GL_RGB32F)
                                    .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                                    .setFilter(GL_LINEAR, GL_LINEAR)
                                    .build();
        
        m_envCubeMap.active(0);
        m_irradianceShader.bind();
        m_irradianceShader.setMat4("projection", projection);
        m_irradianceShader.setInt("envCubeMap", 0);
        
        glViewport(0, 0, mapLength, mapLength);
        for (unsigned int i = 0; i < 6; i++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap.id.value(), 0);
            m_irradianceShader.setMat4("view", views[i]);
            m_cube.draw();
        }

        /* Prefilter Map */
        mapLength = ENVIRONMENT_RESOLUTION / 8;
        m_prefilterMap = core::Texture::Builder()
                                .asEmptyCubeMap(mapLength, GL_RGB32F)
                                .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                                .setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)
                                .genMipmap()
                                .build();
        
        m_envCubeMap.active(0);
        m_prefilterShader.bind();
        m_prefilterShader.setMat4("projection", projection);
        m_prefilterShader.setFloat("envResolution", static_cast<float>(ENVIRONMENT_RESOLUTION));
        m_prefilterShader.setInt("envCubeMap", 0);

        int maxMipmapLevel = 5;

        for (int level = 0; level < maxMipmapLevel; level++) {
            int mipmapWidth = static_cast<int>(mapLength * pow(0.5, level));
            int mipmapHeight = static_cast<int>(mapLength * pow(0.5, level));
            glViewport(0, 0, mipmapWidth, mipmapHeight);

            float roughness = (level + 1.0f) / maxMipmapLevel;
            m_prefilterShader.setFloat("roughnessFactor", roughness);

            for (int i = 0; i < 6; i++) {
                m_prefilterShader.setMat4("view", views[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_prefilterMap.id.value(), level);
                m_cube.draw();
            }
        }

        /* BRDF LUT Map */
        mapLength = ENVIRONMENT_RESOLUTION / 2;
        m_BRDFLUTMap = core::Texture::Builder()
                                .asEmpty2D(mapLength, mapLength, GL_RG32F)
                                .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                                .setFilter(GL_LINEAR, GL_LINEAR)
                                .build();
        auto planeShape = utils::Shape::Builder().asPlane().build();

        m_BRDFLUTShader.bind();
        glViewport(0, 0, mapLength, mapLength);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BRDFLUTMap.id.value(), 0);
        planeShape.draw();

        // Resize viewport to window's size
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        auto [w, h] = getWindowSize();
        glViewport(0, 0, w, h);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
    }

    void renderFrame() override {
        processInput();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightPositions[0].second = {  lightSpacing,  lightSpacing, lightDistance };
        lightPositions[1].second = { -lightSpacing,  lightSpacing, lightDistance };
        lightPositions[2].second = {  lightSpacing, -lightSpacing, lightDistance };
        lightPositions[3].second = { -lightSpacing, -lightSpacing, lightDistance };

        glm::mat4 view = m_camera.getLookAt();
        glm::mat4 projection;
        auto [width, height] = getWindowSize();
        float aspect = static_cast<float>(width) / (height != 0.0f ? height : 1.0f);
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 50.0f);
        
        /* Render Scene */
        glEnable(GL_CULL_FACE);

        if (sceneIndex == 0 || sceneIndex == 1) {
            m_shapePBRShader.bind();
            m_shapePBRShader.setMat4("view", view);
            m_shapePBRShader.setMat4("projection", projection);
            m_shapePBRShader.setVec3("cameraPosition", m_camera.position);
            
            m_irradianceMap.active(0);
            m_shapePBRShader.setInt("irradianceMap", 0);
            m_prefilterMap.active(1);
            m_shapePBRShader.setInt("prefilterMap", 1);
            m_BRDFLUTMap.active(2);
            m_shapePBRShader.setInt("BRDFLUTMap", 2);

            m_shapePBRShader.setVec3("lightColor", lightIntensity * lightColor);
            for (auto& [name, value] : lightPositions) {
                m_shapePBRShader.setVec3(name, value);
            }

            if (sceneIndex == 0) {
                for (int i = 0; i < sphereRowCount; i++) {
                    for (int j = 0; j < sphereColCount; j++) {
                        glm::mat4 model { 1.0f };
                        model = glm::translate(model, {
                            (2.0 * sphereRadius + sphereSpacing) * (sphereRowCount / 2.0f - i),
                            (2.0 * sphereRadius + sphereSpacing) * (j - sphereColCount / 2.0f),
                            0.0f
                        });
                        model = glm::scale(model, glm::vec3(sphereRadius));

                        glm::mat3 normalMatrix = glm::mat3(model);
                        normalMatrix = glm::transpose(glm::inverse(normalMatrix));

                        m_shapePBRShader.setMat4("model", model);
                        m_shapePBRShader.setMat3("normalMatrix", normalMatrix);

                        float metallicFactor = static_cast<float>(j) / sphereColCount;
                        float roughnessFactor = 1.0f - static_cast<float>(i) / sphereRowCount;
                        
                        m_shapePBRShader.setVec3("baseColorFactor", { 0.5f, 0.0f, 0.0f });
                        m_shapePBRShader.setFloat("metallicFactor", metallicFactor);
                        m_shapePBRShader.setFloat("roughnessFactor", roughnessFactor);
                        m_shapePBRShader.setFloat("occlusionFactor", 1.0f);

                        m_sphere.draw();
                    }
                }
            }
            else {
                glm::mat4 model { 1.0f };
                model = glm::scale(model, glm::vec3(0.5f));

                glm::mat3 normalMatrix = glm::mat3(model);
                normalMatrix = glm::transpose(glm::inverse(normalMatrix));

                m_shapePBRShader.setMat4("model", model);
                m_shapePBRShader.setMat3("normalMatrix", normalMatrix);
                
                m_shapePBRShader.setVec3("baseColorFactor", mtBaseColor);
                m_shapePBRShader.setFloat("metallicFactor", mtORM.b);
                m_shapePBRShader.setFloat("roughnessFactor", mtORM.g);
                m_shapePBRShader.setFloat("occlusionFactor", mtORM.r);

                m_sphere.draw();
            }            
        }

        else if (sceneIndex == 2 || sceneIndex == 3) {
            glm::mat4 model { 1.0f };

            if (sceneIndex == 2) {
                model = glm::translate(model, glm::vec3(0.0f, -1.0f, 1.0f));
                model = glm::scale(model, glm::vec3(sponzaScaleFactor));
            }
            else {
                model = glm::translate(model, glm::vec3(0.0, -1.0, 0.0));
                model = glm::rotate(model, glm::radians(coffeeCartRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(coffeeCartScaleFactor));
            }

            glm::mat3 normalMatrix = glm::mat3(model);
            normalMatrix = glm::transpose(glm::inverse(normalMatrix));

            m_modelPBRShader.bind();
            m_modelPBRShader.setMat4("model", model);
            m_modelPBRShader.setMat4("view", view);
            m_modelPBRShader.setMat4("projection", projection);
            m_modelPBRShader.setMat3("normalMatrix", normalMatrix);
            m_modelPBRShader.setVec3("cameraPosition", m_camera.position);

            m_irradianceMap.active(5);
            m_modelPBRShader.setInt("irradianceMap", 5);
            m_prefilterMap.active(6);
            m_modelPBRShader.setInt("prefilterMap", 6);
            m_BRDFLUTMap.active(7);
            m_modelPBRShader.setInt("BRDFLUTMap", 7);

            m_modelPBRShader.setVec3("lightColor", lightIntensity * lightColor);
            for (auto& [name, value] : lightPositions) {
                m_modelPBRShader.setVec3(name, value);
            }

            if (sceneIndex == 2) {
                m_sponzaModel.draw(m_modelPBRShader);
            }
            else {
                m_coffeeCartModel.draw(m_modelPBRShader);
                if (rotateCoffeeCartModel) {
                    coffeeCartRotationAngle += coffeeCartRotationSpeed;
                    if (coffeeCartRotationAngle >= 360.0f)
                        coffeeCartRotationAngle = 0.0f;
                }
            }
        }

        /* Render Skybox */
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LEQUAL);

        switch (skyboxMode) {
            case 0:
                m_envCubeMap.active(0);
                break;
            case 1:
                m_prefilterMap.active(0);
                break;
            case 2:
                m_irradianceMap.active(0);
                break;
            default:
                m_envCubeMap.active(0);
        }

        m_skyboxShader.bind();
        m_skyboxShader.setInt("envCubeMap", 0);
        m_skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        m_skyboxShader.setMat4("projection", projection);
        m_cube.draw();
    }

    void interfaceFrame() override {

        ImGui::SetNextWindowSize(ImVec2(320, 490), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Inspector")) {

            ImGui::SeparatorText("Environment");
            ImGui::BulletText("Skybox"); {
                static const char* HDRIs[] = { "Newport Loft", "Gear Store", "Empty Playroom", "Billiard Hall" };
                if (ImGui::Combo("HDRI", &HDRIIndex, HDRIs, IM_ARRAYSIZE(HDRIs))) {
                    generateIBLCubeMaps();
                }

                static const char* skyboxModes[] = { "HDR", "Prefilter", "Irradiance" };
                ImGui::Combo("Mode", &skyboxMode, skyboxModes, IM_ARRAYSIZE(skyboxModes));
            }
            ImGui::BulletText("Lights"); {
                ImGui::InputFloat("Distance##lights", &lightDistance, 1.0f);
                ImGui::InputFloat("Spacing##lights", &lightSpacing, 1.0f);
                ImGui::InputFloat("Intensity##lights", &lightIntensity, 10.0f);
                ImGui::ColorEdit3("Color##lights", &lightColor[0]);

                lightDistance  =  glm::clamp(lightDistance, 0.0f, 1000.0f);
                lightSpacing   =  glm::clamp(lightSpacing,  0.0f, 1000.0f);
                lightIntensity =  glm::clamp(lightIntensity, 0.0f, 8000.0f);
            }


            ImGui::SeparatorText("Scene");
            static const char* scenes[] = { "Spheres", "Material Sandbox", "Sponza", "Coffee Cart" };
            ImGui::Combo("Scene", &sceneIndex, scenes, IM_ARRAYSIZE(scenes));

            if (sceneIndex == 0) {
                ImGui::Text("-  Spheres Settings");
                ImGui::InputInt("Row##0", &sphereRowCount, 1);
                ImGui::InputInt("Column##0", &sphereColCount, 1);
                ImGui::InputFloat("Spacing##0", &sphereSpacing, 0.1f);
                ImGui::InputFloat("Radius##0", &sphereRadius, 0.1f);

                sphereRowCount = glm::clamp(sphereRowCount, 1, 10);
                sphereColCount = glm::clamp(sphereColCount, 1, 10);
                sphereSpacing  = glm::clamp(sphereSpacing, 0.0f, 10.0f);
                sphereRadius   = glm::clamp(sphereRadius,  0.1f, 10.0f);
            }
            
            else if (sceneIndex == 1) {
                ImGui::Text("-  Material Settings");
                ImGui::ColorEdit3("BaseColor##1", &mtBaseColor[0]);
                ImGui::InputFloat("Metallic##1", &mtORM[2], 0.1f);
                ImGui::InputFloat("Roughness##1", &mtORM[1], 0.1f);
                ImGui::InputFloat("occlusion##1", &mtORM[0], 0.1f);

                mtORM = glm::clamp(mtORM, glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(1.0f));
            }

            else if (sceneIndex == 2) {
                ImGui::Text("- Sponza Settings");
                ImGui::InputFloat("Scale##2", &sponzaScaleFactor, 0.1f);

                sponzaScaleFactor = glm::clamp(sponzaScaleFactor, 0.1f, 10.0f);
            }

            else if (sceneIndex == 3) {
                ImGui::Text("- Coffee Cart Settings");
                ImGui::InputFloat("Scale##3", &coffeeCartScaleFactor, 0.1f);
                
                ImGui::Checkbox("Rotate##3", &rotateCoffeeCartModel);
                ImGui::InputFloat("Rotate speed##3", &coffeeCartRotationSpeed, 0.1f);

                coffeeCartScaleFactor = glm::clamp(coffeeCartScaleFactor, 0.1f, 10.0f);
                coffeeCartRotationSpeed = glm::clamp(coffeeCartRotationSpeed, 0.1f, 10.0f);
            }
        }
        ImGui::End();
    }

    void processInput() {
        m_camera.updateInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

private:
    int sceneIndex = 0;

    // Skybox Settings
    int HDRIIndex = 0;
    int skyboxMode = 0;

    // Lights Settings
    float lightSpacing = 10.0f;
    float lightDistance = 10.0f;
    glm::vec3 lightColor { 1.0f };
    float lightIntensity = 100.0f;
    std::vector<std::pair<std::string, glm::vec3>> lightPositions {};

    // Spheres Settings
    int sphereRowCount = 5;
    int sphereColCount = 5;
    float sphereRadius = 0.5f;
    float sphereSpacing = 0.3f;

    // Material Sandbox Settings
    glm::vec3 mtBaseColor { 0.5f };
    glm::vec3 mtORM { 1.0f, 0.5f, 0.5f };

    // Sponza Model Settings
    float sponzaScaleFactor = 1.0f;

    // Coffee Cart Model Settings
    bool  rotateCoffeeCartModel = false;
    float coffeeCartScaleFactor = 1.0f;
    float coffeeCartRotationSpeed = 1.0f;
    float coffeeCartRotationAngle = 0.0f;
    
private:
    utils::Camera m_camera {
        { 0.0, 0.0, 5.0 },
        { 0.0, 0.0, -1.0 },
        { 0.0, 1.0, 0.0 }
    };
    utils::Shape m_cube {};
    utils::Shape m_sphere {};
    utils::Model m_sponzaModel {};
    utils::Model m_coffeeCartModel {};

    core::Shader m_et2cubeShader {};
    core::Shader m_irradianceShader {};
    core::Shader m_prefilterShader {};
    core::Shader m_BRDFLUTShader {};

    core::Shader m_shapePBRShader {};
    core::Shader m_modelPBRShader {};
    core::Shader m_skyboxShader {};

    core::Texture m_envCubeMap {};
    core::Texture m_irradianceMap {};
    core::Texture m_prefilterMap {};
    core::Texture m_BRDFLUTMap {};
};

int main() {
    return SandboxApp<HelloPBR>::run();
}