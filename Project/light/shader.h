#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader{
public:
	unsigned int programId;

	Shader(const char* vertexPath,const char* fragmentPath){
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit|std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit|std::ifstream::badbit);
		try{
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream,fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}catch(std::ifstream::failure& e){
			std::cout << "error::shader::file not succesfully read\n"<< std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		unsigned int vertexShader,fragmentShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader,1,&vShaderCode,NULL);
		glCompileShader(vertexShader);
		checkCompileErrors(vertexShader,"VERTEX");

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader,1,&fShaderCode,NULL);
		glCompileShader(fragmentShader);
		checkCompileErrors(fragmentShader,"FRAGMENT");

		programId = glCreateProgram();
		glAttachShader(programId,vertexShader);
		glAttachShader(programId,fragmentShader);
		glLinkProgram(programId);
		checkLinkErrors(programId,"PROGRAM");

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void use(){
		glUseProgram(programId);
	}

	void setBool(const std::string &name,bool value) const {
		glUniform1i(glGetUniformLocation(programId,name.c_str()),(int)value);
	}

	void setInt(const std::string &name,int value) const {
		glUniform1i(glGetUniformLocation(programId,name.c_str()),value);
	}

	void setFloat(const std::string &name,float value) const {
		glUniform1f(glGetUniformLocation(programId,name.c_str()),value);
	}
	
	void setMat4(const std::string &name,const float* matrix,bool transpose){
                glUniformMatrix4fv(glGetUniformLocation(programId,name.c_str()),1,transpose,matrix);
        }

	void setVec3(const std::string &name,float v1,float v2,float v3){
		glUniform3f(glGetUniformLocation(programId,name.c_str()),v1,v2,v3);
	}

	void setVec3(const std::string &name,const glm::vec3 &value) const {
		glUniform3fv(glGetUniformLocation(programId,name.c_str()),1,&value[0]);
	}
				
				

private:
	void checkCompileErrors(unsigned int shader,std::string type){
		int success;
		char infoLog[1024];
		glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
		if(!success){
			glGetShaderInfoLog(shader,1024,NULL,infoLog);
			std::cout << "error::shader compile error of type: " << type << "\n" << infoLog << "\n -- -------------------------------- -- " << std::endl;
		}
	}

	void checkLinkErrors(unsigned int programId,std::string type){
		int success;
		char infoLog[1024];
		glGetProgramiv(programId,GL_LINK_STATUS,&success);
		if(!success){
			glGetProgramInfoLog(programId,1024,NULL,infoLog);
			std::cout << "error:program link error of type: " << type << "\n" << infoLog << "\n -- ---------------------------------- -- " << std::endl;
		}
	}
};
#endif
