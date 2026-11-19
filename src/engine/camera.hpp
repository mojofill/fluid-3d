#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#define MOVE_SPEED 80.0f
#define ROT_SPEED 0.5f

typedef struct CameraUBO {
    glm::vec4 pos;
    glm::vec4 forward;
    glm::vec4 right;
    glm::vec4 up;
} CameraUBO;

class Camera {
public:
    float lastX;
    float lastY;
    float pitch = 0;
    glm::vec4 pos;
    glm::vec4 forward;
    glm::vec4 right;
    glm::vec4 up;

    void init(glm::vec4 pos, glm::vec4 forward, glm::vec4 right, glm::vec4 up);
    CameraUBO toCameraUBO();
    void processEvents(GLFWwindow *window, float dt);
    void rotateAroundLocalRight(float t);
    void rotateAroundGlobalUp(float t);
};

#endif