#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"
#include "Model.hpp"

enum Jumper_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	PITCH_UP,
	PITCH_DOWN,
	ROLL_RIGHT,
	ROLL_LEFT,
	YAW_LEFT,
	YAW_RIGHT
};

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Jumper
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec4 Front;
    glm::vec4 Up;
    glm::vec4 Right;
    glm::vec3 WorldUp;
	Model *thisModel;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
	GLfloat Roll;
	GLfloat deltaYaw;
	GLfloat deltaPitch;
	GLfloat deltaRoll;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Fov;

	glm::mat4 rotMatTotal;


    // Constructor with vectors
	Jumper(Model* model, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY)
    {
		this->thisModel = model;
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
		this->rotMatTotal = glm::mat4(1.0f);
		this->Front = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		this->Up = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		this->Right = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        this->updateVectors();
    }
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Jumper_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
		GLfloat turnVelocity = this->MovementSpeed * deltaTime * 5; //slightly faster for turning
        if (direction == FORWARD)
            this->Position += glm::vec3(this->Front) * velocity;
        if (direction == BACKWARD)
            this->Position -= glm::vec3(this->Front) * velocity;
        if (direction == LEFT)
            this->Position -= glm::vec3(this->Right) * velocity;
        if (direction == RIGHT)
            this->Position += glm::vec3(this->Right) * velocity;
		if (direction == UP)
			this->Position += glm::vec3(this->Up) * velocity;
		if (direction == DOWN)
			this->Position -= glm::vec3(this->Up) * velocity;

		if (direction == PITCH_UP) {
			this->Pitch += turnVelocity;
			this->deltaPitch = turnVelocity;
		}
		if (direction == PITCH_DOWN){
			this->Pitch -= turnVelocity;
			this->deltaPitch = -turnVelocity;
		}
		if (direction == ROLL_RIGHT){
			this->Roll += turnVelocity;
			this->deltaRoll = turnVelocity;
		}
		if (direction == ROLL_LEFT){
			this->Roll -= turnVelocity;
			this->deltaRoll = -turnVelocity;
		}
		if (direction == YAW_RIGHT){
			this->Yaw += turnVelocity;
			this->deltaYaw = turnVelocity;
		}
		if (direction == YAW_LEFT){
			this->Yaw -= turnVelocity;
			this->deltaYaw = -turnVelocity;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateVectors();
    }

	void clearMovement(void) {
		this->deltaPitch = 0;
		this->deltaRoll = 0;
		this->deltaYaw = 0;
	}

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Eular angles
        this->updateVectors();
    }

	void setInitialLookAt(glm::vec3 lookAtPos) {
		if ((lookAtPos.x - this->Position.x) == 0.0f) { //avoid division by zero
			lookAtPos.x = 0.01f + this->Position.x;
		}
		if ((lookAtPos.z - this->Position.z) == 0.0f) { //avoid division by zero
			lookAtPos.z = 0.01f + this->Position.z;

		}
		if ((lookAtPos.y - this->Position.y) == 0.0f) { //avoid division by zero
			lookAtPos.y = 0.01f + this->Position.y;
		}
		lookAtPos = glm::normalize(lookAtPos  - this->Position);
		this->Pitch = glm::degrees(asin(lookAtPos.y));
		this->Yaw = glm::degrees(atan2(lookAtPos.z, lookAtPos.x));
		this->updateVectors();
	}

	void updatePosition(glm::vec3 pos) {
		this->Position = pos;
	}

private:

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateVectors()
	{
		// Calculate the new Front vector

		glm::mat4 rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaYaw), glm::vec3(this->Up));
		this->Front = rotMat * this->Front;
		this->Up = rotMat * this->Up;
		this->Right = rotMat * this->Right;

		rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaPitch), glm::vec3(this->Right));
		this->Front = rotMat * this->Front;
		this->Up = rotMat * this->Up;
		this->Right = rotMat * this->Right;

		rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaRoll), glm::vec3(this->Front));
		this->Front = rotMat * this->Front;
		this->Up = rotMat * this->Up;
		this->Right = rotMat * this->Right;

		this->Front = glm::normalize(this->Front); // glm::normalize(front);
		this->Up = glm::normalize(this->Up); // glm::normalize(up);
		this->Right = glm::vec4(glm::normalize(glm::cross(glm::vec3(this->Front), glm::vec3(this->Up))),1.0f);
		this->rotMatTotal = rotMatTotal;

		/* build a coordinate system change matrix from */
		this->rotMatTotal = glm::mat4(1.0f);
		this->rotMatTotal[0] = this->Right;
		this->rotMatTotal[1] = this->Up;
		this->rotMatTotal[2] = this->Front;
	}
};