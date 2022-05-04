#version 330 core

precision mediump float;
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

//公式
//R = Y + 1.4075 * (V-128);  
//G = Y - 0.3455 * (U-128) - 0.7169*(V-128);  
//B = Y + 1.779 * (U-128); 
//注意uv分量需要减去128，即0.5*255,所以下面纹理采样时减去0.5就是这个原因，
//因为纹理采样范围是0-1.0, 即0.5*1.0 = 0.5

void main()
{
	vec3 yuv;
	yuv.x = texture(texture1, TexCoord).r;
	yuv.y = texture(texture2, TexCoord).g-0.5;
	yuv.z = texture(texture2, TexCoord).r-0.5;
	highp vec3 rgb = mat3(
		1.0,	1.0,	1.0,
		0.0,	-0.344,	1.770,
		1.403,	-0.714,	0.0
	) * yuv;
	FragColor = vec4(rgb, 1.0);

	//vec3 yuv;
	//yuv.x = texture(texture1, TexCoord).r;
	//yuv.y = texture(texture2, TexCoord).a-0.5;
	//yuv.z = texture(texture2, TexCoord).r-0.5;
	//float r = yuv.x + 1.4075 * (yuv.z-128);
	//float g = yuv.x - 0.3445 * (yuv.y-128) - 0.7169 * (yuv.z-128);
	//float b = yuv.x + 1.779 * (yuv.y-128);
	//FragColor = vec4(r, g, b, 1.0);	
	
	//FragColor = texture(texture1, TexCoord);
	//FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}
