#include "camera.h"
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace cabin::utils {
    Camera::Camera(const glm::vec3& position, const glm::vec3& front, const glm::vec3 up)
    : position(position) {
        setDirection(front, up);
    }

    void Camera::setSensitivity(float sensitivity) {
        this->sensitivity = sensitivity;
    }

    void Camera::setSpeedMaximum(const glm::vec2& speed) {
        this->speedMaximum = speed;
    }

    void Camera::setAcceleration(const glm::vec2& acceleration) {
        this->acceleration = acceleration;
    }

    void Camera::setPosition(const glm::vec3& position) {
        this->position = position;
    }

    void Camera::setDirection(const glm::vec3& front) {
        m_frontDirectionBase = glm::normalize(front);
        m_leftDirectionBase = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), m_frontDirectionBase));
        m_upDirectionBase = glm::normalize(glm::cross(m_frontDirectionBase, m_leftDirectionBase));

        m_upDirection = m_upDirectionBase;
        m_leftDirection = m_leftDirectionBase;
        m_frontDirection = m_frontDirectionBase;
    }

    void Camera::setDirection(const glm::vec3& front, const glm::vec3& up) {
        m_upDirectionBase = glm::normalize(up);
        m_frontDirectionBase = glm::normalize(front);
        m_leftDirectionBase = glm::normalize(glm::cross(m_upDirectionBase, m_frontDirectionBase));

        m_upDirection = m_upDirectionBase;
        m_leftDirection = m_leftDirectionBase;
        m_frontDirection = m_frontDirectionBase;
    }

    void Camera::setCursorOrigin(const glm::vec2& position) {
        m_cursorOrigin = position;
        m_upDirectionBase = m_upDirection;
        m_leftDirectionBase = m_leftDirection;
        m_frontDirectionBase = m_frontDirection;
    }

    void Camera::updateInput(GLFWwindow* window) {
        /* Get Keyboard and Mouse Input */
        glm::vec2 moveSign {0.0f}, cursorPos {0.0f};

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            moveSign.x += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            moveSign.x -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            moveSign.y += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            moveSign.y -= 1.0f;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        cursorPos.x = static_cast<float>(xpos);
        cursorPos.y = static_cast<float>(ypos);

        /* View Control */
        if (cursorCaptured) {
            glm::vec2 cursorOffset = cursorPos - m_cursorOrigin;
            cursorOffset *= sensitivity * 0.01f;

            glm::mat4 horizontalRotate { 1.0f };
            horizontalRotate = glm::rotate(horizontalRotate, glm::radians(-cursorOffset.x), m_upDirectionBase);
            m_frontDirection = horizontalRotate * glm::vec4(m_frontDirectionBase, 0.0);
            m_leftDirection = glm::normalize(glm::cross({ 0.0, 1.0, 0.0 }, m_frontDirection));

            glm::mat4 verticalRotate { 1.0f };
            verticalRotate = glm::rotate(verticalRotate, glm::radians(cursorOffset.y), m_leftDirection);
            m_frontDirection = verticalRotate * glm::vec4(m_frontDirection, 0.0);
            m_upDirection = glm::normalize(glm::cross(m_frontDirection, m_leftDirection));

            m_frontDirection = glm::normalize(m_frontDirection);
        }

        /* Movement Control */

        // Caculate delta time to compensate different tick/s caused by machine performace.
        static double lastTimeTick = glfwGetTime();
        double deltaTime = glfwGetTime() - lastTimeTick;
        lastTimeTick = glfwGetTime();

        // Always push friction force if camera is moving.
        glm::vec2 frictionAcc = -m_speed * acceleration / speedMaximum;

        if (glm::length(m_speed) >= 0.01f)
            m_speed += frictionAcc;
        else
            m_speed = glm::vec2 {0.0f};

        m_speed += moveSign * acceleration;

        glm::vec2 movement = m_speed * static_cast<float>(deltaTime);

        position += m_frontDirection * movement.x + m_leftDirection * movement.y;
    }

    void Camera::mouseButtonCallback(GLFWwindow* window, int button, int action) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if (!cursorCaptured) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                setCursorOrigin({ xpos, ypos });
            }
            cursorCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            cursorCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    glm::vec2 Camera::getSpeed() const {
        return m_speed;
    }

    glm::vec3 Camera::getVolecity() const {
        return m_frontDirection * m_speed.x + m_leftDirection * m_speed.y;
    }

    glm::vec3 Camera::getFrontDirecction() const {
        return m_frontDirection;
    }

    glm::vec3 Camera::getLeftDirection() const {
        return m_leftDirection;
    }

    glm::vec3 Camera::getUpDirection() const {
        return m_upDirection;
    }

    glm::mat4 Camera::getLookAt() const {
        return glm::lookAt(position, position + m_frontDirection, m_upDirection);
    }
}