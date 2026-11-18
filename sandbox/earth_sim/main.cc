#include <array>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <cabin/sandbox.h>
#include <cabin/core/shader.h>
#include <cabin/core/texture.h>
#include <cabin/core/framebuffer.h>
#include <cabin/core/renderbuffer.h>
#include <cabin/utils/shape.h>
#include <cabin/utils/model.h>
#include <cabin/utils/console.h>
using namespace cabin;


/* ============ App's Settings ============ */
constexpr int APP_DEFAULT_WINDOW_WIDTH = 1500;
constexpr int APP_DEFAULT_WINDOW_HEIGHT = 900;
constexpr float APP_CAMERA_SENSITIVITY = 0.3f;

constexpr size_t RENDER_BLOOM_LEVEL = 7;
constexpr size_t RENDER_ATMOS_SAMPLE_NUM = 32;
constexpr size_t RENDER_TRANSMITTANCE_LUT_SIZE = 512;

constexpr float SUN_DIS_TO_EARTH = 40.0f;
constexpr float CAMERA_DIS_TO_EARTH = 35.0f;
constexpr float EARTH_PLANET_RADIUS = 10.0f;
constexpr float EARTH_ATMOS_RADIUS = 12.0f;


class EarthSim final : public Sandbox {
public:
    EarthSim()
    : Sandbox("Earth Sim", APP_DEFAULT_WINDOW_WIDTH, APP_DEFAULT_WINDOW_HEIGHT) {

        m_shapeSphere = utils::Shape::Builder()
                        .asSphere(1.0f)
                        .build();
        m_shapeSquare = utils::Shape::Builder()
                        .asSquare()
                        .build();
        m_modelEarth = utils::Model::Builder()
                        .fromGLB("assets/models/Earth.glb")
                        .build();

        m_shaderSun = core::Shader::Builder()
                        .fromFile("assets/shaders/Sun.shader")
                        .build();
        m_shaderEarth = core::Shader::Builder()
                        .fromFile("assets/shaders/Earth.shader")
                        .setConstant<float>("SUNLIGHT_INTENSITY", 90.0f)
                        .setConstant<float>("EMISSIVE_INTENSITY", 0.3f)
                        .build();
        m_shaderAtmosphere = core::Shader::Builder()
                        .fromFile("assets/shaders/Atmosphere.shader")
                        .setConstant<float>("ATMOS_PLANET_RADIUS", EARTH_PLANET_RADIUS)
                        .setConstant<float>("ATMOS_ATMOS_RADIUS", EARTH_ATMOS_RADIUS)
                        .setConstant<int>("ATMOS_TRANSMITTANCE_SAMPLE_NUM", RENDER_ATMOS_SAMPLE_NUM)
                        .setConstant<float>("ATMOS_ARTISTIC_FACTOR", 5.0f)
                        .setConstant<int>("SCATTERING_SAMPLE_NUM", RENDER_ATMOS_SAMPLE_NUM)
                        .build();
        m_shaderBloomDS = core::Shader::Builder()
                        .fromFile("assets/shaders/BloomDS.shader")
                        .build();
        m_shaderBloomUS = core::Shader::Builder()
                        .fromFile("assets/shaders/BloomUS.shader")
                        .build();
        m_shaderComposite = core::Shader::Builder()
                        .fromFile("assets/shaders/Composite.shader")
                        .build();
        m_shaderToneMapping = core::Shader::Builder()
                        .fromFile("assets/shaders/ToneMapping.shader")
                        .build();
        m_shaderTransmittanceLUT = core::Shader::Builder()
                        .fromFile("assets/shaders/TransmittanceLUT.shader")
                        .setConstant<float>("ATMOS_PLANET_RADIUS", EARTH_PLANET_RADIUS)
                        .setConstant<float>("ATMOS_ATMOS_RADIUS", EARTH_ATMOS_RADIUS)
                        .setConstant<int>("ATMOS_TRANSMITTANCE_SAMPLE_NUM", RENDER_ATMOS_SAMPLE_NUM)
                        .setConstant<float>("ATMOS_ARTISTIC_FACTOR", 5.0f)
                        .build();

        enableImGui();

        createSurfaceDependResources();

        generateTransmittanceLUT();
        
        glfwSetWindowUserPointer(window, static_cast<void*>(this));

        glfwSetFramebufferSizeCallback(this->window, [](GLFWwindow* window, int width, int height) {
            auto* app = reinterpret_cast<EarthSim*>(glfwGetWindowUserPointer(window));
            app->createSurfaceDependResources();
            glViewport(0, 0, width, height);
        });
        
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
            if (ImGui::GetIO().WantCaptureMouse) {
                ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
                return;
            }

            auto* app = reinterpret_cast<EarthSim*>(glfwGetWindowUserPointer(window));
            if (GLFW_MOUSE_BUTTON_LEFT == button) {
                if (GLFW_PRESS == action && !app->m_cursorDragging) {
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    
                    auto& ctx = app->m_cursorDragStartCtx;
                    ctx.cameraYaw = app->m_cameraYaw;
                    ctx.cameraPitch = app->m_cameraPitch;
                    ctx.cursorPosition.x = static_cast<float>(xpos);
                    ctx.cursorPosition.y = static_cast<float>(ypos);
                    
                    app->m_cursorDragging = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else if (GLFW_RELEASE == action && app->m_cursorDragging) {
                    app->m_cursorDragging = false;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* window, double, double yoffset) {
            auto* app = reinterpret_cast<EarthSim*>(glfwGetWindowUserPointer(window));
            // Apply changes to FOV and limit it to [20, 90].
            app->m_cameraFOV = glm::clamp(
                app->m_cameraFOV - static_cast<float>(yoffset), 
                20.0f, 90.0f
            );
        });

        // Enable raw mouse input if available.
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        // Enable VSync.
        glfwSwapInterval(GL_TRUE);
    }

    void createSurfaceDependResources() {
        auto [width, height] = getFramebufferSize();

        m_textureShared = core::Texture::Builder()
                    .asEmpty2D(width, height, GL_RGB32F)
                    .setFilter(GL_NEAREST, GL_NEAREST)
                    .setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER)
                    .setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f })
                    .build();
        m_renderTargetColorAttachment = core::Texture::Builder()
                    .asEmpty2D(width, height, GL_RGB32F)
                    .setFilter(GL_NEAREST, GL_NEAREST)
                    .setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER)
                    .setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f })
                    .build();

        // Down Sampling Textures
        for (int i = 0; i <= RENDER_BLOOM_LEVEL; i++) {
            int texW = width >> (i + 1);
            int texH = height >> (i + 1);
            m_texturesBloomPass[i] = core::Texture::Builder()
                    .asEmpty2D(texW, texH, GL_RGB32F)
                    .setFilter(GL_LINEAR, GL_LINEAR)
                    .setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER)
                    .setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f })
                    .build();
        }

        // Up Sampling Textures
        for (int i = 0; i < RENDER_BLOOM_LEVEL; i++) {
            int texW = width >> (RENDER_BLOOM_LEVEL - i);
            int texH = height >> (RENDER_BLOOM_LEVEL - i);
            m_texturesBloomPass[RENDER_BLOOM_LEVEL + i + 1] = core::Texture::Builder()
                    .asEmpty2D(texW, texH, GL_RGB32F)
                    .setFilter(GL_LINEAR, GL_LINEAR)
                    .setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER)
                    .setBorderColor({ 0.0f, 0.0f, 0.0f, 0.0f })
                    .build();
        }
    }

    void generateTransmittanceLUT() {
        utils::Console::info("generating Transmittance LUT...");
        m_textureTransmittanceLUT = core::Texture::Builder()
                    .asEmpty2D(RENDER_TRANSMITTANCE_LUT_SIZE, RENDER_TRANSMITTANCE_LUT_SIZE >> 2, GL_RGB32F)
                    .setFilter(GL_LINEAR, GL_LINEAR)
                    .setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
                    .build();

        m_framebuffer.bind();
        m_framebuffer.attachTexture(
            GL_COLOR_ATTACHMENT0, 
            m_textureTransmittanceLUT.id.value()
        );
        glViewport(
            0, 0, 
            m_textureTransmittanceLUT.width,
            m_textureTransmittanceLUT.height
        );

        glDisable(GL_DEPTH_TEST);

        m_shaderTransmittanceLUT.bind();
        m_shapeSquare.draw();
    }

    void renderFrame() override {
        processInput();

        auto [frameW, frameH] = getFramebufferSize();
        
        glEnable(GL_CULL_FACE);

        /* ----------- 1. Scene Pass ----------- */
        m_framebuffer.bind();
        m_framebuffer.attachTexture(
            GL_COLOR_ATTACHMENT0, 
            m_renderTargetColorAttachment.id.value()
        );

        glViewport(0, 0, frameW, frameH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get camera's position and view matrix.
        float yaw = glm::radians(m_cameraYaw);
        float pitch = glm::radians(m_cameraPitch);
        glm::vec3 cameraPos = CAMERA_DIS_TO_EARTH * glm::vec3 {
            glm::cos(pitch) * glm::sin(yaw),
            glm::sin(pitch),
            glm::cos(pitch) * glm::cos(yaw)
        };

        glm::mat4 view = glm::lookAt(cameraPos, m_earthPosition, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 projection;
        float aspect = static_cast<float>(frameW) / (frameH != 0.0f ? frameH : 1.0f);
        projection = glm::perspective(glm::radians(m_cameraFOV), aspect, 0.1f, 100.0f);

        // Get sun's position.
        float sunAngle = glm::radians(m_sunAngle);
        glm::vec3 sunPosition = SUN_DIS_TO_EARTH * glm::vec3 {
            glm::sin(sunAngle),
            0.0f,
            glm::cos(sunAngle)
        };

        // Draw Sun.
        glm::mat4 model { 1.0f };
        model = glm::translate(model, sunPosition);

        m_shaderSun.bind();
        m_shaderSun.setMat4("model", model);
        m_shaderSun.setMat4("view", view);
        m_shaderSun.setMat4("projection", projection);
        m_shaderSun.setVec3("sunLuminance", m_sunLuminance);

        m_shapeSphere.draw();

        // Draw Earth.
        model = { 1.0f };
        model = glm::translate(model, m_earthPosition);
        model = glm::scale(model, glm::vec3(EARTH_PLANET_RADIUS));
        model = glm::rotate(model, glm::radians(100.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::mat3 normalMatrix = glm::mat3(model);
        normalMatrix = glm::transpose(glm::inverse(normalMatrix));

        m_shaderEarth.bind();
        m_shaderEarth.setMat4("model", model);
        m_shaderEarth.setMat4("view", view);
        m_shaderEarth.setMat4("projection", projection);
        m_shaderEarth.setMat3("normalMatrix", normalMatrix);
        m_shaderEarth.setVec3("sunPosition", sunPosition);
        m_shaderEarth.setVec3("sunLuminance", m_sunLuminance);
        m_shaderEarth.setVec3("cameraPosition", cameraPos);

        m_modelEarth.draw(m_shaderEarth);

        /* ----------- 2. Copying Pass (RenderTarget -> TextureShared) ----------- */
        m_framebuffer.bind();
        m_framebuffer.attachTexture(
            GL_COLOR_ATTACHMENT0, 
            m_textureShared.id.value()
        );
        
        glViewport(0, 0, frameW, frameH);
        glDisable(GL_DEPTH_TEST);

        m_shaderComposite.bind();
        m_shaderComposite.setFloat("factor", 1.0f);
        m_renderTargetColorAttachment.active(0);
        m_shaderComposite.setInt("textureInput", 0);

        m_shapeSquare.draw();

        /* ----------- 3. Atmosphere Pass ----------- */
        m_framebuffer.bind();
        m_framebuffer.attachTexture(
            GL_COLOR_ATTACHMENT0, 
            m_renderTargetColorAttachment.id.value()
        );
        glViewport(0, 0, frameW, frameH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        model = { 1.0f };
        model = glm::translate(model, m_earthPosition);
        model = glm::scale(model, glm::vec3(EARTH_ATMOS_RADIUS));

        m_shaderAtmosphere.bind();
        m_shaderAtmosphere.setMat4("model", model);
        m_shaderAtmosphere.setMat4("view", view);
        m_shaderAtmosphere.setMat4("projection", projection);
        m_shaderAtmosphere.setVec3("sunPosition", sunPosition);
        m_shaderAtmosphere.setVec3("earthPosition", m_earthPosition);
        m_shaderAtmosphere.setVec3("cameraPosition", cameraPos);
        m_shaderAtmosphere.setVec3("sunLuminance", m_sunLuminance);
        m_textureShared.active(0);
        m_shaderAtmosphere.setInt("textureBackGround", 0);
        m_textureTransmittanceLUT.active(1);
        m_shaderAtmosphere.setInt("textureTransmittanceLUT", 1);

        m_shapeSphere.draw();

        /* ----------- 4. Bloom Pass ----------- */
        m_framebuffer.bind();
        glDisable(GL_DEPTH_TEST);
        
        // Down Sampling
        for (int i = 0; i <= RENDER_BLOOM_LEVEL; i++) {
            core::Texture& colorAttach = m_texturesBloomPass[i];

            m_framebuffer.attachTexture(
                GL_COLOR_ATTACHMENT0, 
                colorAttach.id.value()
            );
            glViewport(0, 0, colorAttach.width, colorAttach.height);

            core::Texture& texPrev = \
                i == 0 ? 
                m_textureShared :
                m_texturesBloomPass[i - 1];
            
            m_shaderBloomDS.bind();
            m_shaderBloomDS.setVec2("texelSizePrev", {
                1.0f / texPrev.width,
                1.0f / texPrev.height
            });
            texPrev.active(0);
            m_shaderBloomDS.setInt("texturePrev", 0);
            
            m_shapeSquare.draw();
        }

        // Up Sampling
        for (int i = 0; i < RENDER_BLOOM_LEVEL; i++) {
            core::Texture& colorAttach = m_texturesBloomPass[RENDER_BLOOM_LEVEL + i + 1];
            
            m_framebuffer.attachTexture(
                GL_COLOR_ATTACHMENT0, 
                colorAttach.id.value()
            );
            glViewport(0, 0, colorAttach.width, colorAttach.height);

            core::Texture& texCurr = m_texturesBloomPass[RENDER_BLOOM_LEVEL - i - 1];
            core::Texture& texPrev = m_texturesBloomPass[RENDER_BLOOM_LEVEL + i];

            m_shaderBloomUS.bind();
            m_shaderBloomUS.setVec2("texelSizeCurr", {
                1.0f / texCurr.width,
                1.0f / texCurr.height
            });
            texCurr.active(0);
            m_shaderBloomUS.setInt("textureCurr", 0);
            texPrev.active(1);
            m_shaderBloomUS.setInt("texturePrev", 1);

            m_shapeSquare.draw();
        }

        /* ----------- 5. Composite Bloom and Scene Pass ----------- */
        m_framebuffer.bind();
        m_framebuffer.attachTexture(
            GL_COLOR_ATTACHMENT0, 
            m_renderTargetColorAttachment.id.value()
        );

        glViewport(0, 0, frameW, frameH);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        m_shaderComposite.bind();
        m_shaderComposite.setFloat("factor", 0.3f);
        m_texturesBloomPass.back().active(0);
        m_shaderComposite.setInt("textureInput", 0);

        m_shapeSquare.draw();

        glDisable(GL_BLEND);

        /* ----------- 6. Tone Mapping Pass ----------- */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, frameW, frameH);
        glDisable(GL_DEPTH_TEST);

        m_shaderToneMapping.bind();
        m_renderTargetColorAttachment.active(0);
        m_shaderToneMapping.setInt("textureInput", 0);

        m_shapeSquare.draw();
    }

    void interfaceFrame() override {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | 
                                       ImGuiWindowFlags_AlwaysAutoResize | 
                                       ImGuiWindowFlags_NoSavedSettings | 
                                       ImGuiWindowFlags_NoFocusOnAppearing | 
                                       ImGuiWindowFlags_NoNav |
                                       ImGuiWindowFlags_NoMove;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 workPos = viewport->WorkPos;
        ImVec2 workSize = viewport->WorkSize;
        ImVec2 windowPos = {
            workPos.x + 10.0f,
            workPos.y + workSize.y - 10.0f
        };
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, { 0.0f, 1.0f });
        ImGui::SetNextWindowBgAlpha(0.75f);

        if (ImGui::Begin("Menu", nullptr, windowFlags))
        {
            ImGui::BulletText("Sun Angle (degree)");
            ImGui::SliderFloat("", &m_sunAngle, 0.0f, 360.0f, "%.1f");
            ImGui::Text("(hints: use mouse to drag and zoom)");
        }
        ImGui::End();
    }

    void processInput() {
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (m_cursorDragging) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            auto& ctx = m_cursorDragStartCtx;

            glm::vec2 cursorDelta = {
                static_cast<float>(xpos) - ctx.cursorPosition.x,
                static_cast<float>(ypos) - ctx.cursorPosition.y
            };

            m_cameraYaw = ctx.cameraYaw - cursorDelta.x * APP_CAMERA_SENSITIVITY;
            m_cameraPitch = ctx.cameraPitch + cursorDelta.y * APP_CAMERA_SENSITIVITY;

            // Limit camera.Yaw in [-360, 360].
            m_cameraYaw = m_cameraYaw >  360.0f ? m_cameraYaw - 360.0f : m_cameraYaw;
            m_cameraYaw = m_cameraYaw < -360.0f ? m_cameraYaw + 360.0f : m_cameraYaw;

            // Limit camera.Pitch in [-89, 89] to avoid direction oscillating.
            m_cameraPitch = glm::clamp(m_cameraPitch, -89.0f, 89.0f);
        }
    }

private:
    utils::Shape m_shapeSquare;
    utils::Shape m_shapeSphere;
    utils::Model m_modelEarth;

    core::FrameBuffer m_framebuffer;

    core::Shader m_shaderSun;
    core::Shader m_shaderEarth;
    core::Shader m_shaderAtmosphere;
    core::Shader m_shaderBloomDS;
    core::Shader m_shaderBloomUS;
    core::Shader m_shaderComposite;
    core::Shader m_shaderToneMapping;
    core::Shader m_shaderTransmittanceLUT;

    core::Texture m_textureShared;
    core::Texture m_textureTransmittanceLUT;
    core::Texture m_renderTargetColorAttachment;
    std::array<core::Texture, RENDER_BLOOM_LEVEL * 2 + 1> m_texturesBloomPass;

    // Space
    float m_sunAngle = 140.0f;
    glm::vec3 m_sunLuminance = { 10.0f, 10.0f, 9.0f };
    glm::vec3 m_earthPosition = { 0.0f, 0.0f, 0.0f };

    // Camera
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = 0.0f;
    float m_cameraFOV = 50.0f;

    // Input & Performance
    bool m_cursorDragging = false;
    struct {
        float cameraYaw;
        float cameraPitch;
        glm::vec2 cursorPosition;
    } m_cursorDragStartCtx;
};

int main() {
    return SandboxApp<EarthSim>::run();
}