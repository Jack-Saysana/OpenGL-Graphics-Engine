#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum Camera_Input {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    SPRINT,
    WALK
};

//Starting variables
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 0.1f;

//Class for an FPS-style camera
class Camera
{
    public:
    glm::vec3 pos, front, up, right;
    float yaw, pitch;
    float rate;
    
    Camera();
    glm::mat4 GetViewMatrix();
    void mouseInput(float xOffset, float yOffset);
    void buttonInput(Camera_Input input, float deltaTime);
};

#endif