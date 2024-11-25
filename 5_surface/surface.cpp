#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/shader_m.h"
#include "../include/camera.h"
#include "../include/colormap.h"
#include "../include/functions.h"
 
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 640;

// camera
Camera camera(glm::vec3(2.0f, 2.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool use_cull_face = false;
bool correct_cull_face = false;
bool use_primitive_restart = true;

int main()
{
    int i, j, k=0;
    int nrChannels = 3;

    float xstart = -5;
    float xstop = 5;
    float xstep = 0.1;
    float zstart = -5;
    float zstop = 5;
    float zstep = 0.1;

    float xlen = xstop - xstart;
    float zlen = zstop - zstart;
    int width = xlen / xstep + 1;
    int height = zlen / zstep + 1;
    
    float* x_arr = calloc1d_float(width);
    float* z_arr = calloc1d_float(height);
    for (i = 0; i < width; i++) {
        x_arr[i] = xstart+i*xstep;
    }
    for (j = 0; j < height; j++) {
        z_arr[j] = zstart + j * zstep;
    }

    float x, z, x2,z2;
    float** data_raw = malloc2d(width, height);
    for (i = 0; i < width; i++) {
        for (j = 0; j < height; j++) {
            x = x_arr[i];
            z = z_arr[j];
            x2 = x * x;
            z2 = z * z;
            data_raw[i][j] = 3 * (1 - x) * (1 - x) * exp(-(x2)-(z + 1) * (z + 1))
                - 10 * (x / 5 - x2 * x - z2 * z2 * z) * exp(-x2 - z2)
                - 1.0 / 3 * exp(-(x + 1) * (x + 1) - z2);
        }
    }
    min_max_2d(data_raw, width, height, 1);

    float* plt_x = calloc1d_float(width * height);
    float* plt_y = calloc1d_float(width * height);
    float* plt_z = calloc1d_float(width * height);

    float xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0, zmin = 0.0, zmax = 0.0;
    for (i = 0; i < width; i++){
        xmin = std::min(xmin, x_arr[i]);
        xmax = std::max(xmax, x_arr[i]);
        for (j = 0; j < height; j++){
            ymin = std::min(ymin, data_raw[i][j]);
            ymax = std::max(ymax, data_raw[i][j]);
        }
    }
    for (j = 0; j < height; j++) {
        zmin = std::min(zmin, z_arr[j]);
        zmax = std::max(zmax, z_arr[j]);
    }
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            plt_x[k] = 2*(x_arr[i] - xmin)/(xmax-xmin)-1;
            plt_y[k] = 2*(data_raw[i][j] - ymin)/(ymax-ymin)-1;
            plt_z[k] = 2*(z_arr[j]-zmin)/(zmax-zmin)-1;
            k += 1;
        }
    }
    std::cout << width << " " << height << std::endl;
    unsigned char* data = (unsigned char*)calloc(height * width * nrChannels, sizeof(unsigned char));
     
    int now_id, now_v;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            now_id = (j * width + i) * 3;
            now_v = round(data_raw[i][j]);
            if (now_v < 0)
                now_v = 0;
            data[now_id] = colormap_viridis[now_v][0];
            data[now_id + 1] = colormap_viridis[now_v][1];
            data[now_id + 2] = colormap_viridis[now_v][2];
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Surface", NULL, NULL);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    if (use_cull_face)
        glEnable(GL_CULL_FACE);
    if (use_primitive_restart) {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFF);
    }
    glEnable(GL_DEPTH_TEST);
    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("./surface.vs", "./surface.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float* vertices = calloc1d_float(width*height*3);
    for (i = 0; i < width * height; i += 1) {
        vertices[3*i] = plt_x[i];
        vertices[3*i+1] = plt_y[i];
        vertices[3*i+2] = plt_z[i];
    }


    int indices_n = 0;
    if (correct_cull_face or use_primitive_restart)
        indices_n = (height - 1) * (2*width+1);//-2
    else
        indices_n = 2 * (height - 1) * width;

    unsigned int* indices = calloc1d_uint(indices_n);
    if (use_primitive_restart) {
        for (j = 0; j < height - 1; j++) {
            for (i = 0; i < width; i++) {
                now_id = 2 * (j * width + i) + j;
                indices[now_id] = j * width + i;
                indices[now_id + 1] = (j + 1) * width + i;
                if (i==width-1)
                    indices[now_id + 2] = 0xFFFF;
            }
        }

        indices[indices_n - 1] = indices[indices_n - 2];
    }
    else {
        if (correct_cull_face) {
            for (j = 0; j < height - 1; j++) {
                for (i = 0; i < width; i++) {
                    now_id = 2 * (j * width + i) + j;
                    if (j % 2 == 0) {
                        indices[now_id] = j * width + i;
                        indices[now_id + 1] = (j + 1) * width + i;
                        if (i == width - 1)
                            indices[now_id + 2] = (j + 1) * width + i;
                    }
                    else {
                        indices[now_id] = (j + 1) * width - i - 1;
                        indices[now_id + 1] = (j + 2) * width - i - 1;
                        if (i == width - 1)
                            indices[now_id + 2] = (j + 2) * width - i - 1;
                    }
                }
            }
            indices[indices_n - 1] = indices[indices_n - 2];
        }
        else {
            for (j = 0; j < height - 1; j++) {
                for (i = 0; i < width; i++) {
                    now_id = 2 * (j * width + i);
                    if (j % 2 == 0) {
                        indices[now_id] = j * width + i;
                        indices[now_id + 1] = (j + 1) * width + i;
                    }
                    else {
                        indices[now_id] = (j + 1) * width - i - 1;
                        indices[now_id + 1] = (j + 2) * width - i - 1;
                    }
                }
            }
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, width* height * 12, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * indices_n, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load and create a texture 
    // -------------------------
    unsigned int texture1;
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!


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

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // render container
        ourShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}