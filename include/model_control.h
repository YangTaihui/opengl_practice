#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for Model movement. Used as abstraction to stay away from window-system specific input methods
enum Model_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    RESET
};

// Default Model values
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 100.0f;
const float ZOOM = 45.0f;

// An abstract Model class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class ModelControl
{
public:
    // euler Angles
    float Yaw;
    float Pitch;
    // Model options
    float MovementSpeed;
    float Zoom;
    glm::mat4 Model;
    glm::mat4 tran1, tran2, one = glm::mat4(1.0f);

    ModelControl(float yaw = YAW, float pitch = PITCH): MovementSpeed(SPEED), Zoom(ZOOM)
    {
        Yaw = yaw;
        Pitch = pitch;
        Model = one;
        updateModelVectors();
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of Model defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Model_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) {
            Pitch += velocity;
            if (Pitch > 90.0)
                Pitch = 90.0;
        }
        if (direction == BACKWARD) {
            Pitch -= velocity;
            if (Pitch < -90.0)
                Pitch = -90.0;
        }
        if (direction == LEFT) {
            Yaw -= velocity;
        }
        if (direction == RIGHT) {
            Yaw += velocity;
        }
        if (direction == RESET) {
            Pitch = 0.0;
            Yaw = 0.0;
        }

        updateModelVectors();
    }

private:
    // calculates the front vector from the Model's (updated) Euler Angles
    void updateModelVectors()
    {
        tran1 = glm::rotate(one, glm::radians(Pitch), glm::vec3(1.0, 0.0, 0.0));
        tran2 = glm::rotate(one, glm::radians(Yaw), glm::vec3(0.0, 1.0, 0.0));
        Model = tran2*tran1*one;
    }
};
#endif#pragma once
