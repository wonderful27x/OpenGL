#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main(){
	Normal = mat3(transpose(inverse(modelMatrix))) * aNormal;
       Position = vec3(modelMatrix * vec4(aPos,1.0));
       gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos,1.0);
}
        

