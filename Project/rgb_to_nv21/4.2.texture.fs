#version 330 core

precision mediump float;
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

uniform float u_offset;

//转换公式
//Y =  0.299R + 0.587G + 0.114B
//U = -0.1687R - 0.3313G + 0.5B + 128
//V =  0.5R - 0.4187G - 0.0813B + 128
//注意uv分量后面+128, 即0.5*255, 所以后面uv纹理也需要+ 0.5*1.0=0.5
const vec3 COEF_Y = vec3(0.299, 0.587, 0.114);
const vec3 COEF_U = vec3(-0.147, -0.289, 0.436);
const vec3 COEF_V = vec3(0.615, -0.515, -0.100);
const float UV_DIVIDE_LINE = 2.0 / 3.0;

//https://zhuanlan.zhihu.com/p/427975854
//rgb转nv21必须明确的几个点: 
//1. 输入纹理是一张w*h的rgba格式的图片, 其中a分量我们忽略
//2. 输出的也是一张rgba格式的纹理图片，注意我们是在shader中处理，所以必须在纹理上操作，
//而opengl的纹理通常都是rgba格式的，所以输出实际上是将yuv分量按照rgba分量的数据格式来存储，
//简单说就是rgba当中的一个分量对应yuv的一个分量，比如一个rgba像素可以代表4个y分量,或两组uv分量，
//一张w*h的rgba图片转成nv21, y分量数等于w*h，uv分量数等于w*h/4, yuv_w = w, yuv_h = h*3/2
//用rgba来存储，因此输出纹理的宽高为 宽=w/4, 高=h*3/2
//3. glViewport(0,0,width,high)如何影响纹理坐标？ 对于横坐标步长为1/w, 总步数w，纵坐标类似
//4. 现在的问题是(假设图片w=h=512)
//	* 输入是一张512*512的纹理图片
//	* 输出是一张128*768的纹理图片 (w=512/4, h=512*3/2);
//	* 现在glViewport(0,0,128,768), 即按照1/128,1/768的步长如何对输入进行采样最后通过rgb转换公式映射到输出上???
//
//解释: 渲染的宽高和输出是对应的，每次渲染一个像素rgba，即得到yuv的四个分量(比如YYYY),
//	* 对于横坐标: 
//		因此对于输入横向每次应该采集4个rgba才能转换得到四个Y分量。注意现在横坐标的步长是1/128,
//		对应输入等于1/128*512=4,即每个纹理坐标跨越了四个像素，这样我们可以对这个纹理坐标做一定的偏移，
//		一次采集四次，这样就能在1/128的步长下采集到所有的输入像素
//	* 对于纵坐标:
//		纵坐标的步长为1/768,总步数768,而输入纹理只有512,但是请注意一个很重要的问题: 
//		rgb转nv21应该对rgb遍历两次，第一次全部遍历得到所有的y分量，第二次隔行遍历得到所有的uv分量
//		令1/768 * (3/2) 正好得到1/512的步长，即对于每个纹理坐标y我们*3/2变换回1/512的步长,进行第一次遍历
//		这样当h达到2/3时，即768*(2/3) / 768, 按照1/512的步长*3/2正好遍历整个512的高度	
//		当h超过2/3时，我们经过变换对512的高进行第二次隔行遍历得到uv分量
//
//请结合解释和下面的代码实现细细体会




void main()
{
	vec2 offset = vec2(u_offset, 0.0);
	if(TexCoord.y <= UV_DIVIDE_LINE) {
		vec2 texCoord = vec2(TexCoord.x, TexCoord.y * 3.0 / 2.0);	
		vec4 color0 = texture(texture1, texCoord);
		vec4 color1 = texture(texture1, texCoord + offset);
		vec4 color2 = texture(texture1, texCoord + offset * 2.0);
		vec4 color3 = texture(texture1, texCoord + offset * 3.0);

		float y0 = dot(color0.rgb, COEF_Y);
		float y1 = dot(color1.rgb, COEF_Y);
		float y2 = dot(color2.rgb, COEF_Y);
		float y3 = dot(color2.rgb, COEF_Y);
	
		FragColor = vec4(y0, y1, y2, y3);
	} 
	else {
		vec2 texCoord = vec2(TexCoord.x, (TexCoord.y - UV_DIVIDE_LINE) * 3.0);	
		vec4 color0 = texture(texture1, texCoord);
		vec4 color1 = texture(texture1, texCoord + offset);
		vec4 color2 = texture(texture1, texCoord + offset * 2.0);
		vec4 color3 = texture(texture1, texCoord + offset * 3.0);

		float v0 = dot(color0.rgb, COEF_V) + 0.5;
		float u0 = dot(color1.rgb, COEF_U) + 0.5;
		float v1 = dot(color2.rgb, COEF_V) + 0.5;
		float u1 = dot(color2.rgb, COEF_U) + 0.5;
	
		FragColor = vec4(v0, u0, v1, u1);
	}
}
