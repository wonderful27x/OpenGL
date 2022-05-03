#version 330 core
out vec4 FragColor;
in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D sampler0;
void main(){
       FragColor = texture(sampler0,texCoord)*vec4(vertexColor,1.0);
}


