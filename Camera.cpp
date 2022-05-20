#include "Camera.h"

Camera::Camera() {
	// 初始化相机的位置 根据半径仰角旋转角计算
	// 初始化相机位置
	eyex = 0.6f;
	eyey = 0.6f;
	eyez = 2.0f;
	updateCamera();
};
Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix() {
	// 注意到求视图矩阵需要修改
	return this->lookAt(eye, eye + cameraDirection, up);
}

glm::mat4 Camera::getProjectionMatrix(bool isOrtho) {
	if(isOrtho) {
		// 正交投影
		return this->ortho(-scale, scale, -scale, scale, this->zNear, this->zFar);
	} else {
		// 透视投影
		return this->perspective(fov, aspect, this->zNear, this->zFar);
	}
}

glm::mat4 Camera::lookAt(const glm::vec4& eye, const glm::vec4& at, const glm::vec4& up) {
	// 获得相机方向。
	glm::vec4 n = glm::normalize(eye - at);
	// 获得右(x)轴方向。
	glm::vec3 up_3 = up;
	glm::vec3 n_3 = n;
	glm::vec4 u = glm::normalize(glm::vec4(glm::cross(up_3, n_3), 0.0));
	// 获得上(y)轴方向。
	glm::vec3 u_3 = u;
	glm::vec4 v = glm::normalize(glm::vec4(glm::cross(n_3, u_3), 0.0));

	glm::vec4 t = glm::vec4(0.0, 0.0, 0.0, 1.0);
	glm::mat4 c = glm::mat4(u, v, n, t);

	// 处理相机位置向量。
	glm::mat4 p = glm::mat4(1.0f);
	p[0].w = -(eye.x);
	p[1].w = -(eye.y);
	p[2].w = -(eye.z);

	glm::mat4 view = p * c;
	return view;					// 计算最后需要沿-eye方向平移
}

glm::mat4 Camera::ortho(const GLfloat left, const GLfloat right,
	const GLfloat bottom, const GLfloat top,
	const GLfloat zNear, const GLfloat zFar) {

	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 / (right - left);
	c[1][1] = 2.0 / (top - bottom);
	c[2][2] = -2.0 / (zFar - zNear);
	c[3][3] = 1.0;
	c[0][3] = -(right + left) / (right - left);
	c[1][3] = -(top + bottom) / (top - bottom);
	c[2][3] = -(zFar + zNear) / (zFar - zNear);
	return c;
}

glm::mat4 Camera::perspective(const GLfloat fovy, const GLfloat aspect,
	const GLfloat zNear, const GLfloat zFar) {
	// @TODO: Task2:请按照实验课内容补全透视投影矩阵的计算
	GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
	GLfloat right = top * aspect;

	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = zNear / right;
	c[1][1] = zNear / top;
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;
	return c;
}

glm::mat4 Camera::frustum(const GLfloat left, const GLfloat right,
	const GLfloat bottom, const GLfloat top,
	const GLfloat zNear, const GLfloat zFar) {
	// 任意视锥体矩阵
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 * zNear / (right - left);
	c[0][2] = (right + left) / (right - left);
	c[1][1] = 2.0 * zNear / (top - bottom);
	c[1][2] = (top + bottom) / (top - bottom);
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -2.0 * zFar * zNear / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;
	return c;
}


void Camera::updateCamera() {

	// 使用相对于at的角度控制相机的时候，注意在upAngle大于90的时候，相机坐标系的u向量会变成相反的方向，
	// 要将up的y轴改为负方向才不会发生这种问题

	// 也可以考虑直接控制相机自身的俯仰角，
	// 保存up，eye-at 这些向量，并修改这些向量方向来控制
	// 看到这里的有缘人可以试一试

	// 开始设定摄像机的一系列参数

	// 将对应的up反向
	up = glm::vec4(0.0, 1.0, 0.0, 0.0);
	if(upAngle > 90) {
		up.y = -1;
	} else if(upAngle < -90) {
		up.y = -1;
	}

	// 计算欧拉角以确定相机的朝向 cameraDirection表示摄像机的朝向向量 此时计算出来相机的三个坐标轴的朝向
	float cameraDirectionX = -cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	float cameraDirectionY = sin(glm::radians(pitch));
	float cameraDirectionZ = cos(glm::radians(pitch)) * cos(glm::radians(yaw));

	// 确定每时每刻的对应的相机的朝向

	cameraDirection = glm::vec4(cameraDirectionX, cameraDirectionY, cameraDirectionZ, 1.0);

	eye = glm::vec4(eyex, eyey, eyez, 1.0);
}


// 用来改变相机的函数 改变小人的函数写在外面
void Camera::keyboard(int key, int action, int mode) {
	// 键盘事件处理
	// 通过按键改变相机和投影的参数
	if(key == GLFW_KEY_U && mode == 0x0000) {
		rotateAngle += 5.0;
	} else if(key == GLFW_KEY_U && mode == GLFW_MOD_SHIFT) {
		rotateAngle -= 5.0;
	} else if(key == GLFW_KEY_I && mode == 0x0000) {
		upAngle += 5.0;
		if(upAngle > 180)
			upAngle = 180;
	} else if(key == GLFW_KEY_I && mode == GLFW_MOD_SHIFT) {
		upAngle -= 5.0;
		if(upAngle < -180)
			upAngle = -180;
	} else if(key == GLFW_KEY_O && mode == 0x0000) {
		radius -= 0.1;
	} else if(key == GLFW_KEY_O && mode == GLFW_MOD_SHIFT) {
		radius += 0.1;
	}
	// 空格键初始化所有参数
	else if(key == GLFW_KEY_SPACE && mode == 0x0000) {
		radius = 2.0;
		rotateAngle = 0.0;
		upAngle = 0.0;
		fov = 45.0;
		aspect = 1.0;
		scale = 1.5;
	}
}

// 调整旋转 根据鼠标移动 -> 欧拉角
void Camera::mouse(double x, double y) {
	// 调整旋转 根据鼠标移动的变化设置欧拉角 根据欧拉角计算出相机的位置
	yaw += 35 * (x - float(900) / 2.0) / 900;
	yaw = glm::mod(yaw + 180.0f, 360.0f) - 180.0f;    // 取模范围 -180 ~ 180
	pitch += -35 * (y - float(900) / 2.0) / 900;
	// 限制一下pitch角度 防止摄像机穿模
	pitch = glm::clamp(pitch, -89.0f, 10.0f);
}

void Camera::setAtPoint(float x, float y, float z) {
	at = glm::vec4(x, y, z, 1.0);
}
