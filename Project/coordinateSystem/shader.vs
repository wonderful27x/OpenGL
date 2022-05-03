#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertexColor;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){
       //gl_Position = vec4(aPos,1.0);
       gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos,1.0);
       texCoord = aTexCoord;
}
        

