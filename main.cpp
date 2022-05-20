#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>

#include <vector>
#include <string>

using namespace std;

// 窗口变量
GLFWwindow* window;

int WIDTH = 900;
int HEIGHT = 900;

TriMesh* skyBox;
TriMesh* castle;
TriMesh* landObj;
TriMesh* oldMan;
TriMesh* landMode;
TriMesh* test;
TriMesh* cylinder;
TriMesh* cylinder2;

// 机器人朝向的方向
int direction = 0;

// 动画的关键帧
int state = 0;
int state2 = 0;
int state3 = 0;
int state4 = 0;

// 维护一个变量记录人物的变身状态
int condition = 0;// 用来记录是公主态还是老人状态

// 维护一个敌人的数量
int enemyCount = 5;

int mainWindow;

Camera* camera = new Camera();
Light* light = new Light();	// 定义灯光
MeshPainter* painter = new MeshPainter();

// 天空盒子贴图容器
vector<std::string> faces
{
	"right.jpg",
	"left.jpg",
	"top.jpg",
	"bottom.jpg",
	"front.jpg",
	"back.jpg"
};

glm::vec3 translateTheta(0.0, 0.0, 0.0);	//平移控制变量
// 这个用来回收和删除我们创建的物体对象
std::vector<TriMesh*> meshList;

int meshIndex = 0;
TriMesh* mesh;

// 定义颜色
#define White	glm::vec3(1.0, 1.0, 1.0)
#define Yellow	glm::vec3(1.0, 1.0, 0.0)
#define Green	glm::vec3(0.0, 1.0, 0.0)
#define Cyan	glm::vec3(0.0, 1.0, 1.0)
#define Magenta	glm::vec3(1.0, 0.0, 1.0)
#define Red		glm::vec3(1.0, 0.0, 0.0)
#define Black	glm::vec3(0.0, 0.0, 0.0)
#define Blue	glm::vec3(0.0, 0.0, 1.0)
#define Brown	glm::vec3(0.5, 0.5, 0.5)


// 定义一个机器人结构体 将对应的参数进行初始化
struct Robot {
	// 关节大小 对应的关节大小
	float BODY_HEIGHT = 1.0;
	float BODY_WIDTH = 1.0;
	float UPPER_ARM_HEIGHT = 0.23;
	float LOWER_ARM_HEIGHT = 0.23;
	float UPPER_ARM_WIDTH = 0.25;
	float LOWER_ARM_WIDTH = 0.25;
	float UPPER_LEG_HEIGHT = 0.23;
	float LOWER_LEG_HEIGHT = 0.23;
	float UPPER_LEG_WIDTH = 0.25;
	float LOWER_LEG_WIDTH = 0.25;
	float HEAD_HEIGHT = 1.0;
	float HEAD_WIDTH = 1.0;
	float SWORD_HEIGHT = 1.0;
	float SWORD_WIDTH = 1.0;

	// 特别注意的是
	enum {
		Body,		   // 躯干
		Head,		   // 头部
		RightArm,	   // 右臂
		RightSmallArm, // 右小臂
		LeftArm,	   // 左臂
		LeftSmallArm,  // 左小臂
		RightLeg,	   // 右大腿
		RightSmallLeg, // 右小腿
		LeftLeg,	   // 左大腿
		LeftSmallLeg,  // 左小腿
		shell,		   // 盾牌
		sword,         // 剑
	};
	// 各个部位关节角大小  在Y轴上的旋转
	GLfloat theta[12] = {
		0.0, // Body
		0.0, // Head
		0.0, // RightArm
		0.0, // RightSmallArm
		0.0, // LeftArm
		0.0, // LeftSmallArm
		0.0, // RightLeg
		0.0, // RightSmallLeg
		0.0, // LeftLeg
		0.0, // LeftSmallLeg
		0.0, // shell
		0.0	 // sword
	};
	// 各个部位与关节在x轴上的旋转角 
	GLfloat thetaX[12]{
		0.0, // Body
		0.0, // Head
		0.0, // RightArm
		0.0, // RightSmallArm
		0.0, // LeftArm
		0.0, // LeftSmallArm
		0.0, // RightLeg
		0.0, // RightSmallLeg
		0.0, // LeftLeg
		0.0, // LeftSmallLeg
		0.0, // shell
		0.0  // sword
	};
	// 各个部位与关节在z轴上的旋转角 
	GLfloat thetaZ[12]{
		0.0, // Body	
		0.0, // Head
		0.0, // RightArm
		0.0, // RightSmallArm
		0.0, // LeftArm
		0.0, // LeftSmallArm
		0.0, // RightLeg
		0.0, // RightSmallLeg
		0.0, // LeftLeg
		0.0, // LeftSmallLeg
		0.0, // shell
		0.0  // sword
	};
};

struct  EnemyRobot {
	float BODY_HEIGHT = 1.0;
	float BODY_WIDTH = 1.0;
	float ARM_HEIGHT = 0.2;
	float ARM_WIDTH = 0.2;
	float LEG_HEIGHT = 0.2;
	float LEG_WIDTH = 0.2;

	// 枚举
	enum {
		Body,
		leftArm,
		rightArm,
		leftLeg,
		rightLeg
	};
	GLfloat theta[5]{
		0.0,	// Body
		// 四肢的旋转角
		0.0,	// leftArm
		0.0,	// rightArm
		0.0,	// LeftLeg
		0.0		// rightLeg
	};
};


// 定义一个矩阵栈 对矩阵进行对应的栈操作 自定的一个连续栈操作
class MatrixStack {
	int _index;	// 表示现在栈中含有的元素的数量
	int _size;
	glm::mat4* _matrices;
public:
	// 矩阵栈的构造函数
	MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices) {
		// 初始化index 和 size
		_matrices = new glm::mat4[numMatrices];	// 指向容量为100的矩阵数组
	}
	~MatrixStack() {
		delete[] _matrices;
	}
	// push函数 将一个矩阵压入到栈中
	void push(const glm::mat4& m) {
		// asssert 检查是否有超过容量大小
		assert(_index + 1 < _size);
		_matrices[_index++] = m;
	}
	// pop函数 在栈中弹出一个元素
	glm::mat4& pop() {
		// 同理检查合法性
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};

Robot robot;
EnemyRobot enemy;
// 定义一个机器人 本人控制
vector<TriMesh*> Man;
// 定义敌人 随机移动
vector<TriMesh*> Enemy;
vector<TriMesh*> Enemy2;
vector<TriMesh*> Enemy3;
vector<TriMesh*> Enemy4;
vector<TriMesh*> Enemy5;



// 定义对应的物体部位 一共十个部分进行绘制
openGLObject BodyObject;
openGLObject HeadObject;
openGLObject RightUpperArmObject;
openGLObject RightLowerArmObject;
openGLObject LeftUpperArmObject;
openGLObject LeftLowerArmObject;
openGLObject RightUpperLegObject;
openGLObject RightLowerLegObject;
openGLObject LeftUpperLegObject;
openGLObject LeftLowerLegObject;
openGLObject shellObject;
openGLObject swordObject;


openGLObject EnemyBodyObject;
openGLObject EnemyleftArmObject;
openGLObject EnemyrightArmObject;
openGLObject EnemyleftLegObject;
openGLObject EnemyrightLegObject;


openGLObject EnemyBodyObject2;
openGLObject EnemyleftArmObject2;
openGLObject EnemyrightArmObject2;
openGLObject EnemyleftLegObject2;
openGLObject EnemyrightLegObject2;

openGLObject cycleObject;

int selectedMesh = robot.Body;	// 维护对应的选择部位

const int manNum = 12;	// 对应的存储部位 一共分为了6部分
const int enemyNum = 5; // 敌人一共层次建模了五部分

// 头部
glm::mat4 head(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.25 * robot.HEAD_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.HEAD_WIDTH, robot.HEAD_HEIGHT, robot.HEAD_WIDTH));
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 身体
glm::mat4 body(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, robot.BODY_HEIGHT * 0.5, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.BODY_WIDTH, robot.BODY_HEIGHT, robot.BODY_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体 注意是左乘
	return modelMatrix * instance;
}

// 身体
glm::mat4 bodyEnemy(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, enemy.BODY_HEIGHT * 0.5, 0.0));
	instance = glm::scale(instance, glm::vec3(enemy.BODY_WIDTH, enemy.BODY_HEIGHT, enemy.BODY_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体 注意是左乘
	return modelMatrix * instance;
}


// 左大手臂
glm::mat4 Left_Big_Arm(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(-0.2, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体 
	return modelMatrix * instance;
}

glm::mat4 LeftArmEnemy(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(enemy.ARM_WIDTH, enemy.ARM_HEIGHT, enemy.ARM_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体 
	return modelMatrix * instance;
}

// 左小手臂
glm::mat4 Left_Small_Arm(glm::mat4 modelMatrix) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(-0.1, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右大手臂
glm::mat4 Right_Big_Arm(glm::mat4 modelMatrix) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.2, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_ARM_WIDTH, robot.UPPER_ARM_HEIGHT, robot.UPPER_ARM_WIDTH));
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右大手臂
glm::mat4 RightArmEnemy(glm::mat4 modelMatrix) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(enemy.ARM_WIDTH, enemy.ARM_HEIGHT, enemy.ARM_WIDTH));
	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右小手臂
glm::mat4 Right_Small_Arm(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.1, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_ARM_WIDTH, robot.LOWER_ARM_HEIGHT, robot.LOWER_ARM_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右手剑
glm::mat4 sword(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.1, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.SWORD_WIDTH, robot.SWORD_HEIGHT, robot.SWORD_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 左大腿
glm::mat4 Left_Big_Leg(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	// 旋转中心点位置在此
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 左大腿
glm::mat4 LeftLegEnemy(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	// 旋转中心点位置在此
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(enemy.LEG_WIDTH, enemy.LEG_HEIGHT, enemy.LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 左小腿
glm::mat4 Left_Small_Leg(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右大腿
glm::mat4 Right_Big_Leg(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.UPPER_LEG_WIDTH, robot.UPPER_LEG_HEIGHT, robot.UPPER_LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 左大腿
glm::mat4 RightLegEnemy(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	// 旋转中心点位置在此
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.UPPER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(enemy.LEG_WIDTH, enemy.LEG_HEIGHT, enemy.LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 右小腿
glm::mat4 Right_Small_Leg(glm::mat4 modelMatrix) {
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.5 * robot.LOWER_LEG_HEIGHT, 0.0));
	instance = glm::scale(instance, glm::vec3(robot.LOWER_LEG_WIDTH, robot.LOWER_LEG_HEIGHT, robot.LOWER_LEG_WIDTH));

	// 乘以来自父物体的模型变换矩阵，绘制当前物体
	return modelMatrix * instance;
}

// 设置关键帧 传入关键帧参数 然后根据关键帧来改变人物的动作 关键帧存储人物的角度
void RobotArmAnim(int i) {
	if(i % 20 <= 5) {
		// 改变角度
		// 同时让头也可以移动起来
		robot.theta[robot.Head] += 3.0;
		robot.theta[robot.RightArm] += 10.0;
		robot.theta[robot.LeftArm] += 10.0;
		robot.thetaX[robot.LeftLeg] += 10.0;
		robot.thetaX[robot.RightLeg] -= 10.0;
	} else if(i % 20 > 5 && i % 20 <= 15) {
		// 改变角度
		robot.theta[robot.Head] -= 3.0;
		robot.theta[robot.RightArm] -= 10.0;
		robot.theta[robot.LeftArm] -= 10.0;
		robot.thetaX[robot.LeftLeg] -= 10.0;
		robot.thetaX[robot.RightLeg] += 10.0;
	} else if(i % 20 > 15 && i % 20 <= 20) {
		// 改变角度 
		robot.theta[robot.Head] += 3.0;
		robot.theta[robot.RightArm] += 10.0;
		robot.theta[robot.LeftArm] += 10.0;
		robot.thetaX[robot.LeftLeg] += 10.0;
		robot.thetaX[robot.RightLeg] -= 10.0;
	}
}

void EnemyAnim(int i) {
	if(i % 20 <= 5) {
		// 改变角度
		enemy.theta[enemy.leftArm] += 10.0;
		enemy.theta[enemy.rightArm] -= 10.0;
		enemy.theta[enemy.leftLeg] += 10.0;
		enemy.theta[enemy.rightLeg] -= 10.0;
	} else if(i % 20 > 5 && i % 20 <= 15) {
		// 改变角度
		enemy.theta[enemy.leftArm] -= 10.0;
		enemy.theta[enemy.rightArm] += 10.0;
		enemy.theta[enemy.leftLeg] -= 10.0;
		enemy.theta[enemy.rightLeg] += 10.0;
	} else if(i % 20 > 15 && i % 20 <= 20) {
		// 改变角度 
		enemy.theta[enemy.leftArm] += 10.0;
		enemy.theta[enemy.rightArm] -= 10.0;
		enemy.theta[enemy.leftLeg] += 10.0;
		enemy.theta[enemy.rightLeg] -= 10.0;
	}
}

// 设置挥动剑的动作函数 绕着z轴进行旋转
void swordMove(int i) {
	if(i % 20 <= 5) {
		robot.thetaZ[robot.LeftArm] += 10.0;
	} else if(i % 20 > 5 && i % 20 <= 15) {
		robot.thetaZ[robot.LeftArm] -= 10.0;
	} else if(i % 20 > 15 && i % 20 <= 20) {
		// 同理改变角度
		robot.thetaZ[robot.LeftArm] += 10.0;
	}
}
// 绘制天空盒子 先构造对应的函数
unsigned int loadCubemap(vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for(unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if(data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		} else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

const double NomalizeUp = 0.25;


void setObj() {
	std::string vshader, fshader;
	// 读取着色器并使用
#ifdef __APPLE__	// for MacOS
	vshader = "shaders/vshader_mac.glsl";
	fshader = "shaders/fshader_mac.glsl";
#else				// for Windows
	vshader = "shaders/vshader_win.glsl";
	fshader = "shaders/fshader_win.glsl";
#endif
	// 设置光源位置
	light->setTranslation(glm::vec3(1.0, 3.0, 2.0));
	light->setAmbient(glm::vec4(0.3, 0.3, 0.3, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(0.5, 0.5, 0.5, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(0.7, 0.7, 0.7, 1.0)); // 镜面反射
	light->setAttenuation(1.0, 0.045, 0.0075); // 衰减系数
	// 绘制天空盒子
	skyBox = new TriMesh();
	skyBox->setNormalize(true);
	skyBox->readObj("./assets/skybox2.obj");
	// 设置对应的物体的旋转位移
	// 设置物体的旋转位移
	skyBox->setTranslation(glm::vec3(0.0, 1.5, 0.0));
	skyBox->setRotation(glm::vec3(0.0, 0.0, 0.0));
	skyBox->setScale(glm::vec3(110.0, 110.0, 110.0));
	skyBox->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
	skyBox->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
	skyBox->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	skyBox->setShininess(1.0);						  //高光系数
	painter->addMesh(skyBox, "noShadow", "./assets/skybox2.png", vshader, fshader);
	painter->drawMeshes(light, camera);
	// 绘制城堡
	castle = new TriMesh();
	castle->setNormalize(true);
	castle->readObj("./assets/monu2.obj");
	// 设置对应的物体的旋转位移
	// 设置物体的旋转位移
	castle->setTranslation(glm::vec3(0.0, 1.7, -3.0));
	castle->setRotation(glm::vec3(0.0, 0.0, 0.0));
	castle->setScale(glm::vec3(6.0, 6.0, 6.0));
	castle->setAmbient(glm::vec4(0.1, 0.1, 0.1, 1.0));  // 环境光
	castle->setDiffuse(glm::vec4(0.4, 0.4, 0.4, 1.0));  // 漫反射
	castle->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	castle->setShininess(1.0);						  //高光系数
	painter->addMesh(castle, "hasShadow", "./assets/monu2.png", vshader, fshader);
	painter->drawMeshes(light, camera);

	// 绘制地面装饰
	landObj = new TriMesh();
	landObj->setNormalize(true);
	landObj->readObj("./assets/landObj.obj");
	// 设置对应的物体的旋转位移
	// 设置物体的旋转位移
	landObj->setTranslation(glm::vec3(-3.0, 0.56, 0.0));
	landObj->setRotation(glm::vec3(0.0, 0.0, 0.0));
	landObj->setScale(glm::vec3(20.0, 20.0, 20.0));
	landObj->setAmbient(glm::vec4(0.1, 0.1, 0.1, 1.0));  // 环境光
	landObj->setDiffuse(glm::vec4(0.4, 0.4, 0.4, 1.0));  // 漫反射
	landObj->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	landObj->setShininess(1.0);						  //高光系数
	painter->addMesh(landObj, "hasShadow", "./assets/landObj.png", vshader, fshader);
	painter->drawMeshes(light, camera);


	// 读取老人模型
	oldMan = new TriMesh();
	oldMan->setNormalize(true);
	oldMan->readObj("./assets/chr_old.obj");
	// 设置对应的物体的旋转位移
	// 设置物体的旋转位移
	oldMan->setTranslation(glm::vec3(0.0, 0.118, 0.0));
	oldMan->setRotation(glm::vec3(0.0, 0.0, 0.0));
	oldMan->setScale(glm::vec3(0.3, 0.3, 0.3));
	oldMan->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
	oldMan->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
	oldMan->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	oldMan->setShininess(1.0);						  //高光系数


	// 加到painter中
	painter->addMesh(oldMan, "hasShadow", "./assets/chr_old.png", vshader, fshader);

	// 读取地图模型
	landMode = new TriMesh;
	landMode->setNormalize(true);
	landMode->readObj("./assets/gameLand.obj");
	// 设置对应的物体的旋转位移
	// 设置物体的旋转位移
	landMode->setTranslation(glm::vec3(0.0, -0.075, 0.0));
	landMode->setRotation(glm::vec3(0.0, 0.0, 0.0));
	// 地面尽量弄成无限大 防止人物出界
	landMode->setScale(glm::vec3(100.0, 10.0, 100.0));
	landMode->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
	landMode->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
	landMode->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	landMode->setShininess(1.0);						  //高光系数
	// 加到painter中
	painter->addMesh(landMode, "noShadow", "./assets/gameLand.png", vshader, fshader);

	// 绘制圆柱体
	cylinder = new TriMesh();
	// 创建圆柱体 
	cylinder->generateCylinder(100, 0.1, 0.3);
	cylinder->setNormalize(true);
	cylinder->setTranslation(glm::vec3(-2.0, 0.5, -2.5));
	cylinder->setRotation(glm::vec3(90.0, 0.0, 0.0));
	cylinder->setScale(glm::vec3(1.0, 1.0, 1.0));
	cylinder->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
	cylinder->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
	cylinder->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	cylinder->setShininess(1.0);
	// 加到painter中
	painter->addMesh(cylinder, "hasShadow", "./assets/cylinder10.jpg", vshader, fshader); 	// 指定纹理与着色器

	// 绘制圆柱体 并对其添加纹理
	cylinder2 = new TriMesh();
	// 创建圆柱体 
	cylinder2->generateCylinder(100, 0.1, 0.3);
	cylinder2->setNormalize(true);
	cylinder2->setTranslation(glm::vec3(2.0, 0.45, -2.5));
	cylinder2->setRotation(glm::vec3(90.0, 0.0, 0.0));
	cylinder2->setScale(glm::vec3(1.0, 1.0, 1.0));
	cylinder2->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
	cylinder2->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
	cylinder2->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
	cylinder2->setShininess(1.0);
	// 加到painter中
	painter->addMesh(cylinder2, "hasShadow", "./assets/cylinder10.jpg", vshader, fshader); 	// 指定纹理与着色器s
}

void setRobot() {
	std::string vshader, fshader;
	// 读取着色器并使用
#ifdef __APPLE__	// for MacOS
	vshader = "shaders/vshader_mac.glsl";
	fshader = "shaders/fshader_mac.glsl";
#else				// for Windows
	vshader = "shaders/vshader_win.glsl";
	fshader = "shaders/fshader_win.glsl";
#endif
	for(int i = 0; i < manNum; i++) {
		TriMesh* Mesh = new TriMesh();

		// 设定好初始位置 初始化所有层级建模的部位 初始化所有部位的属性
		Mesh->setTranslation(glm::vec3(1.0, 0.005, 1.0));
		Mesh->setRotation(glm::vec3(0.0, 0.0, 0.0));
		// 设置物体的初始大小
		Mesh->setScale(glm::vec3(0.3, 0.3, 0.3));
		Mesh->setNormalize(true);
		Mesh->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
		Mesh->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
		Mesh->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
		Mesh->setShininess(1.0);						  //高光系数
		Man.push_back(Mesh);
	}

	// 读取对应的obj文件 将对应部分的obj全部读取到对应的Trimesh指针数组中
	Man[robot.Body]->readObj("./assets/body.obj");
	Man[robot.Head]->readObj("./assets/head.obj");
	Man[robot.RightArm]->readObj("./assets/arm.obj");
	Man[robot.RightSmallArm]->readObj("./assets/arm.obj");
	Man[robot.LeftArm]->readObj("./assets/arm.obj");
	Man[robot.LeftSmallArm]->readObj("./assets/arm.obj");
	Man[robot.RightLeg]->readObj("./assets/leg.obj");
	Man[robot.RightSmallLeg]->readObj("./assets/rightLeg.obj");
	Man[robot.LeftLeg]->readObj("./assets/leg.obj");
	Man[robot.LeftSmallLeg]->readObj("./assets/leftLeg.obj");
	// 将剑和盾牌读进模型中
	Man[robot.shell]->readObj("./assets/shell.obj");
	Man[robot.sword]->readObj("./assets/sword.obj");


	// 对对应的位置进行绑定 传入对应的物体进行一个绑定
	painter->bindObjectAndData(Man[robot.Body], BodyObject, "./assets/body.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.Head], HeadObject, "./assets/head.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.RightArm], RightUpperArmObject, "./assets/arm2.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.RightSmallArm], RightLowerArmObject, "./assets/arm.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.LeftArm], LeftUpperArmObject, "./assets/arm.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.LeftSmallArm], LeftLowerArmObject, "./assets/arm.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.RightLeg], RightUpperLegObject, "./assets/rightLeg.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.RightSmallLeg], RightLowerLegObject, "./assets/arm.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.LeftLeg], LeftUpperLegObject, "./assets/leftLeg.png", vshader, fshader);
	painter->bindObjectAndData(Man[robot.LeftSmallLeg], LeftLowerLegObject, "./assets/arm.png", vshader, fshader);
}

void setEnemy(vector<TriMesh*>& Enemy, glm::vec3& position) {
	std::string vshader, fshader;
	// 读取着色器并使用
#ifdef __APPLE__	// for MacOS
	vshader = "shaders/vshader_mac.glsl";
	fshader = "shaders/fshader_mac.glsl";
#else				// for Windows
	vshader = "shaders/vshader_win.glsl";
	fshader = "shaders/fshader_win.glsl";
#endif
	// 对敌人的部位进行设置
	for(int i = 0; i < enemyNum; i++) {
		TriMesh* Mesh = new TriMesh();
		// 设定好初始位置 初始化所有层级建模的部位 初始化所有部位的位置
		// 设定所在的位置
		Mesh->setTranslation(position);
		Mesh->setRotation(glm::vec3(0.0, 0.0, 0.0));
		// 设置物体的初始大小
		Mesh->setScale(glm::vec3(0.3, 0.3, 0.3));
		Mesh->setNormalize(true);
		Mesh->setAmbient(glm::vec4(0.2, 0.2, 0.2, 1.0));  // 环境光
		Mesh->setDiffuse(glm::vec4(0.7, 0.7, 0.7, 1.0));  // 漫反射
		Mesh->setSpecular(glm::vec4(0.2, 0.2, 0.2, 1.0)); // 镜面反射
		Mesh->setShininess(1.0);						  //高光系数
		Enemy.push_back(Mesh);
	}
	Enemy[enemy.Body]->readObj("./assets/enemyBody.obj");
	Enemy[enemy.leftArm]->readObj("./assets/enemyArm.obj");
	Enemy[enemy.rightArm]->readObj("./assets/enemyArm.obj");
	Enemy[enemy.leftLeg]->readObj("./assets/enemyleg.obj");
	Enemy[enemy.rightLeg]->readObj("./assets/enemyleg.obj");

	// 对相应部位进行绑定
	painter->bindObjectAndData(Enemy[enemy.Body], EnemyBodyObject, "./assets/enemyBody.png", vshader, fshader);
	painter->bindObjectAndData(Enemy[enemy.leftArm], EnemyleftArmObject, "./assets/enemyArm.png", vshader, fshader);
	painter->bindObjectAndData(Enemy[enemy.rightArm], EnemyrightArmObject, "./assets/enemyArm.png", vshader, fshader);
	painter->bindObjectAndData(Enemy[enemy.leftLeg], EnemyleftLegObject, "./assets/enemyleg.png", vshader, fshader);
	painter->bindObjectAndData(Enemy[enemy.rightLeg], EnemyrightLegObject, "./assets/enemyleg.png", vshader, fshader);
}



void init() {
	// 初始化场景各个物体
	setObj();
	// 初始化场景的机器人
	setRobot();

	// 初始化场景的对应的随机敌人
	setEnemy(Enemy, glm::vec3(-7.0, 0.005, -3.0));
	setEnemy(Enemy2, glm::vec3(3.0, 0.005, 3.0));
	setEnemy(Enemy3, glm::vec3(5.0, 0.005, 1.0));
	setEnemy(Enemy4, glm::vec3(12.0, 0.005, -5.0));
	setEnemy(Enemy5, glm::vec3(-7.0, 0.005, 10.0));
	glClearColor(0.3, 0.3, 0.3, 1.0);
}

// 绘制控制机器人的层级建模代码
void drawRobot() {
	// 先计算躯干的模式变换矩阵
	glm::mat4 modelMatrix = Man[robot.Body]->getModelMatrix();

	// 构造栈
	MatrixStack  mStack;


	// =====================躯干========================
	// 躯干（这里我们希望机器人的躯干只绕Y轴旋转，所以只计算了RotateY） 
	// 首先计算本身的变换矩阵
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
	// 整个身体沿着摄像机的方向进行转动
	// y轴
	modelMatrix = glm::rotate(modelMatrix, glm::radians(camera->yaw), glm::vec3(0.0, -1.0, 0.0));
	painter->drawMesh(Man[robot.Body], BodyObject, light, camera, body(modelMatrix));

	// 将躯干的变换矩阵压入
	mStack.push(modelMatrix);

	// ======================头部========================
	// 头部 (机器人的头部只能沿着Y轴旋转) 所以只计算了RotateY 往上移动BodyHeiGht个单位
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.5 * robot.BODY_HEIGHT, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.Head]), glm::vec3(0.0, 1.0, 0.0));
	// 绘制头部
	painter->drawMesh(Man[robot.Head], HeadObject, light, camera, head(modelMatrix));
	// 恢复躯干变化矩阵
	modelMatrix = mStack.pop();

	// ======================左大臂=========================
	// 保存躯干变化矩阵 
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.05, 0.55, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftArm]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.LeftArm]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.LeftArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左大臂
	painter->drawMesh(Man[robot.LeftArm], LeftUpperArmObject, light, camera, Left_Big_Arm(modelMatrix));

	// ======================左小臂==========================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.3, 0.0, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftSmallArm]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.LeftSmallArm]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.LeftSmallArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左小臂
	painter->drawMesh(Man[robot.LeftSmallArm], LeftLowerArmObject, light, camera, Left_Small_Arm(modelMatrix));


	// =======================剑==============================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.3, 0.0, 0.0));
	painter->drawMesh(Man[robot.sword], swordObject, light, camera, sword(modelMatrix));
	// 恢复躯干变换矩阵
	modelMatrix = mStack.pop();

	// =======================右大臂==========================
	// 保存躯干变换矩阵
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.05, 0.57, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightArm]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.RightArm]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.RightArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制右大臂
	painter->drawMesh(Man[robot.RightArm], RightUpperArmObject, light, camera, Right_Big_Arm(modelMatrix));

	// =======================右边小臂==========================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.3, 0.0, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightSmallArm]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.RightSmallArm]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.RightSmallArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制右小臂
	painter->drawMesh(Man[robot.RightSmallArm], RightLowerArmObject, light, camera, Right_Small_Arm(modelMatrix));
	modelMatrix = mStack.pop();

	// ========================左腿===========================
	// 保存躯干的变换矩阵
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.1 * robot.BODY_WIDTH, 0.40 * robot.BODY_HEIGHT, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftLeg]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.LeftLeg]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.LeftLeg]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左腿
	painter->drawMesh(Man[robot.LeftLeg], LeftUpperLegObject, light, camera, Left_Big_Leg(modelMatrix));

	// ========================左小腿==========================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.86 * robot.UPPER_LEG_HEIGHT, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.LeftSmallLeg]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.LeftSmallLeg]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.LeftSmallLeg]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左小腿
	painter->drawMesh(Man[robot.LeftSmallLeg], LeftLowerLegObject, light, camera, Left_Small_Leg(modelMatrix));
	// 恢复躯干变换矩阵
	modelMatrix = mStack.pop();

	// =========================右腿============================
	// 保存躯干变换矩阵
	mStack.push(modelMatrix);   // 保存躯干变换矩阵
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.1 * robot.BODY_WIDTH, 0.40 * robot.BODY_HEIGHT, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightLeg]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.RightLeg]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.RightLeg]), glm::vec3(0.0, 0.0, 1.0));
	painter->drawMesh(Man[robot.RightLeg], RightUpperLegObject, light, camera, Right_Big_Leg(modelMatrix));

	// =========================右小腿===========================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.86 * robot.UPPER_LEG_HEIGHT, 0.0));
	// 绕y轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.theta[robot.RightSmallLeg]), glm::vec3(0.0, 1.0, 0.0));
	// 绕x轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaX[robot.RightSmallLeg]), glm::vec3(1.0, 0.0, 0.0));
	// 绕z轴旋转
	modelMatrix = glm::rotate(modelMatrix, glm::radians(robot.thetaZ[robot.RightSmallLeg]), glm::vec3(0.0, 0.0, 1.0));
	painter->drawMesh(Man[robot.RightSmallLeg], RightLowerLegObject, light, camera, Right_Small_Leg(modelMatrix));

	// 最后恢复躯干变换矩阵
	modelMatrix = mStack.pop();

}

// 绘制移动恐龙敌人的层级建模代码
void drawEnemy(vector<TriMesh*> Enemy) {
	// 先计算躯干的模式变换矩阵
	glm::mat4 modelMatrix = Enemy[enemy.Body]->getModelMatrix();

	// 构造栈
	MatrixStack  mStack;

	// =====================躯干========================
	// 躯干（这里我们希望机器人的躯干只绕Y轴旋转，所以只计算了RotateY） 
	// 首先计算本身的变换矩阵
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
	painter->drawMesh(Enemy[enemy.Body], EnemyBodyObject, light, camera, bodyEnemy(modelMatrix));

	// 将躯干的变换矩阵压入
	mStack.push(modelMatrix);

	// ======================左臂=========================
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.2, 0.5, 0.1));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.theta[enemy.leftArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左大臂
	painter->drawMesh(Enemy[enemy.leftArm], EnemyleftArmObject, light, camera, LeftArmEnemy(modelMatrix));
	// 恢复模式矩阵
	modelMatrix = mStack.pop();

	// ======================右臂=========================
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.2, 0.5, -0.1));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.theta[enemy.rightArm]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制右臂
	painter->drawMesh(Enemy[enemy.rightArm], EnemyrightArmObject, light, camera, RightArmEnemy(modelMatrix));
	// 恢复模式矩阵
	modelMatrix = mStack.pop();

	// ======================左腿=========================
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.2, -0.1));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.theta[enemy.leftLeg]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左腿
	painter->drawMesh(Enemy[enemy.leftLeg], EnemyleftLegObject, light, camera, LeftLegEnemy(modelMatrix));
	// 恢复模式矩阵
	modelMatrix = mStack.pop();

	// ======================右腿=========================
	mStack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.2, 0.1));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(enemy.theta[enemy.rightLeg]), glm::vec3(0.0, 0.0, 1.0));
	// 绘制左腿
	painter->drawMesh(Enemy[enemy.rightLeg], EnemyrightLegObject, light, camera, RightLegEnemy(modelMatrix));
	// 恢复模式矩阵
	modelMatrix = mStack.pop();
}

// 敌人随机移动函数
void enemyMove(int i, vector<TriMesh*> Enemy) {
	// 用i的取模结果来确定敌人的移动方向
	if(i <= 2500) {
		Enemy[enemy.Body]->setTranslation(Enemy[enemy.Body]->getTranslation() + glm::vec3(0.001, 0.0, 0.0));
		if(i == 2500) {
			// 对恐龙进行转向
			Enemy[enemy.Body]->setRotation(glm::vec3(0.0, -90.0, 0.0));
		}
	} else if(i > 2500 && i <= 5000) {
		Enemy[enemy.Body]->setTranslation(Enemy[enemy.Body]->getTranslation() + glm::vec3(0.0, 0.0, 0.001));
		if(i == 5000) {
			Enemy[enemy.Body]->setRotation(glm::vec3(0.0, -180.0, 0.0));
		}
	} else if(i > 5000 && i <= 7500) {
		Enemy[enemy.Body]->setTranslation(Enemy[enemy.Body]->getTranslation() + glm::vec3(-0.001, 0.0, 0.0));
		if(i == 7500) {
			Enemy[enemy.Body]->setRotation(glm::vec3(0.0, -270.0, 0.0));
		}
	} else if(i > 7500 && i <= 10000) {
		Enemy[enemy.Body]->setTranslation(Enemy[enemy.Body]->getTranslation() + glm::vec3(0.0, 0.0, -0.001));
		// 因为到不达10000 所以在9999进行一个方位的更新
		if(i == 9999) {
			Enemy[enemy.Body]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		}
	}
}

// 第二个敌人随机移动函数
void enemyMove2(int i) {
	// 用i的取模结果来确定敌人的移动方向
	if(i <= 2500) {
		Enemy2[enemy.Body]->setTranslation(Enemy2[enemy.Body]->getTranslation() + glm::vec3(-0.0010, 0.0, 0.0));
		if(i == 2500) {
			// 对恐龙进行转向
			Enemy2[enemy.Body]->setRotation(glm::vec3(0.0, 90.0, 0.0));
		}
	} else if(i > 2500 && i <= 5000) {
		Enemy2[enemy.Body]->setTranslation(Enemy2[enemy.Body]->getTranslation() + glm::vec3(0.0, 0.0, -0.0010));
		if(i == 5000) {
			Enemy2[enemy.Body]->setRotation(glm::vec3(0.0, 0.0, 0.0));
		}
	} else if(i > 5000 && i <= 7500) {
		Enemy2[enemy.Body]->setTranslation(Enemy2[enemy.Body]->getTranslation() + glm::vec3(0.0010, 0.0, 0.0));
		if(i == 7500) {
			Enemy2[enemy.Body]->setRotation(glm::vec3(0.0, 270.0, 0.0));
		}
	} else if(i > 7500 && i <= 10000) {
		Enemy2[enemy.Body]->setTranslation(Enemy2[enemy.Body]->getTranslation() + glm::vec3(0.0, 0.0, 0.0010));
		// 因为到不达10000 所以在9999进行一个方位的更新
		if(i == 9999) {
			Enemy2[enemy.Body]->setRotation(glm::vec3(0.0, 180.0, 0.0));
		}
	}
}



// 消灭敌人
void enemyDistroy() {
	// 计算二者位置的距离 当小于一个阈值的时候进行消灭
	if(distance(Man[robot.Body]->getTranslation(), Enemy[enemy.Body]->getTranslation()) < 0.1) {
		if(condition == 1) {
			Enemy[enemy.Body]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy[enemy.leftArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy[enemy.rightArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy[enemy.leftLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy[enemy.rightLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
		} else {
			cout << endl << "You are not warrior, you can not kill the Enemy" << endl;
		}
	}
	if(distance(Man[robot.Body]->getTranslation(), Enemy2[enemy.Body]->getTranslation()) < 0.1) {
		if(condition == 1) {
			// 消灭第二个敌人
			Enemy2[enemy.Body]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy2[enemy.leftArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy2[enemy.rightArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy2[enemy.leftLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy2[enemy.rightLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
		} else {
			cout << endl << "You are not warrior, you can not kill the Enemy" << endl;
		}
	}
	if(distance(Man[robot.Body]->getTranslation(), Enemy3[enemy.Body]->getTranslation()) < 0.1) {
		if(condition == 1) {
			// 消灭第三个敌人
			Enemy3[enemy.Body]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy3[enemy.leftArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy3[enemy.rightArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy3[enemy.leftLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy3[enemy.rightLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
		} else {
			cout << endl << "You are not warrior, you can not kill the Enemy" << endl;
		}
	}
	if(distance(Man[robot.Body]->getTranslation(), Enemy4[enemy.Body]->getTranslation()) < 0.1) {
		if(condition == 1) {
			// 消灭第四个敌人
			Enemy4[enemy.Body]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy4[enemy.leftArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy4[enemy.rightArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy4[enemy.leftLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy4[enemy.rightLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
		} else {
			cout << endl << "You are not warrior, you can not kill the Enemy" << endl;
		}
	}
	if(distance(Man[robot.Body]->getTranslation(), Enemy5[enemy.Body]->getTranslation()) < 0.1) {
		if(condition == 1) {
			// 消灭第四个敌人
			Enemy5[enemy.Body]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy5[enemy.leftArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy5[enemy.rightArm]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy5[enemy.leftLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
			Enemy5[enemy.rightLeg]->setScale(glm::vec3(0.0, 0.0, 0.0));
		} else {
			cout << endl << "You are not warrior, you can not kill the Enemy" << endl;
		}
	}
}

void MoveEnemy() {
	// 对敌人进行一个随机的移动
	++state3;
	if(state3 >= 10000) {
		state3 = 0;
	}
	enemyMove(state3, Enemy);
	enemyMove2(state3);
	enemyMove(state3, Enemy3);
	enemyMove(state3, Enemy4);
	enemyMove(state3, Enemy5);
}

void saveOldMan() {
	// 判断是否和老人碰撞
	if(distance(Man[robot.Body]->getTranslation(), oldMan->getTranslation()) < 0.15) {
		if(condition == 1) {
			cout << endl;
			cout << "You are not princess, you can not save old man" << endl;
		} else if(Enemy[enemy.Body]->getScale() != glm::vec3(0.0, 0.0, 0.0) || Enemy2[enemy.Body]->getScale() != glm::vec3(0.0, 0.0, 0.0) || Enemy3[enemy.Body]->getScale() != glm::vec3(0.0, 0.0, 0.0) || Enemy4[enemy.Body]->getScale() != glm::vec3(0.0, 0.0, 0.0) || Enemy5[enemy.Body]->getScale() != glm::vec3(0.0, 0.0, 0.0)) {
			cout << endl;
			cout << "You have to kill all enemy" << endl;
		} else {
			cout << endl << "Congratulations!You save old Man and win the game!" << endl;
		}
	}
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 绘制除层级机器人外的场景
	painter->drawMeshes(light, camera);

	// 绘制对应的机器人
	drawRobot();

	// 对敌人进行绘制
	drawEnemy(Enemy);
	drawEnemy(Enemy2);
	drawEnemy(Enemy3);
	drawEnemy(Enemy4);
	drawEnemy(Enemy5);

	// 对敌人进行移动
	MoveEnemy();

	// 消灭敌人的函数
	enemyDistroy();

	// 拯救老人函数
	saveOldMan();
}




void printHelp() {
	std::cout << "================================================" << std::endl;
	std::cout << "Control your character to annihilate all the enemy and save the Old Man" << std::endl;
	std::cout << "================================================" << std::endl << std::endl;

	std::cout << "Keyboard Usage and game rules are in instructions" << std::endl;
	cout << endl;
	cout << "You must be in warrior form to kill the enemy" << endl;
	cout << endl;
	cout << "You must be a princess to rescue the old man, otherwise you will scare the old man" << endl;
	cout << endl;
}



// 键盘响应函数 
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	std::string vshader, fshader;
	// 读取着色器并使用
#ifdef __APPLE__	// for MacOS
	vshader = "shaders/vshader_mac.glsl";
	fshader = "shaders/fshader_mac.glsl";
#else				// for Windows
	vshader = "shaders/vshader_win.glsl";
	fshader = "shaders/fshader_win.glsl";
#endif
	float tmp;
	glm::vec4 ambient;
	switch(key) {
	case GLFW_KEY_ESCAPE: exit(EXIT_SUCCESS); break;
	case GLFW_KEY_H: printHelp(); break;
	case GLFW_KEY_W: {
		if(action == GLFW_REPEAT && mode == 0x0000) {
			// 这里描绘移动小人的函数 根据摄像机的角度进行移动
			// 对摄像机的位置进行移动
			Man[robot.Body]->setTranslation(glm::vec3(Man[robot.Body]->getTranslation().x - 0.03 * sin(glm::radians(camera->yaw)), Man[robot.Body]->getTranslation().y, Man[robot.Body]->getTranslation().z + 0.03 * cos(glm::radians(camera->yaw))));
			// 移动摄像头
			camera->eyex -= 0.03 * sin(glm::radians(camera->yaw));
			camera->eyez += 0.03 * cos(glm::radians(camera->yaw));
			// 增加关键帧 改变层级机器人的形状
			// 防止state溢出
			if(++state > 10000) {
				state = 0;
			}
			RobotArmAnim(state);
		} else if(action == GLFW_REPEAT && mode == GLFW_MOD_SHIFT) {
			// 移动相机的位置
			// 根据方向的位置更新相机的位置
			camera->eyex += 0.03f * camera->cameraDirection.x;
			camera->eyey += 0.03f * camera->cameraDirection.y;
			camera->eyez += 0.03f * camera->cameraDirection.z;
		}
		break;
	}
	case GLFW_KEY_S: {
		if(action == GLFW_REPEAT && mode == GLFW_MOD_SHIFT) {
			// 移动相机的位置往后退
			// 移动相机的位置
			// 根据方向的位置更新相机的位置
			camera->eyex -= 0.03f * camera->cameraDirection.x;
			camera->eyey -= 0.03f * camera->cameraDirection.y;
			camera->eyez -= 0.03f * camera->cameraDirection.z;
		}
		break;
	}
	case GLFW_KEY_1: selectedMesh = robot.Body; break;
	case GLFW_KEY_2: selectedMesh = robot.Head; break;
	case GLFW_KEY_3: selectedMesh = robot.RightArm; break;
	case GLFW_KEY_4: selectedMesh = robot.RightSmallArm; break;
	case GLFW_KEY_5: selectedMesh = robot.LeftArm; break;
	case GLFW_KEY_6: selectedMesh = robot.LeftSmallArm; break;
	case GLFW_KEY_7: selectedMesh = robot.RightLeg; break;
	case GLFW_KEY_8: selectedMesh = robot.RightSmallLeg; break;
	case GLFW_KEY_9: selectedMesh = robot.LeftLeg; break;
	case GLFW_KEY_0: selectedMesh = robot.LeftSmallLeg; break;
		// 对角度进行更改以及旋转
	case GLFW_KEY_Q:
		robot.theta[selectedMesh] += 5.0;
		if(robot.theta[selectedMesh] > 360.0)
			robot.theta[selectedMesh] -= 360.0;
		break;
	case GLFW_KEY_E:
		robot.theta[selectedMesh] -= 5.0;
		if(robot.theta[selectedMesh] < 0.0)
			robot.theta[selectedMesh] += 360.0;
		break;
	case GLFW_KEY_Z:
		if(selectedMesh != robot.Body && selectedMesh != robot.Head) {
			// 排除两个部位的x轴旋转
			robot.thetaX[selectedMesh] += 5.0;
			if(robot.thetaX[selectedMesh] > 360.0) {
				robot.thetaX[selectedMesh] -= 360.0;
			}
		}
		break;
	case GLFW_KEY_C:
		if(selectedMesh != robot.Body && selectedMesh != robot.Head) {
			robot.thetaX[selectedMesh] -= 5.0;
			if(robot.thetaX[selectedMesh] < 0.0) {
				robot.thetaX[selectedMesh] += 360.0;
			}
		}
		break;
	case GLFW_KEY_J:
		if(selectedMesh != robot.Body && selectedMesh != robot.Head) {
			robot.thetaZ[selectedMesh] += 5.0;
			if(robot.thetaZ[selectedMesh] > 360.0) {
				robot.thetaZ[selectedMesh] -= 360.0;
			}
		}
		break;
	case GLFW_KEY_L:
		if(selectedMesh != robot.Body && selectedMesh != robot.Head) {
			robot.thetaZ[selectedMesh] -= 5.0;
			if(robot.thetaZ[selectedMesh] < 0.0) {
				robot.thetaZ[selectedMesh] += 360.0;
			}
		}
		break;
	case GLFW_KEY_P:
		// 更换模型的头与身体
		// 换身体
		Man[robot.Body]->readObj("./assets/body2.obj");
		painter->bindObjectAndData(Man[robot.Body], BodyObject, "./assets/body2.png", vshader, fshader);
		robot.BODY_HEIGHT = 1.0;
		robot.BODY_WIDTH = 0.6;
		// 换头
		Man[robot.Head]->readObj("./assets/head2.obj");
		painter->bindObjectAndData(Man[robot.Head], HeadObject, "./assets/head2.png", vshader, fshader);
		// 改变头的大小尺寸
		robot.HEAD_HEIGHT = 1.3;
		robot.HEAD_WIDTH = 1.3;

		// 绘制剑
		Man[robot.sword]->readObj("./assets/sword.obj");
		painter->bindObjectAndData(Man[robot.sword], swordObject, "./assets/sword.png", vshader, fshader);
		robot.SWORD_HEIGHT = 1.0;
		robot.SWORD_WIDTH = 1.0;
		condition = 1;
		break;
	case GLFW_KEY_O:
		// 换身体
		Man[robot.Body]->readObj("./assets/body.obj");
		painter->bindObjectAndData(Man[robot.Body], BodyObject, "./assets/body.png", vshader, fshader);
		robot.BODY_HEIGHT = 1.0;
		robot.BODY_WIDTH = 1.0;
		// 换头
		Man[robot.Head]->readObj("./assets/head.obj");
		painter->bindObjectAndData(Man[robot.Head], HeadObject, "./assets/body.png", vshader, fshader);
		robot.HEAD_HEIGHT = 1.0;
		robot.HEAD_WIDTH = 1.0;
		robot.SWORD_HEIGHT = 0.0;
		robot.SWORD_WIDTH = 0.0;
		condition = 0;
		break;
	case GLFW_KEY_B:
		++state4;
		if(state4 > 10000) {
			state4 = 0;
		}
		EnemyAnim(state4);
		break;
	default:
		camera->keyboard(key, action, mode);
		break;
	}
}


// 鼠标位置回调函数 当鼠标移动的时候就会调用这个函数并且返回鼠标的位置 用来计算欧拉角
void cursor_position_callback(GLFWwindow* window, double x, double y) {
	camera->mouse(x, y);
	// 鼠标回归屏幕中心
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	// 设置鼠标不可见 输入模式是隐藏鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	return;
}

// 鼠标点击函数 当鼠标点击的回调函数
void mouse_button(GLFWwindow* window, int button, int action, int mode) {
	switch(button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		if(action == GLFW_PRESS) {
			// 鼠标点击左键 执行挥剑的动作
			// 维护一个帧
			++state2;
			if(state2 >= 10000) {
				state2 = 0;
			}
			swordMove(state2);
		}
	default:
		break;
	}
}

// 重新设置窗口
void reshape(GLsizei w, GLsizei h) {
	glViewport(0, 0, w, h);
}

void cleanData() {
	// 释放内存

	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();

	delete painter;
	painter = NULL;

	for(int i = 0; i < meshList.size(); i++) {
		delete meshList[i];
	}
	meshList.clear();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性 设置窗口的大小和名字等
	window = glfwCreateWindow(900, 900, "2019122180-cjl-final", NULL, NULL);
	if(window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 调用任何OpenGL的函数之前初始化GLAD
	// ---------------------------------------
	if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	while(!glfwWindowShouldClose(window)) {
		display();
		//reshape();
		// 交换颜色缓冲 以及 检查有没有触发什么事件（比如键盘输入、鼠标移动等）
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();


	return 0;
}
// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理。
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
