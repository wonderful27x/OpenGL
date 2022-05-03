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
const unsigned int SRC_WIDTH = 1920;
const unsigned int SRC_HEIGHT = 1080;

//timing
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

//rotate control
glm::vec3 viewPosition(0.0f,0.0f,2.0f);
Camera camera(viewPosition);
bool firstMouse = true;
float lastX = SRC_WIDTH/2.0;
float lastY = SRC_HEIGHT/2.0;

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SRC_WIDTH,SRC_HEIGHT,"LearnOpenGL-我老婆云韵",NULL,NULL);
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
	Shader shader("light_yy.vs","light_yy.fs");
	Shader lightShader("light.vs","light.fs");

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
	// positions of the point lights
    	glm::vec3 dotLightPositions[] = {
    	    glm::vec3( 0.7f,  0.2f,  2.0f),
    	    glm::vec3( 2.3f, -3.3f, -4.0f),
    	    glm::vec3(-4.0f,  2.0f, -12.0f),
    	    glm::vec3( 0.0f,  0.0f, -3.0f)
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
	
	//light buffer data
	unsigned int lightVAO;
	glGenVertexArrays(1,&lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);

	
	unsigned int diffuseMap = loadTexture("../../images/woodtexture1.png");
	unsigned int specularMap = loadTexture("../../images/steeltexture.png");
	unsigned int yyMap = loadTexture("../../images/yunyun2.jpeg");
	shader.use();
	shader.setInt("material.diffuse",0);
	shader.setInt("material.specular",1);
	shader.setInt("spotLight.yy",2);

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
		lightShader.use();
		lightShader.setMat4("viewMatrix",glm::value_ptr(viewTrans),false);
		lightShader.setMat4("projectionMatrix",&projectionTrans[0][0],false);
		//lightShader.setVec3("lightColor",lightColor);
		lightShader.setVec3("lightColor",glm::vec3(1.0f,1.0f,1.0f));
		glBindVertexArray(lightVAO);
		for(int i=0; i<4; i++){
			modelTrans = glm::mat4(1.0f);
		        modelTrans = glm::translate(modelTrans,dotLightPositions[i]);
	       	        modelTrans = glm::scale(modelTrans,glm::vec3(0.2f));
			lightShader.setMat4("modelMatrix",glm::value_ptr(modelTrans),false);
			glDrawArrays(GL_TRIANGLES,0,36); 
		} 

		//draw object -------------------------------------------------
		shader.use();
		//directional light
		shader.setVec3("dirLight.direction",-0.2f,-1.0f,-0.3f);
		shader.setVec3("dirlight.ambient",0.05f,0.05f,0.05f);
		shader.setVec3("dirLight.diffuse",0.4f,0.4f,0.4f);
		shader.setVec3("dirLight.specular",0.5f,0.5f,0.5f);
		//dot light 1
		shader.setVec3( "dotLights[0].position",dotLightPositions[0]);
		shader.setVec3( "dotLights[0].ambient",0.05f,0.05f,0.05f);
		shader.setVec3( "dotLights[0].diffuse",0.8f,0.8f,0.8f);
		shader.setVec3( "dotLights[0].specular",1.0f,1.0f,1.0f);
		shader.setFloat("dotLights[0].constant",1.0f);
		shader.setFloat("dotLights[0].linear",0.09f);
		shader.setFloat("dotLights[0].quadratic",0.032f);
		//dot light 2
		shader.setVec3( "dotLights[1].position",dotLightPositions[1]);
		shader.setVec3( "dotLights[1].ambient",0.05f,0.05f,0.05f);
		shader.setVec3( "dotLights[1].diffuse",0.8f,0.8f,0.8f);
		shader.setVec3( "dotLights[1].specular",1.0f,1.0f,1.0f);
		shader.setFloat("dotLights[1].constant",1.0f);
		shader.setFloat("dotLights[1].linear",0.09f);
		shader.setFloat("dotLights[1].quadratic",0.032f);
		//dot light 3
		shader.setVec3( "dotLights[2].position",dotLightPositions[2]);
		shader.setVec3( "dotLights[2].ambient",0.05f,0.05f,0.05f);
		shader.setVec3( "dotLights[2].diffuse",0.8f,0.8f,0.8f);
		shader.setVec3( "dotLights[2].specular",1.0f,1.0f,1.0f);
		shader.setFloat("dotLights[2].constant",1.0f);
		shader.setFloat("dotLights[2].linear",0.09f);
		shader.setFloat("dotLights[2].quadratic",0.032f);
		//dot light 4
		shader.setVec3( "dotLights[3].position",dotLightPositions[3]);
		shader.setVec3( "dotLights[3].ambient",0.05f,0.05f,0.05f);
		shader.setVec3( "dotLights[3].diffuse",0.8f,0.8f,0.8f);
		shader.setVec3( "dotLights[3].specular",1.0f,1.0f,1.0f);
		shader.setFloat("dotLights[3].constant",1.0f);
		shader.setFloat("dotLights[3].linear",0.09f);
		shader.setFloat("dotLights[3].quadratic",0.032f);
		//spot light
		shader.setVec3( "spotLight.position",camera.position);
		shader.setVec3( "spotLight.direction",camera.front);
		shader.setFloat("spotLight.cutOff",glm::cos(glm::radians(12.5f)));
		shader.setFloat("spotLight.outerCutOff",glm::cos(glm::radians(17.5f)));
		shader.setVec3( "spotLight.ambient",0.0f,0.0f,0.0f);
		shader.setVec3( "spotLight.diffuse",1.0f,1.0f,1.0f);
		shader.setVec3( "spotLight.specular",0.0f,0.0f,0.0f);
		shader.setFloat("spotLight.constant",1.0f);
		shader.setFloat("spotLight.linear",0.09f);
		shader.setFloat("spotLight.quadratic",0.032f);
		//material
		shader.setVec3("material.ambient",1.0f,0.5f,0.31f);
		shader.setVec3("material.diffuse",1.0f,0.5f,0.31f);
		shader.setVec3("material.specular",0.5f,0.5f,0.5f);
		shader.setFloat("material.shininess",32.0f);
		//viewPos and matrix
		shader.setVec3("viewPos",camera.position);
		shader.setMat4("viewMatrix",glm::value_ptr(viewTrans),false);
		shader.setMat4("projectionMatrix",&projectionTrans[0][0],false);

		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,diffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,specularMap);
		//bind yy map
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,yyMap);
		
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
	
unsigned int loadTexture(char const* path){
	unsigned int textureId;
	glGenTextures(1,&textureId);
	int width,height,nrComponents;
	stbi_set_flip_vertically_on_load(true);
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


