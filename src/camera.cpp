#include <Camera/camera.h>

Camera::Camera()
{
    yaw = YAW;
    pitch = PITCH;
    pos = glm::vec3(0.0f, 0.0f, 0.0f);
    front = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    rate = 2.5f;
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(pos, pos + front, up);
}

void Camera::mouseInput(float xOffset, float yOffset)
{
    xOffset *= SENSITIVITY;
    yOffset *= SENSITIVITY;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    front = glm::normalize(glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
}

void Camera::buttonInput(Camera_Input input, float deltaTime)
{
    float speed = rate * deltaTime;
    if(input == SPRINT)
    {
        rate = 10.0f;
    }

    if(input == WALK)
    {
        rate = 2.0f;
    }

    if(input == FORWARD)
    {
        pos += speed * front;
    }
    
    if(input == BACKWARD)
    {
        pos -= speed * front;
    }
    
    if(input == LEFT)
    {
        pos -= speed * glm::normalize(glm::cross(front, up));
    }
    
    if(input == RIGHT)
    {
        pos += speed * glm::normalize(glm::cross(front, up));
    }
}