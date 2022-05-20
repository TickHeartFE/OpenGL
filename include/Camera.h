#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Angel.h"

class Camera {
public:
	Camera();
	~Camera();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix(bool isOrtho);

	glm::mat4 lookAt(const glm::vec4& eye, const glm::vec4& at, const glm::vec4& up);

	glm::mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 perspective(const GLfloat fov, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 frustum(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	// 每次更改相机参数后更新一下相关的数值
	void updateCamera();
	// 处理相机的键盘操作
	void keyboard(int key, int x, int y);

	// 处理相机的鼠标操作
	void mouse(double x, double y);
	// 设置相机指向的目标
	void setAtPoint(float x, float y, float z);

	// 模视矩阵
	glm::mat4 viewMatrix;
	glm::mat4 projMatrix;

	// 相机位置参数
	float radius = 1.5;
	float rotateAngle = 220.0;	// 调整好摄像机的角度
	float upAngle = 0.0;
	glm::vec4 eye;
	glm::vec4 at = { 0.0,0.0,0.0,1.0 };
	glm::vec4 up;

	// 投影参数
	float zNear = 0.1;
	float zFar = 100.0;
	// 透视投影参数
	float fov = 45.0;
	float aspect = 1.0;
	// 正交投影参数
	float scale = 1.5;

	// 目标位置
	float atx = 0.0, aty = 0.0, atz = 0;
	// 摄像机位置
	float eyex, eyey, eyez;

	//pitch 俯仰角 yaw 偏航角 roll 横滚角
	float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;

	// 欧拉角计算的相机的朝向
	glm::vec4 cameraDirection;


	// 相机的前移参数 
	float moveForward = 0;

};
#endif