#version 330 core

// 给光源数据一个结构体
struct Light{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	vec3 position;

    // 光源衰减系数的三个参数
    float constant; // 常数项
    float linear;	// 一次项
    float quadratic;// 二次项

};

// 给物体材质数据一个结构体
struct Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float shininess;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;


// 相机坐标
uniform vec3 eye_position;
// 光源
uniform Light light;
// 物体材质
uniform Material material;

uniform int isShadow;
// 纹理数据
uniform sampler2D texture;

// 传递给片元着色器
out vec4 fColor;

void main()
{
	if (isShadow == 1) {
		fColor = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		// 计算四个归一化的向量
		// @TODO: Task2 计算四个归一化的向量 N,V,L,R(或半角向量H)
   	 	vec3 N= normalize(normal);
    	vec3 V= normalize(eye_position - position);
    	vec3 L= normalize(light.position - position);
    	vec3 R= normalize(reflect(-L, N));

		// 计算环境光分量I_a 
		vec4 I_a = light.ambient * material.ambient;

		// @TODO: Task2 计算漫反射系数alpha和漫反射分量I_d
		float diffuse_dot = 0.0;
		diffuse_dot = max(dot(L, N), 0);
		vec4 I_d = diffuse_dot *  light.diffuse * material.diffuse;

		// @TODO: Task2 计算高光系数beta和镜面反射分量I_s
		float specular_dot_pow = 0.0;
		specular_dot_pow = pow(max(dot(R, V),0), material.shininess);
		vec4 I_s = specular_dot_pow * light.specular * material.specular;

		// 注意如果光源在背面则去除高光
		// if( dot(L, N) < 0.0 ) {
		//     I_s = vec4(0.0, 0.0, 0.0, 1.0);
		// } 

		// 合并三个分量的颜色，修正透明度 
		fColor = texture2D( texture, texCoord );
		// 叠加phong模型光照颜色
		// 修正得到最后的颜色
		// 其中id和is加上衰减系数
		float d = distance(light.position,position);
		// 使用衰减系数进行模拟
 		fColor += I_a + 1/(light.constant+light.linear*d+light.quadratic*d*d)*(I_d+I_s);
	}
}
