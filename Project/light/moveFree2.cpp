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
	Shader shader("objectShader2.vs","objectShader2.fs");
	Shader lightShader("lightShader.vs","lightShader.fs");

	//set up vertex data and configure vertex attributes
	float vertices[] = {
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
	
	    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	
	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	
	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	
	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	//对应的法向量
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	
	//light buffer data
	unsigned int lightVAO;
	glGenVertexArrays(1,&lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);

	//glm::vec3 lightPosition(1.2f,1.0f,-1.0f);
	//glm::vec3 lightPosition(0.0f,1.0f,0.0f);
	glm::vec3 lightPosition(1.0f,0.0f,0.0f);
	glm::vec3 objectPosition(0.0f,0.0f,0.0f);
	
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

		//draw object -------------------------------------------------
		glm::mat4 viewTrans = glm::mat4(1.0f);
		glm::mat4 projectionTrans = glm::mat4(1.0f);
		glm::mat4 modelTrans = glm::mat4(1.0f);

		viewTrans = camera.getViewMatrix();
		projectionTrans = glm::perspective(glm::radians(camera.fov),(float)SRC_WIDTH/(float)SRC_HEIGHT,0.1f,100.0f);
		modelTrans = glm::translate(modelTrans,objectPosition);

		shader.use();
		shader.setVec3("objectColor",1.0f,0.5f,0.31f);
		shader.setVec3("lightColor",1.0f,1.0f,1.0f);
		shader.setVec3("lightPos",lightPosition);
		shader.setVec3("viewPos",camera.position);
		shader.setMat4("viewMatrix",glm::value_ptr(viewTrans),false);
		shader.setMat4("projectionMatrix",&projectionTrans[0][0],false);
		shader.setMat4("modelMatrix",glm::value_ptr(modelTrans),false);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES,0,36);

		//draw light -----------------------------------------------------	
		modelTrans = glm::mat4(1.0f);
		modelTrans = glm::translate(modelTrans,lightPosition);
		modelTrans = glm::scale(modelTrans,glm::vec3(0.2f));

		lightShader.use();
		lightShader.setMat4("viewMatrix",glm::value_ptr(viewTrans),false);
		lightShader.setMat4("projectionMatrix",&projectionTrans[0][0],false);
		lightShader.setMat4("modelMatrix",glm::value_ptr(modelTrans),false);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES,0,36); 

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteVertexArrays(1,&VAO);
	glDeleteVertexArrays(1,&lightVAO);
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
	



