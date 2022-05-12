#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
flat out int playMove;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//试图判断当前绘制的是哪个面，无法判断
	//if(
	//	//(aPos.x > -0.5 && aPos.x < 0.5)	&&
	//	//(aPos.y > -0.5 && aPos.y < 0.5)	&&
	//	(aPos.z == 0.5)
	//) {
	//	playMove = 1;
	//}else {
	//	playMove = 0;
	//}

	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
