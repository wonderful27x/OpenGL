#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int playMove;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform int movie;

void main()
{
	//// linearly interpolate between both textures (80% container, 20% awesomeface)
	//if(playMove == 1) {
	//	FragColor = texture(texture1, TexCoord);
	//}else {
	//	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	//}

	if(movie == 1) {
		FragColor = texture(texture2, TexCoord);
	}else {
		FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	}
}
