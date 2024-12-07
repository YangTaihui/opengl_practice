#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"
#include "model_control.h"
#include "colormap.h"
#include "functions.h"
#define PI 3.1416

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 640;

glm::vec3 camera_position = glm::vec3(2.0f, 2, 2.0f);
glm::vec3 look_point = glm::vec3(0.0f, 0.0f, 0.0f);
ModelControl model;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// longitudes
float dv = 1.0f;
int nv = (int)360 / dv + 1;

// latitudes
float du = 1.0f;
int nu = (int)180 / du+1;

int main()
{
    int i, j, k=0;

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("ball_img.png", &width, &height, &nrChannels, 0);
    std::cout << "w " << width << " h " << height << " nc " << nrChannels << std::endl;

    float* u_arr = calloc1d_float(nu);
    float* v_arr = calloc1d_float(nv);

    int grid_w = 10, grid_h = 8;
    int one_grid_n = 4*width * height / grid_w / grid_h;
    //for (j = 0;j < grid_h;j++) {
    //    for (i = 0; i < grid_w; i++) {
    //        if (i % 2 == j % 2) {
    //            for (int m = 0; m < height/ grid_h; m++) {
    //                for (int n = 0; n < width / grid_w; n++) {
    //                    data[j * grid_w * one_grid_n + 4 * i * width / grid_w + 4 * m * width + 4 * n] = 255;
    //                    data[j * grid_w * one_grid_n + 4 * i * width / grid_w + 4 * m * width + 4 * n + 1] = 255;
    //                    data[j * grid_w * one_grid_n +4*i* width/grid_w+ 4*m* width+4*n+2] = 255;
    //                    data[j * grid_w * one_grid_n +4*i* width/grid_w+ 4*m* width+4*n+3] = 40;
    //                }
    //            }
    //        }
    //    }
    //}

    for (i = 0; i < nu; i++) {
        u_arr[i] = (i*du - 90)/180.0f*PI;
    }

    for (j = 0; j < nv; j++) {
        v_arr[j] = j*dv/180.0f*PI;
    }

    float* plt_u = calloc1d_float(nv * nu);
    float* plt_v = calloc1d_float(nv * nu);

    for (j = 0; j < nu; j++) {
        for (i = 0; i < nv; i++) {
            plt_v[j* nv+i] = v_arr[i];
            plt_u[j* nv+i] = u_arr[j];
        }
    }

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Globe", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("./ball.vs", "./ball.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float* vertices = calloc1d_float(nv*nu*2);
    for (i = 0; i < nv * nu; i += 1) {
        vertices[2*i] = plt_u[i];
        vertices[2*i+1] = plt_v[i];
    }

    int now_id, indices_n = (nu - 1) * (2* nv +1);
    unsigned int* indices = calloc1d_uint(indices_n);
    for (j = 0; j < nu - 1; j++) {
        for (i = 0; i < nv; i++) {
            now_id = 2 * (j * nv + i) + j;
            indices[now_id] = j * nv + i;
            indices[now_id + 1] = (j + 1) * nv + i;
            if (i== nv -1)
                indices[now_id + 2] = 0xFFFF;
        }
    }
    indices[indices_n - 1] = indices[indices_n - 2];

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nu * nv * 8, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices_n, indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load and create a texture 
    // -------------------------
    unsigned int texture1;
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLint textureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize);
    std::cout << "GL_MAX_TEXTURE_SIZE " << textureSize << std::endl;

    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    glm::vec3 camera_front = glm::normalize(-camera_position);
    glm::vec3 camera_right = glm::normalize(glm::cross(camera_front, glm::vec3(0.0, 1.0, 0.0)));
    glm::vec3 camera_up = glm::normalize(glm::cross(camera_right, camera_front));
    glm::mat4 view = glm::lookAt(camera_position, look_point, camera_up);
    ourShader.setMat4("view", view);
    glm::mat4 projection = glm::perspective(glm::radians(model.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("model", model.Model);

    // render loop
    // -----------
    float rad = 0.0f;
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

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        // render container
        ourShader.use();

        //rad += 0.05f;
        //glm::mat4 tran = glm::rotate(glm::mat4(1.0f), glm::radians(rad), glm::vec3(0.0, 1.0, 0.0));
        //ourShader.setMat4("model", tran);

        ourShader.setMat4("model", model.Model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLE_STRIP, indices_n, GL_UNSIGNED_INT, 0);

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
    //free(data);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        model.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        model.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        model.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        model.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        model.ProcessKeyboard(RESET, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
