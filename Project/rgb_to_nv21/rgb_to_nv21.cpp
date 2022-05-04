#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_m.h"
#include "stb_image.h"

using namespace std;

//https://zhuanlan.zhihu.com/p/427975854

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int win_w = SCR_WIDTH;
int win_h = SCR_HEIGHT;

const char *input_file = "../../images/awesomeface.rgb24.rgb";
int w = 512;
int h = 512;
long long  len = w*h*3;
unsigned char *data = new unsigned char[len];
unsigned char *data_out = nullptr;
ifstream in;
ofstream out;

void rgb_to_nv21(unsigned char *rgb, int w, int h, unsigned char **nv21) {
	if(*nv21 == nullptr) {
		*nv21 = new unsigned char[w*h*3/2];
	}
	int y,u,v,r,g,b;
	//vu起始位置
	long long vu_index = w*h;
	unsigned char *data = *nv21;
	for(int i=0; i<h; i++) {
		for(int j=0; j<w; j++) {
			int rd = i*w*3 + j*3;
			r = rgb[rd];
			g = rgb[rd+1];
			b = rgb[rd+2];

			y = (77 * r + 150 * g + 29 * b) >> 8;
			u = ((-44 * r - 87 * g + 131 * b) >> 8) + 128;
			v = ((131 * r - 110 * g - 21 * b) >> 8) + 128;	

			y = (unsigned char) (y > 255 ? 255 : (y < 0 ? 0 : y));
			u = (unsigned char) (u > 255 ? 255 : (u < 0 ? 0 : u));
			v = (unsigned char) (v > 255 ? 255 : (v < 0 ? 0 : v));

			data[i*w+j] = y;

			//行列index为偶数时采集一次vu，注意index从0开始
			if(i % 2 == 0 && j % 2 == 0) {
				data[vu_index] = v;
				data[vu_index+1] = u;
				vu_index += 2;
			}
		}
	}
}

void init_file() {
	cout << "input_file: " << input_file << endl;
	cout << "w: " << w << " h: " << h << endl;
	
	in.open(input_file, ios::binary);
	if(!in) {
		cout << "failed to open input file: " << input_file << endl;
	}

	cout << "read rgb ... " << endl;
	in.read(reinterpret_cast<char *>(data), len);
	streamsize rds = in.gcount();
	in.close();
	cout << "read: " << rds << "/" << len << endl;
	
}

void write_nv21() {
	const char *output_file = "../../images/awesomeface_cpu.nv21.yuv";
	out.open(output_file, ios::binary);
	if(!out) {
		cout << "failed to open output file: " << output_file << endl;
	}
	cout << "change to nv21..." << endl;
	rgb_to_nv21(data, w, h, &data_out);

	cout << "write cpu_nv21..." << endl;
	out.write(reinterpret_cast<char *>(data_out), w*h*3/2);
	out.close();
	cout << "write cpu_nv21 ok" << endl;
}

void release_data() {
	delete[] data;
	delete[] data_out;
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    init_file(); //从文件中读取rgb数据
    write_nv21(); //将rgb转成nv21并写入文件，cpu直接转，主要用于测试

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("4.2.texture.vs", "4.2.texture.fs");
    Shader screenShader("screen.vs", "screen.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load rgb, 加载rgb数据到纹理1
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);

    //FBO离屏渲染,用于读取纹理，保存到文件中
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int texture_fbo;
    glGenTextures(1, &texture_fbo);
    glBindTexture(GL_TEXTURE_2D, texture_fbo);
    //用于存储nv21的纹理，宽高的解释请看fragment shader
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w/4, h*3/2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_fbo, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
	    cout << "error: framebuffer: framebuffer is not complete!" << endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    ourShader.setInt("texture1", 0);
    ourShader.setFloat("u_offset", 1.0f/w);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // render loop
    // -----------
    int count = 0;
    while (!glfwWindowShouldClose(window))
    {
        glViewport(0, 0, w/4, h*3/2);

        // input
        // -----
        processInput(window);

	//render to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
		
        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//保存rgb数据
	if(count == 0) {
		count = 1;
		data_out = new unsigned char[w*h*3/2];
		glReadPixels(0, 0, w/4, h*3/2, GL_RGBA, GL_UNSIGNED_BYTE, data_out);
		ofstream out_file("../../images/awesomeface_gpu.nv21.yuv", ios::binary);
		if(out_file) {
			cout << "write gpu_nv21..." << endl;
			out_file.write(reinterpret_cast<char *>(data_out), w*h*3);
			out_file.close();
			cout << "write gpu_nv21 ok" << endl;
		}else {
			cout << "error to create output file for writing gpu_nv21!" << endl;
		}
	}

        // render to screen
        // ------
        glViewport((win_w - w)/2, (win_h - h)/2, w, h);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

	screenShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteFramebuffers(1, &fbo);

    release_data();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    win_w = width;
    win_h = height;
    glViewport(0, 0, width, height);
}
