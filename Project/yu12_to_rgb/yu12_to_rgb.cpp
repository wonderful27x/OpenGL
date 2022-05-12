#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_m.h"
#include "stb_image.h"


using namespace std;

//todo 有些分辨率的文件播放不了，比如426x240

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int win_w = SCR_WIDTH;
int win_h = SCR_HEIGHT;

//const char *input_file = "../../images/movie_426_240_yu12.yuv";
//int w = 426;
//int h = 240;
int w = 768;
int h = 432;
string input_file = string("../../images/movie_") + to_string(w) + "_" + to_string(h) + "_yu12.yuv";
long long  len = w*h*3/2;
unsigned char *data = new unsigned char[len];
unsigned char *data_out = nullptr;
ifstream in;
ofstream out;

void yu12_to_rgb(unsigned char *yuv,int width,int height, unsigned char **data) {
    if(*data == nullptr) {
	    *data = new unsigned char[height*width*3];
    }

    //动态分配内存连续的二维数组
    //data是一块连续的内存
    //rgb是一个指针数组，数组的每个元素是一个unsigned char的指针,rgb是动态分配的，需要delete[]
    //在for循环中让数组的每个元素指针指向连续的内存地址
    unsigned char **rgb = new unsigned char *[height];
    for(int i=0; i<height; i++) {
	    rgb[i] = *data + i*width*3;
    }

    ////rgb是一个指针数组，但不是动态分配的，rgb本身不需要delete[]
    //unsigned char *rgb[] = unsigned char *[height];
    //for(int i=0; i<height; i++) {
    //        rgb[i] = *data + i*width*3;
    //}

    //定义一个二维数组存储像素，与openCv的Mat矩阵式是对应的，
    //Mat(高，宽), CV_8UC3:三个通道，每个通道8位
    //unsigned char rgb[height][width * 3];
    unsigned char r, g, b;                   //r g b分量
    int y, v, u;                       //y u v分量
    int p_y, p_v, p_u;                 //y u v分量的位置索引
    int frameSize = width * height;

    long long i = 0;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {

	    if((++i)%100==0)printf("for %lld : %lld\n", i, (long long)width*height);

            int index_u = frameSize + h / 2 * width / 2 + w / 2;
            int index_v = frameSize + frameSize/4 + h / 2 * width / 2 + w / 2;
            p_y = h * width + w;
            p_u = index_u;
            p_v = index_v;

            y = yuv[p_y];
            u = yuv[p_u];
            v = yuv[p_v];

            //公式六（网上）
            int R = y + ((360 * (v - 128))>>8) ;
            int G = y - ((( 88 * (u - 128)  + 184 * (v - 128)))>>8) ;
            int B = y +((455 * (u - 128))>>8) ;

            r = (unsigned char) (R > 255 ? 255 : (R < 0 ? 0 : R));
            g = (unsigned char) (G > 255 ? 255 : (G < 0 ? 0 : G));
            b = (unsigned char) (B > 255 ? 255 : (B < 0 ? 0 : B));

            rgb[h][w * 3 + 0] = r;
            rgb[h][w * 3 + 1] = g;;
            rgb[h][w * 3 + 2] = b;
        }
    }
    delete[] rgb;
}

void init_file() {
	cout << "input_file: " << input_file << endl;
	cout << "w: " << w << " h: " << h << endl;
	
	in.open(input_file, ios::binary);
	if(!in) {
		cout << "failed to open input file: " << input_file << endl;
	}

	cout << "read yu12 ... " << endl;
	in.read(reinterpret_cast<char *>(data), len);
	streamsize rds = in.gcount();
	in.close();
	cout << "read: " << rds << "/" << len << endl;
	
}

void write_rgb() {
	//const char *output_file = "../../images/movie_426_240_cpu.rgb";
	string output_file = string("../../images/movie_") +  to_string(w) + "_" + to_string(h) + "_cpu.rgb";
	out.open(output_file, ios::binary);
	if(!out) {
		cout << "failed to open output file: " << output_file << endl;
	}
	cout << "change to rgb..." << endl;
	yu12_to_rgb(data, w, h, &data_out);

	cout << "write cpu_rgb..." << endl;
	out.write(reinterpret_cast<char *>(data_out), w*h*3);
	out.close();
	cout << "write cpu_rgb ok" << endl;
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

    init_file(); //从文件中读取yu12数据
    write_rgb();

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
    unsigned int texture1, texture2, texture3;
    // texture 1 -> y
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);

    // texture 2 -> u
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w>>1, h>>1, 0, GL_RED, GL_UNSIGNED_BYTE, (data + w*h));

    // texture 2 -> v
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w>>1, h>>1, 0, GL_RED, GL_UNSIGNED_BYTE, (data + w*h + w*h/4));

    //FBO离屏渲染,用于读取纹理，保存到文件中
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int texture_fbo;
    glGenTextures(1, &texture_fbo);
    glBindTexture(GL_TEXTURE_2D, texture_fbo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
    // either set it manually like so:
    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    // or set it via the texture class
    ourShader.setInt("texture2", 1);
    ourShader.setInt("texture3", 2);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // render loop
    // -----------
    int count = 0;
    while (!glfwWindowShouldClose(window))
    {
	//glViewPort渲染窗口大小为图片宽高
	//归一化顶点坐标-1.0到1.0将映射到此宽高上，而纹理坐标与顶点坐标绑定，
	//所以可以理解为纹理坐标也映射到此坐标上(此例子中纹理坐标没有裁剪，从0到1.0)
	//所以在片段着色器中每次运行的纹理坐标为(1/w, 1/h)*i, 
	//即对于宽，步长为1/w, 总步数为w
	//对于高，步长为1/h, 总步数为h
	//于是当对y分量的纹理进行采样时，由于纹理的宽高就等于w/h,所以正好每次对应准确的像素
	//而对于uv分量，由于宽高均为w,h的二分之一，所以按照1/w, 1/h的步长进行采样时，每个坐标并不是与像素准确对应
	//但是通过纹理过滤仍然可以得到所对应的像素,可以想象为将其放大到glViewport设置的尺寸，再采样
        glViewport(0, 0, w, h);

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
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture3);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//保存rgb数据
	if(count == 0) {
		count = 1;
		data_out = new unsigned char[w*h*3];
		glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data_out);
		string name = string("../../images/movie_") +  to_string(w) + "_" + to_string(h) + "_gpu.rgb";
		ofstream out_file(name, ios::binary);
		if(out_file) {
			cout << "write gpu_rgb..." << endl;
			out_file.write(reinterpret_cast<char *>(data_out), w*h*3);
			out_file.close();
			cout << "write gpu_rgb ok" << endl;
		}else {
			cout << "error to create output file for writing gpu_rgb!" << endl;
		}
	}

        // render to screen
        // ------
        glViewport((win_w - w)/2, (win_h - h)/2, w, h);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_fbo);

	screenShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
	
	
	////no fbo
        //glViewport(0, 0, w, h);

        //// input
        //// -----
        //processInput(window);

        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
	//	
        //// bind textures on corresponding texture units
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture1);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture2);
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, texture3);

        //// render container
        //ourShader.use();
        //glBindVertexArray(VAO);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        //// -------------------------------------------------------------------------------
        //glfwSwapBuffers(window);
        //glfwPollEvents();
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
