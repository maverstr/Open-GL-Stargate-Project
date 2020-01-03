#pragma once

// Std. Includes
#include <vector>

// GL Includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	CAM_FORWARD,
	CAM_BACKWARD,
	CAM_LEFT,
	CAM_RIGHT,
	CAM_UP,
	CAM_DOWN
};

// Default camera values
const GLfloat YAW        =  0.0f;
const GLfloat PITCH      =  0.0f;
const GLfloat SPEED      =  12.0f;
const GLfloat SENSITIVTY =  0.25f;
const GLfloat FOV       =  40.0f;


// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Eular Angles
    GLfloat Yaw;
    GLfloat Pitch;
    // Camera options
    GLfloat MovementSpeed;
    GLfloat MouseSensitivity;
    GLfloat Fov;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), GLfloat yaw = YAW, GLfloat pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Fov(FOV)
    {
        this->Position = position;
        this->WorldUp = up;
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw, GLfloat pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Fov(FOV)
    {
        this->Position = glm::vec3(posX, posY, posZ);
        this->WorldUp = glm::vec3(upX, upY, upZ);
        this->Yaw = yaw;
        this->Pitch = pitch;
        this->updateCameraVectors();
    }

    // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, -(this->Up));
    }

	void copyThisCamera(Camera cam) {
		this->Position = cam.Position;
		this->Front = cam.Front;
		this->Up = cam.Up;
		this->Right = cam.Right;
		this->WorldUp = cam.WorldUp;
		this->Yaw = cam.Yaw;
		this->Pitch = cam.Pitch;
		this->MovementSpeed = cam.MovementSpeed;
		this->MouseSensitivity = cam.MouseSensitivity;
		this->Fov = cam.Fov;
	}

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * deltaTime;
        if (direction == CAM_FORWARD)
            this->Position += this->Front * velocity;
        if (direction == CAM_BACKWARD)
            this->Position -= this->Front * velocity;
        if (direction == CAM_LEFT)
            this->Position -= this->Right * velocity;
        if (direction == CAM_RIGHT)
            this->Position += this->Right * velocity;
		if (direction == CAM_UP)
			this->Position += this->WorldUp * velocity;
		if (direction == CAM_DOWN)
			this->Position -= this->WorldUp * velocity;
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
        this->updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(GLfloat yoffset)
    {
		if (this->Fov >= 25.0f && this->Fov <=90.0f)
            this->Fov -= yoffset;
        if (this->Fov <= 25.0f)
            this->Fov = 25.0f;
        if (this->Fov >= 90.0f)
            this->Fov = 90.0f;
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
		this->updateCameraVectors();
	}

	void updatePositionFPSView(glm::vec3 pos, glm::vec3 frontPos) {
		this->Position = pos;
		this->setInitialLookAt(frontPos);
	}

private:
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        this->Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};