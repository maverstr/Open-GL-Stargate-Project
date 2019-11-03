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
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
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
	Jumper(Model* model, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(1.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY)
    {
		this->thisModel = model;
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateVectors();
    }
    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Jumper_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == FORWARD)
            this->Position += this->Front * velocity;
        if (direction == BACKWARD)
            this->Position -= this->Front * velocity;
        if (direction == LEFT)
            this->Position -= this->Right * velocity;
        if (direction == RIGHT)
            this->Position += this->Right * velocity;
		if (direction == UP)
			this->Position += this->Up * velocity;
		if (direction == DOWN)
			this->Position -= this->Up * velocity;

		if (direction == PITCH_UP) {
			this->Pitch += velocity;
			this->deltaPitch = velocity;
		}
		if (direction == PITCH_DOWN){
			this->Pitch -= velocity;
			this->deltaPitch = -velocity;
		}
		if (direction == ROLL_RIGHT){
			this->Roll += velocity;
			this->deltaRoll = velocity;
		}
		if (direction == ROLL_LEFT){
			this->Roll -= velocity; 
			this->deltaRoll = -velocity;
		}
		if (direction == YAW_RIGHT){
			this->Yaw += velocity;
			this->deltaYaw = velocity;
		}
		if (direction == YAW_LEFT){
			this->Yaw -= velocity;
			this->deltaYaw = -velocity;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateVectors();
    }

	void clearMovement(void) {
		this->Position = glm::vec3(0.0f, 0.0f, 0.0f);
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
//    // Calculates the front vector from the Camera's (updated) Eular Angles
//    void updateVectors()
//    {
//        // Calculate the new Front vector
//        glm::vec3 front, up;
//        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
//        front.y = sin(glm::radians(this->Pitch));
//        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
//        this->Front = glm::normalize(front);
//		
//		up.x = -cos(glm::radians(this->Yaw)) * sin(glm::radians(this->Pitch)) * sin(glm::radians(this->Roll)) - sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Roll));
//		up.y = -sin(glm::radians(this->Yaw)) * sin(glm::radians(this->Pitch)) * sin(glm::radians(this->Roll)) + cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Roll));
//		up.z = cos(glm::radians(this->Pitch)) * sin(glm::radians(this->Roll));
//		cout << "P:"<< this->Pitch << "Y:" << this->Yaw << "R:" << this->Roll << endl;
//		cout << "up: " << endl;
//		cout << up.x << " " << up.y << " " << up.z << endl;
//		// Also re-calculate the Right and Up vector
//		//this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//		//this->Right = glm::normalize(right);
//		this->Up = glm::normalize(up);
//		//this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
//		this->Right = glm::normalize(glm::cross(this->Front, this->Up));
//    }

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateVectors()
	{
		// Calculate the new Front vector
		static glm::vec4 front = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		static glm::vec4 up = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		static glm::vec4 right = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		static glm::mat4 rotMatTotal = glm::mat4(1.0f);

		glm::mat4 rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaRoll), glm::vec3(front));
		rotMatTotal = rotMat * rotMatTotal;
		front = rotMat * front;
		up = rotMat * up;
		right = rotMat * right;

		rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaYaw), glm::vec3(up));
		rotMatTotal = glm::rotate(rotMatTotal, glm::radians(this->deltaYaw), glm::vec3(up));
		front = rotMat * front;
		up = rotMat * up;
		right = rotMat * right;

		rotMat = glm::mat4(1.0f);
		rotMat = glm::rotate(rotMat, glm::radians(this->deltaPitch), glm::vec3(right));
		rotMatTotal = glm::rotate(rotMatTotal, glm::radians(this->deltaPitch), glm::vec3(right));
		front = rotMat * front;
		up = rotMat * up;
		right = rotMat * right;

		this->Front = glm::normalize(front);
		this->Up = glm::normalize(up);
		this->Right = glm::normalize(glm::cross(this->Front, this->Up));
		this->rotMatTotal = rotMatTotal;

		cout << "P:" << this->Pitch << "Y:" << this->Yaw << "R:" << this->Roll << endl;
		cout << "up: " << endl;
		cout << up.x << " " << up.y << " " << up.z << endl;

	}
};