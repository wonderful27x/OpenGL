#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int playMove;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D textureY;
uniform sampler2D textureU;
uniform sampler2D textureV;

uniform int movie;
uniform float scale;

void main()
{
	//// linearly interpolate between both textures (80% container, 20% awesomeface)
	//if(playMove == 1) {
	//	FragColor = texture(texture1, TexCoord);
	//}else {
	//	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	//}

	if(movie == 1) {
		//跳过字节对齐的无效数据
		//if(TexCoord.x > scale) return;
		vec3 yuv;
		vec2 coordinate = vec2(TexCoord.x, 1.0 - TexCoord.y);
		yuv.x = texture(textureY, coordinate).r;
		yuv.y = texture(textureU, coordinate).r-0.5;
		yuv.z = texture(textureV, coordinate).r-0.5;
		highp vec3 rgb = mat3(
			1.0,	1.0,	1.0,
			0.0,	-0.344,	1.770,
			1.403,	-0.714,	0.0
		) * yuv;
		FragColor = mix(texture(texture1, TexCoord), vec4(rgb, 1.0), 0.65);
	}else {
		FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	}
}
