#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 Normal;
out vec3 FragPos;

void main(){
       gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos,1.0);
	//将法向量和片段坐标传到片段着色器，
	//所有光照计算都基于世界坐标，所以需要将片段坐标转成世界坐标
	//法向量同理也需要转换
	Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
	FragPos = vec3(modelMatrix * vec4(aPos,1.0f));
}
        

