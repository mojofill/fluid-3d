#include "camera.hpp"

void Camera::init(glm::vec4 pos, glm::vec4 forward, glm::vec4 right, glm::vec4 up) {
    this->pos = pos;
    this->forward = forward;
    this->right = right;
    this->up = up;
}

CameraUBO Camera::toCameraUBO() {
    return (CameraUBO) {
        .pos = pos,
        .forward = forward,
        .right = right,
        .up = up
    };
}

static inline glm::vec3 flat(glm::vec3 v) {
    return glm::normalize(glm::vec3(v.x, v.y, 0));
}

void Camera::rotateAroundLocalRight(float t) {
    glm::vec3 f = glm::vec3(forward);
    glm::vec3 r = glm::normalize(glm::vec3(right));

    glm::mat4 rot = glm::rotate(glm::mat4(1), t, r);

    f = glm::normalize(glm::vec3(rot * glm::vec4(f, 0)));

    glm::vec3 u = glm::normalize(glm::cross(r, f));

    forward = glm::vec4(f, 0);
    up = glm::vec4(u, 0);
    right = glm::vec4(r, 0);
}

void Camera::rotateAroundGlobalUp(float t) {
    glm::vec3 f = glm::vec3(forward);
    glm::vec3 worldUp(0, 0, 1);

    glm::mat4 rot = glm::rotate(glm::mat4(1), t, worldUp);

    f = glm::normalize(glm::vec3(rot * glm::vec4(f, 0)));

    glm::vec3 r = glm::normalize(glm::cross(f, worldUp));
    glm::vec3 u = glm::normalize(glm::cross(r, f));

    forward = glm::vec4(f, 0);
    right = glm::vec4(r, 0);
    up = glm::vec4(u, 0);
}

void Camera::processEvents(GLFWwindow *window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_SPACE)) {
        pos += MOVE_SPEED * glm::vec4(0,0,1,0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
        pos -= MOVE_SPEED * glm::vec4(0,0,1,0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_W)) {
        pos += MOVE_SPEED * glm::vec4(flat(glm::vec3(forward)),0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S)) {
        pos -= MOVE_SPEED * glm::vec4(flat(glm::vec3(forward)),0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_A)) {
        pos -= MOVE_SPEED * glm::vec4(flat(glm::vec3(right)),0) * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_D)) {
        pos += MOVE_SPEED * glm::vec4(flat(glm::vec3(right)),0) * dt;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    float dx = x - lastX;
    float dy = y - lastY;

    dy *= -1;
    dx *= -1;

    dt = 0.01;

    rotateAroundGlobalUp(ROT_SPEED * dx * dt);
    if (-M_PI_2 + 0.01 < pitch + ROT_SPEED * dy * dt && pitch + ROT_SPEED * dy * dt < M_PI_2 - 0.01) {
        rotateAroundLocalRight(ROT_SPEED * dy * dt);
        pitch += ROT_SPEED * dy * dt;
    }

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, width / 2, height / 2);

    lastX = width / 2;
    lastY = height / 2;
}
