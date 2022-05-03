#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void processInput(GLFWwindow* window);
void scrollCallback(GLFWwindow* window,double xoffset,double yoffset);
void mouseCallback(GLFWwindow* window,double xpos,double ypos);
unsigned int loadTexture(char const* path);

//view size
const unsigned int SRC_WIDTH = 800;
const unsigned int SRC_HEIGHT = 600;

//timing
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

//rotate control
glm::vec3 viewPosition(0.0f,0.0f,3.0f);
Camera camera(viewPosition);
bool firstMouse = true;
float lastX = SRC_WIDTH/2.0;
float lastY = SRC_HEIGHT/2.0;

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH,SRC_HEIGHT,"LearnOpenGL",NULL,NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window\n" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetCursorPosCallback(window,mouseCallback);
	glfwSetScrollCallback(window,scrollCallback);
	//tell glfw to capture our mouse
	glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
	
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD\n" << std::endl;
		return -1;
	}
	 
	//build and compile shader program
	Shader shader("light_parallel.vs","light_parallel.fs");

	//set up vertex data and configure vertex attributes
	float vertices[] = {
		// positions          // normals           // texture coords
        	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// positions all containers
    	glm::vec3 cubePositions[] = {
    	    glm::vec3( 0.0f,  0.0f,  0.0f),
    	    glm::vec3( 2.0f,  5.0f, -15.0f),
    	    glm::vec3(-1.5f, -2.2f, -2.5f),
    	    glm::vec3(-3.8f, -2.0f, -12.3f),
    	    glm::vec3( 2.4f, -0.4f, -3.5f),
    	    glm::vec3(-1.7f,  3.0f, -7.5f),
    	    glm::vec3( 1.3f, -2.0f, -2.5f),
    	    glm::vec3( 1.5f,  2.0f, -2.5f),
    	    glm::vec3( 1.5f,  0.2f, -1.5f),
    	    glm::vec3(-1.3f,  1.0f, -1.5f)
    	};
	
	unsigned int VBO; 
	//object buffer data
       	unsigned int VAO;
       	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
	
	glBindVertexArray(VAO);
	//顶点坐标
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	//对应的法向量
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	//纹理坐标
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	
	unsigned int diffuseMap = loadTexture("../../images/woodtexture1.png");
	unsigned int specularMap = loadTexture("../../images/steeltexture.png");
	shader.use();
	shader.setInt("material.diffuse",0);
	shader.setInt("material.specular",1);

	
	glEnable(GL_DEPTH_TEST);	
	

	while(!glfwWindowShouldClose(window)){
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrameTime;
		lastFrameTime = currentTime;

		//input
		processInput(window);

		//glClearColor(0.2f,0.3f,0.3f,1.0f);
		glClearColor(0.0f,0.0f,0.0f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewTrans = glm::mat4(1.0f);
		glm::mat4 projectionTrans = glm::mat4(1.0f);
		glm::mat4 modelTrans = glm::mat4(1.0f);

		viewTrans = camera.getViewMatrix();
		projectionTrans = glm::perspective(glm::radians(camera.fov),(float)SRC_WIDTH/(float)SRC_HEIGHT,0.1f,100.0f);

		glm::vec3 lightColor;
		lightColor.x = sin(glfwGetTime() * 2.0f);
		lightColor.y = sin(glfwGetTime() * 0.7f);
		lightColor.z = sin(glfwGetTime() * 1.3f);
	
		//draw light -----------------------------------------------------	
		//这个demo不需要画灯了，模拟太阳平行光

		//draw object -------------------------------------------------
		shader.use();
		//light
		shader.setVec3("light.direction",-0.2f,-1.0f,-0.3f);
		shader.setVec3("light.ambient",0.2f,0.2f,0.2f);
		shader.setVec3("light.diffuse",0.5f,0.5f,0.5f);
		shader.setVec3("light.specular",1.0f,1.0f,1.0f);
		//light
		//glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		//glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
		//shader.setVec3("light.position",lightPosition);
		//shader.setVec3("light.ambient",ambientColor);
		//shader.setVec3("light.diffuse",diffuseColor);
		//shader.setVec3("light.specular",1.0f,1.0f,1.0f);
		//material
		shader.setVec3("material.ambient",1.0f,0.5f,0.31f);
		shader.setVec3("material.diffuse",1.0f,0.5f,0.31f);
		shader.setVec3("material.specular",0.5f,0.5f,0.5f);
		shader.setFloat("material.shininess",32.0f);
		//viewPos and matrix
		shader.setVec3("viewPos",camera.position);
		shader.setMat4("viewMatrix",glm::value_ptr(viewTrans),false);
		shader.setMat4("projectionMatrix",&projectionTrans[0][0],false);
		shader.setMat4("modelMatrix",glm::value_ptr(modelTrans),false);

		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,diffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,specularMap);
		
		glBindVertexArray(VAO);
		for(int i=0; i<10; i++){
			modelTrans = glm::mat4(1.0f);
			modelTrans = glm::translate(modelTrans,cubePositions[i]);
			float angle = 20.0f * i;
			modelTrans = glm::rotate(modelTrans,glm::radians(angle),glm::vec3(1.0f,0.3f,0.5f));
			shader.setMat4("modelMatrix",glm::value_ptr(modelTrans),false);
			glDrawArrays(GL_TRIANGLES,0,36);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window){
	if(glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window,true);

	if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS){
		camera.processKeyboard(FORWARD,deltaTime);
	}
	if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS){
		camera.processKeyboard(BACKWARD,deltaTime);
	}
	if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS){
		camera.processKeyboard(LEFT,deltaTime);
	}
	if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS){
		camera.processKeyboard(RIGHT,deltaTime);
	}
}

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
	glViewport(0,0,width,height);
}

void scrollCallback(GLFWwindow* window,double xoffset,double yoffset){
	camera.processMouseScroll(yoffset);
}


void mouseCallback(GLFWwindow* window,double xpos,double ypos){
	if(firstMouse){
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.processMouseMovement(xoffset,yoffset);
}
	
unsigned int loadTexture(char const* path){
	unsigned int textureId;
	glGenTextures(1,&textureId);
	int width,height,nrComponents;
	unsigned char* data = stbi_load(path,&width,&height,&nrComponents,0);
	if(data){
		GLenum format;
		if(nrComponents == 1){
			format = GL_RED;
		}else if(nrComponents == 3){
			format = GL_RGB;
		}else if(nrComponents == 4){
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D,textureId);
		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 

		stbi_image_free(data);
	}else{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureId;
}


