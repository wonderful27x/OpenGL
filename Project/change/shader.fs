#version 330 core
out vec4 FragColor;
in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D sampler0;
uniform sampler2D sampler1;
void main(){
        FragColor = mix(texture(sampler0,texCoord),texture(sampler1,texCoord),0.2);
	//FragColor = texture(sampler0,texCoord);
	//FragColor = texture(sampler1,texCoord);
}


