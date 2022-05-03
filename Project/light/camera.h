#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

enum Camera_Movement{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.01f;
const float FOV = 45.0f;

class Camera{
	public:
		glm::vec3 position;
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		glm::vec3 worldUp;
		
		float yaw;
		float pitch;

		float movementSpeed;
		float mouseSensitivity;
		float fov;

		Camera(glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f),glm::vec3 worldUp = glm::vec3(0.0f,1.0f,0.0f),float yaw = YAW,float pitch = PITCH) : front(glm::vec3(0.0f,0.0f,-1.0f)),movementSpeed(SPEED),mouseSensitivity(SENSITIVITY),fov(FOV){
			this->position = position;
			this->worldUp = worldUp;
			this->yaw = yaw;
			this->pitch = pitch;
			updateCameraVectors();
		}

		Camera(float  posX,float posY,float posZ,float upX,float upY,float upZ,float yaw,float pitch) : front(glm::vec3(0.0f,0.0f,-1.0f)),movementSpeed(SPEED),mouseSensitivity(SENSITIVITY),fov(FOV){
			this->position = glm::vec3(posX,posY,posZ);
			this->worldUp = glm::vec3(upX,upY,upZ);
			this->yaw = yaw;
			this->pitch = pitch;
			updateCameraVectors();
		}

		glm::mat4 getViewMatrix(){
			//return glm::lookAt(position,position + front ,up);
			return createLookAt(position,position + front,up);
		}

		void processKeyboard(Camera_Movement direction,float deltaTime){
			float velocity = movementSpeed * deltaTime;
			if(direction == FORWARD)
				position += front * velocity;
			if(direction == BACKWARD)
				position -= front * velocity;
			if(direction == LEFT)
				position -= right * velocity;
			if(direction == RIGHT)
			        position += right * velocity;
			//position.y = 0.0f;
		}

		void processMouseMovement(float xoffset,float yoffset,GLboolean constrainPitch = true){
			xoffset *= mouseSensitivity;
			yoffset *= mouseSensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if(constrainPitch){
				if(pitch > 89.0f)pitch = 89.0f;
				if(pitch < -89.0f) pitch = -89.0f;
			}

			updateCameraVectors();
		}

		void processMouseScroll(float yoffset){
			fov -= (float) yoffset;
			if(fov < 1.0f) fov = 1.0f;
			if(fov > 45.0f) fov = 45.0f;
		}

	private:
		void updateCameraVectors(){
			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			this->front = glm::normalize(front);
			this->right = glm::normalize(glm::cross(this->front,this->worldUp));
			this->up = glm::normalize(glm::cross(this->right,this->front));
		}


		glm::mat4 createLookAt(glm::vec3 position,glm::vec3 target,glm::vec3 up){
			glm::vec3 zaxis = glm::normalize(position - target);
			glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(up),zaxis));
			glm::vec3 yaxis = glm::cross(zaxis,xaxis);
			glm::mat4 rotationMatrix = glm::mat4(1.0f);
			rotationMatrix[0][0]= xaxis.x;
			rotationMatrix[1][0]= xaxis.y;
			rotationMatrix[2][0]= xaxis.z;
			rotationMatrix[0][1]= yaxis.x;
			rotationMatrix[1][1]= yaxis.y;
			rotationMatrix[2][1]= yaxis.z;
			rotationMatrix[0][2]= zaxis.x;
			rotationMatrix[1][2]= zaxis.y;
			rotationMatrix[2][2]= zaxis.z;
			glm::mat4 positionMatrix = glm::mat4(1.0f);
			positionMatrix[3][0] = -position.x;
			positionMatrix[3][1] = -position.y;
			positionMatrix[3][2] = -position.z;
			return rotationMatrix * positionMatrix;
		}

};
#endif
