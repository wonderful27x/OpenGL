#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_m.h"
#include "stb_image.h"
#include "decode.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdio>

/* TODO 当有字节对齐时，纹理的居中问题 */
/* TODO 纹理图像颠倒180度 */
/* TODO 播放速度控制 */
/* TODO 音频播放 */

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp);
void fillYUV(unsigned char *y, unsigned char *u, unsigned char *v, AVFrame *frame);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int win_w = SCR_WIDTH;
int win_h = SCR_HEIGHT;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
//float yaw   = 90.0f;
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main(int argc, char *argv[])
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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    /* 初始化解码器 */
    /* Decode decode("/home/wonderful/wonderful/FILE/video-audio/LOL.mp4"); */
    /* FILE *t = fopen("../../tmp/LOL_std.mp4", "rb"); */
    /* if(t) { */
    /*     std::cout << "Success to open ../../tmp/LOL_std.mp4" << std::endl; */
    /* }else{ */
    /*     std::cout << "Failed to open ../../tmp/LOL_std.mp4" << std::endl; */
    /* } */
    /* fclose(t); */

    /* Decode decode("../../tmp/LOL_std.mp4"); */
    std::string movie_path = "../../tmp/Smooth_criminal.mp4";
    if(argc > 1)
    {
        movie_path = argv[1];
    }
    std::cout << "Movie path: " << movie_path << std::endl;
    Decode decode(movie_path);
    int videoWidth = decode.getWidth();
    int videoHeight = decode.getHeight();
    int frameDuration = 1000 / decode.getFrameRate();
    unsigned char *Y = new unsigned char[videoWidth * videoHeight];
    unsigned char *U = new unsigned char[videoWidth * videoHeight / 4];
    unsigned char *V = new unsigned char[videoWidth * videoHeight / 4];
    /* 测试加载静态yuv数据 */
    //std::ifstream in("/home/wonderful/wonderful/FILE/video-audio/movie_426_240_yu12.yuv", std::ios::binary);
    //if(in) {
    //        long long len = videoWidth*videoHeight*3/2;
    //        unsigned char yu12[len];
    //        in.read(reinterpret_cast<char *>(yu12), len);
    //        long long rds = in.gcount();
    //        in.close();
    //        std::cout << "read: " << rds << "/" << len << std::endl;
    //        len = videoWidth * videoHeight;
    //        memcpy(Y, yu12, len);
    //        memcpy(U, yu12 + len, len/4);
    //        memcpy(V, yu12 + len + len/4, len/4);
    //}

    /* 测试用，发现426x240这个分辨率无法播放 */
    /* FILE *in = fopen("/home/wonderful/wonderful/FILE/video-audio/movie_426_240_yu12.yuv", "rb"); */
    std::string name = std::string("../../tmp/movie_") + std::to_string(videoWidth) + "_" + std::to_string(videoHeight) + "_yu12.yuv";
    FILE *in = fopen(name.c_str(), "rb");
    if(in) {
        long long len = videoWidth*videoHeight*3/2;
        unsigned char *yu12 = new unsigned char[len];
        long long rds = fread(yu12, len, 1, in);
	    fclose(in);
        std::cout << "read: " << rds << "/" << len << std::endl;
        len = videoWidth * videoHeight;
        memcpy(Y, yu12, len);
        memcpy(U, yu12 + len, len/4);
        memcpy(V, yu12 + len + len/4, len/4);
        delete[] yu12;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("7.3.camera.vs", "7.3.camera.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    // world space positions of our cubes
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
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("../../images/woodtexture2.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load("../../images/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    /* 创建yuv纹理 */
    std::cout << "w: " << videoWidth << " h: " << videoHeight << std::endl;
    unsigned int yuv[3];
    glGenTextures(3, yuv);
    //y
    glBindTexture(GL_TEXTURE_2D, yuv[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth, videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth, videoHeight, 0, GL_RED, GL_UNSIGNED_BYTE, Y);
    //u 
    glBindTexture(GL_TEXTURE_2D, yuv[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth / 2, videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth / 2, videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, U);
    //v
    glBindTexture(GL_TEXTURE_2D, yuv[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth / 2, videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, videoWidth / 2, videoHeight / 2, 0, GL_RED, GL_UNSIGNED_BYTE, V);
    //bind to default
    //glBindTexture(GL_TEXTURE_2D, 0);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);
    ourShader.setInt("textureY", 2);
    ourShader.setInt("textureU", 3);
    ourShader.setInt("textureV", 4);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

	//glViewport(0, 0, videoWidth, videoHeight);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
	
        // activate shader
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        //glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 view = calculate_lookAt_matrix(cameraPos, cameraPos + cameraFront, cameraUp);
        ourShader.setMat4("view", view);

        // render boxes
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model", model);

            if(i==2) {
                    //glViewport(0, 0, videoWidth, videoHeight);

                AVFrame *frame = decode.getFrame();
                //AVFrame *frame = nullptr;
                if(frame) {
                    /* opengl硬件也有字节对齐，去掉ffmpeg解码对齐的无效数据会使得有些分辨率无法播放!!! */
                    ////std::cout << "decode a frame ok" << std::endl;
                    //fillYUV(Y, U, V, frame);
                    ////y
                    //glActiveTexture(GL_TEXTURE2);
                    //glBindTexture(GL_TEXTURE_2D, yuv[0]);
                    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width, frame->height, GL_RED, GL_UNSIGNED_BYTE, Y);
                    ////u
                    //glActiveTexture(GL_TEXTURE3);
                    //glBindTexture(GL_TEXTURE_2D, yuv[1]);
                    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width / 2, frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, U);
                    ////v
                    //glActiveTexture(GL_TEXTURE4);
                    //glBindTexture(GL_TEXTURE_2D, yuv[2]);
                    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->width / 2, frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, V);

                    /* opengl硬件也有字节对齐，所以直接使用ffmpeg解码后对齐的数据，不做处理， */
                    /* TODO 但是这样会造成显示多出一块区域!!! */
                    static int first = true;
                    double scale = -1.0;
                    if(first) {
                        first = false;
                        glBindTexture(GL_TEXTURE_2D, yuv[0]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[0], frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
                        glBindTexture(GL_TEXTURE_2D, yuv[1]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[1], frame->height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
                        glBindTexture(GL_TEXTURE_2D, yuv[2]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, frame->linesize[2], frame->height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

                        /* 计算实际宽度占总宽度的比例 */
                        scale = frame->width * 1.0 / frame->linesize[0];
                        ourShader.setFloat("scale", scale);
                    }

                    /* 去掉字节对齐，并保存文件，但是下面不使用 */
                    fillYUV(Y, U, V, frame);

                    /* /1* 我们甚至可用直接加载yuv数据 *1/ */
                    /* //y */
                    /* glActiveTexture(GL_TEXTURE2); */
                    /* glBindTexture(GL_TEXTURE_2D, yuv[0]); */
                    /* glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[0], frame->height, GL_RED, GL_UNSIGNED_BYTE, Y); */
                    /* //u */
                    /* glActiveTexture(GL_TEXTURE3); */
                    /* glBindTexture(GL_TEXTURE_2D, yuv[1]); */
                    /* glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[1], frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, U); */
                    /* //v */
                    /* glActiveTexture(GL_TEXTURE4); */
                    /* glBindTexture(GL_TEXTURE_2D, yuv[2]); */
                    /* glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[2], frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, V); */

                    //y
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, yuv[0]);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[0], frame->height, GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);
                    //u
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, yuv[1]);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[1], frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, frame->data[1]);
                    //v
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, yuv[2]);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame->linesize[2], frame->height / 2, GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);

                    /* 跳过字节对齐的宽度，让纹理剧中, 通过glViewport似乎无法做到, 因为纹理坐标和顶点坐标已经绑定了, */
                    /* glViewport只能改变绘制的区域, 即顶点和纹理共同的位置，它们的相对位置并不会改变 */
                    //if(scale > 0) {
                    //        int x = (1.0 - scale) * win_w;
                    //        glViewport(-x, 0, win_w + x, win_h);
                    //        std::cout  << -x << " " << 0 << " " << win_w + x << " " << win_h << std::endl;
                    //}

                    /* 将立方体的右侧面绘制为move方式 */
                    ourShader.setInt("movie", 1);
                    glDrawArrays(GL_TRIANGLES, 30, 6);
                    /* glDrawArrays(GL_TRIANGLES, 18, 6); */
                    /* glDrawArrays(GL_TRIANGLES, 6, 6); */

                    //glViewport(0, 0, win_w, win_h);
                } else {
                    std::cout << "No available frame to use, decode to slow!!!" << std::endl;
                }
                /* 其他面正常绘制 */
                ourShader.setInt("movie", 0);
                glDrawArrays(GL_TRIANGLES, 0, 30);
                /* glDrawArrays(GL_TRIANGLES, 0, 18); */
                /* glDrawArrays(GL_TRIANGLES, 24, 12); */
                //glDrawArrays(GL_TRIANGLES, 0, 6);
                //glDrawArrays(GL_TRIANGLES, 12, 24);
            }else {
                //glViewport(0, 0, width, height);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

	    std::this_thread::sleep_for(std::chrono::milliseconds(frameDuration - 5));
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    decode.stop();

    delete[] Y;
    delete[] U;
    delete[] V;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void fillYUV(unsigned char *y, unsigned char *u, unsigned char *v, AVFrame *frame) {
    int max_count = 300;
	static int count = 0;
    static std::ofstream out1;
    static std::ofstream out2;
	if(count++ < max_count) {
        /* 将AVFrame的数据拷贝到Y U V */
        for (int i = 0; i < frame->height; ++i) {
            memcpy(y + i*frame->width, frame->data[0] + i*frame->linesize[0], frame->width);
        }
        for (int i = 0; i < frame->height/2; ++i) {
            memcpy(u + i*frame->width/2, frame->data[1] + i*frame->linesize[1], frame->width/2);
        }
        for (int i = 0; i < frame->height/2; ++i) {
            memcpy(v + i*frame->width/2, frame->data[2] + i*frame->linesize[2], frame->width/2);
        }

		/* 转换后的，去掉字节对齐数据 */
		/* std::string name = std::string("/home/wonderful/wonderful/FILE/video-audio/movie_") + std::to_string(frame->width) + "_" + std::to_string(frame->height) + "_yu12.yuv"; */
		std::string name = std::string("../../tmp/movie_") + std::to_string(frame->width) + "_" + std::to_string(frame->height) + "_yu12.yuv";
        if(count == 1) {
            out1.open(name, std::ios::binary | std::ios::trunc);
        }
		if(out1) {
			out1.write(reinterpret_cast<char *>(y), frame->width*frame->height);
			out1.write(reinterpret_cast<char *>(u), frame->width*frame->height / 4);
			out1.write(reinterpret_cast<char *>(v), frame->width*frame->height / 4);
		}
        if(count >= max_count) {
			out1.close();
        }

		/* 原始数据 */
		/* name = std::string("/home/wonderful/wonderful/FILE/video-audio/movie_") + std::to_string(frame->linesize[0]) + "_" + std::to_string(frame->height) + "_yu12o.yuv"; */
		name = std::string("../../tmp/movie_") + std::to_string(frame->linesize[0]) + "_" + std::to_string(frame->height) + "_yu12o.yuv";
        if(count == 1) {
            out2.open(name, std::ios::binary | std::ios::trunc);
        }
		if(out2) {
			std::cout << "linesize[0] = " << frame->linesize[0] << " linesize[1] = " << frame->linesize[1] << " linesize[2] = " << frame->linesize[2] << std::endl;
			out2.write(reinterpret_cast<char *>(frame->data[0]), frame->linesize[0]*frame->height);
			out2.write(reinterpret_cast<char *>(frame->data[1]), frame->linesize[1]*frame->height / 2);
			out2.write(reinterpret_cast<char *>(frame->data[2]), frame->linesize[2]*frame->height / 2);
		}
        if(count >= max_count) {
			out2.close();
        }
	}
    if(count >= max_count * 2) {
        count = max_count * 2;
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    win_w = width;
    win_h = height;
    glViewport(0, 0, win_w, win_h);
    //std::cout << "glViewport(" << width << ", " << height << ")" << std::endl;
    //glViewport(100, 100, width/2, height/2);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    std::cout << "last yaw: " << yaw;
    yaw += xoffset;
    std::cout << "  current yaw: " << yaw;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    std::cout << "  x: " << front.x << " y: " << front.y << " z: " << front.z <<std::endl;
    cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

// Custom implementation of the LookAt function
glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
{
    // 1. Position = known
    // 2. Calculate cameraDirection
    glm::vec3 zaxis = glm::normalize(position - target);
    // 3. Get positive right axis vector
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
    translation[3][0] = -position.x; // Third column, first row
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = xaxis.x; // First column, first row
    rotation[1][0] = xaxis.y;
    rotation[2][0] = xaxis.z;
    rotation[0][1] = yaxis.x; // First column, second row
    rotation[1][1] = yaxis.y;
    rotation[2][1] = yaxis.z;
    rotation[0][2] = zaxis.x; // First column, third row
    rotation[1][2] = zaxis.y;
    rotation[2][2] = zaxis.z;

    // Return lookAt matrix as combination of translation and rotation matrix
    return rotation * translation; // Remember to read from right to left (first translation then rotation)
}
