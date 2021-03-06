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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int win_w = SCR_WIDTH;
int win_h = SCR_HEIGHT;

const char *input_file = "../../images/awesomeface.nv21.yuv";
int w = 512;
int h = 512;
long long  len = w*h*3/2;
unsigned char *data = new unsigned char[len];
unsigned char *data_out = nullptr;
ifstream in;
ofstream out;

/**
 * NV21转RGB,这是根据NV21数据结构和存储方式自己写的算法
 * @param nv21
 * @param width
 * @param height
 * @return
 * https://www.pianshen.com/article/4792452708/
 */
void nv21_to_rgb(unsigned char *nv21,int width,int height, unsigned char **data) {
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

            //y u v分量位置索引算法（自己：效率低，但容易理解）
            //当前像素y分量对应的uv分量的索引，有些复杂，可以一步一步验证
	    //解释:
	    //首先循环的单位是像素，即图片宽高，而不是字节
	    //frameSize: 为y分量的大小，跳过y,将指向vu分量的位置
	    //h/2*width: 注意nv21的采样方式，每四个Y对应一组vu,且这四个Y是相邻两行上下对陈的四个Y，而不是同一行连续的四个Y，这一点非常重要
	    //也就是每两行Y对应所需的vu的字节数为width*2/2=width,所以h/2*width代表遍历的Y所对应的需要跳过的vu字节数
	    //w/2*2: 代表横向每两个像素共用一组vu,一组vu占两个字节
            int index = frameSize + h / 2 * width + w / 2 * 2;
            //这里使用的是NV21格式，计算方式需要结合NV21的排列
            p_y = h * width + w;
            p_v = index;
            p_u = index + 1;

            y = nv21[p_y];
            v = nv21[p_v];
            u = nv21[p_u];

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

	cout << "read nv21 ... " << endl;
	in.read(reinterpret_cast<char *>(data), len);
	streamsize rds = in.gcount();
	in.close();
	cout << "read: " << rds << "/" << len << endl;
	
}

void write_rgb() {
	const char *output_file = "../../images/awesomeface_cpu.rgb";
	out.open(output_file, ios::binary);
	if(!out) {
		cout << "failed to open output file: " << output_file << endl;
	}
	cout << "change to rgb..." << endl;
	nv21_to_rgb(data, w, h, &data_out);

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

    init_file(); //从文件中读取nv21数据
    write_rgb(); //将nv21转成rgb并写入文件，cpu直接转，主要用于测试

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
    //load y,加载Y分量到纹理1
    //glTexImage2D会将data按照图片的格式来解析，尽管data在这里是连续的内存，但会把它解析成具有宽高的图片
    //所以这里可以理解为将y分量单独解析成纹理图片,只有转换成纹理，之后才能根据纹理坐标采集坐标对应的像素,uv分量同理
    //2D纹理都是RGBA格式的，GL_RED方式会对data的每一个元素解析为纹理的R分量，并将GB分量设置为0,A分量设置为1,
    //即data的一个y被解析为R001,具体看官网注释
    //GLint swizzleMask[] = {GL_RED, GL_ZERO, GL_ZERO, GL_ZERO};
    //glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);
    //load rgb
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data_out);
    //// load image, create texture and generate mipmaps
    //int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    //// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    //unsigned char *data = stbi_load("../../images/woodtexture2.jpg", &width, &height, &nrChannels, 0);
    //if (data)
    //{
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //    glGenerateMipmap(GL_TEXTURE_2D);
    //    //test
    //    cout << "w:h:c = " << width << ":" << height << ":" << nrChannels << endl;
    //    ofstream rgb("../../images/woodtexture2_cpu.rgb", ios::binary);
    //    rgb.write(reinterpret_cast<char *>(data), width * height * 3);
    //    rgb.close();
    //}
    //else
    //{
    //    std::cout << "Failed to load texture" << std::endl;
    //}
    //stbi_image_free(data);

    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load uv, 加载vu分量到纹理2
    //将uv分量解析为纹理图片，GL_RG方式会对data的每两个元素分别解析为RG分量，B分量赋值为0，A分量为1;
    //GLint swizzleMask2[] = {GL_RED, GL_GREEN, GL_ZERO, GL_ZERO};
    //glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, w>>1, h>>1, 0, GL_RG, GL_UNSIGNED_BYTE, (data + w*h));
    //load rgb
    //glGenerateMipmap(GL_TEXTURE_2D);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data_out);
    //// load image, create texture and generate mipmaps
    //data = stbi_load("../../images/awesomeface.png", &width, &height, &nrChannels, 0);
    //if (data)
    //{
    //    // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //    glGenerateMipmap(GL_TEXTURE_2D);
    //}
    //else
    //{
    //    std::cout << "Failed to load texture" << std::endl;
    //}
    //stbi_image_free(data);

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

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	//保存rgb数据
	if(count == 0) {
		count = 1;
		data_out = new unsigned char[w*h*3];
		glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data_out);
		ofstream out_file("../../images/awesomeface_gpu.rgb", ios::binary);
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
