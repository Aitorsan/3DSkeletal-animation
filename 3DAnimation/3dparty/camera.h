#pragma once
#include <vector>
#include <glm/glm.hpp>
struct GLFWwindow;
class Camera
{
	glm::vec3 CameraFront{};
	glm::vec3 CameraPos{};
	glm::vec3 CameraUp{};
	glm::vec3 CameraRight{};
	float yaw{};
	float pitch{};
	float lastX{};
	float lastY{};

public:
	//camera data 
	Camera();
	Camera(float x, float y, float z, float fov);
	//void MouseMoved( double xpos, double ypos);
	void CheckMouseMovement(GLFWwindow& window,const glm::vec3& target);
	void CheckMouseMovement(GLFWwindow& window);
	void MoveFront(float elapsedTime, float velocity);
	void MoveBack(float elapsedTime, float velocity);
	void MoveRight(float elapsedTime, float velocity);
	void MoveLeft(float elapsedTime, float velocity);
	glm::mat4 GetCameraTranslationMatrix();
	glm::vec3& GetCameraPosition();
	glm::vec3 & GetCameraFront();
	void setCameraFront(const glm::vec3& front);
	void SetCameraPosition(float x, float y, float z);
	void SetCameraPosition( const glm::vec3& pos);


	float Distance{};
	bool init{};
};