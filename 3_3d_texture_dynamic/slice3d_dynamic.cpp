#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "./include/camera.h"
#include "./include/colormap.h"
#include "./include/functions.h"
#include "./include/shader_m.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 640;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
    int i, j, k;
    char fname[] = "D:/RunData/3d_101x301x200.bin";
    int nx = 101, ny = 301, nz = 200, nrChannels = 3;
    float*** data_raw_3d = read3d(fname, nx, ny, nz);
    std::cout << nx << " " << ny << " " << nz << std::endl;
    min_max_3d(data_raw_3d, nx, ny, nz, 0.1);

    unsigned char* data = (unsigned char*)calloc(nx * ny * nz * nrChannels, sizeof(unsigned char));
    int width = nx;
    int height = ny;
    int depth = nz;
    int now_id, now_v;
    for (k = 0; k < depth; k++) {
        for (j = 0; j < height; j++) {
            for (i = 0; i < width; i++) {
                now_id = (k * height * width + j* width +i) * 3;
                now_v = round(data_raw_3d[i][j][k]);
                if (now_v < 0)
                    now_v = 0;
                data[now_id] = colormap_promax[now_v][0];
                data[now_id + 1] = colormap_promax[now_v][1];
                data[now_id + 2] = colormap_promax[now_v][2];
            }
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "slice3d_dynamic", NULL, NULL);
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

    glEnable(GL_DEPTH_TEST);
    // build and compile our shader zprogram
    Shader ourShader("./slice3d_dynamic.vs", "./slice3d_dynamic.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // index //positions(-1, 1) -> // texture coords(0, 1)
        // x
         0,  1.0f,  1.0f,
         0,  1.0f, -1.0f,
         0, -1.0f,  1.0f,
         0,  1.0f, -1.0f,
         0, -1.0f, -1.0f,
         0, -1.0f,  1.0f,
         // y
         1,  1.0f,  1.0f,
         1,  1.0f, -1.0f,
         1, -1.0f,  1.0f,
         1,  1.0f, -1.0f,
         1, -1.0f, -1.0f,
         1, -1.0f,  1.0f,
        // z
         2,  1.0f,  1.0f,
         2,  1.0f, -1.0f,
         2, -1.0f,  1.0f,
         2,  1.0f, -1.0f,
         2, -1.0f, -1.0f,
         2, -1.0f,  1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // axis
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // position attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(1 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // load and create a texture 
    // -------------------------
    unsigned int texture1;
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_3D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLint textureSize = 0, texture3DSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texture3DSize);
    std::cout << "GL_MAX_TEXTURE_SIZE " << textureSize << std::endl;
    std::cout << "GL_MAX_3D_TEXTURE_SIZE " << texture3DSize << std::endl;

    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, width, height, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_3D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
    //ourShader.setInt("texture1", 1);
    //ourShader.setInt("texture2", 0);

    float rend_speed = 0.1;
    float id0 = 0.0f, id1 = 0.0f, id2 = 0.0f;
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
        glBindTexture(GL_TEXTURE_3D, texture1);

        // render container
        ourShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        id0 += deltaTime * rend_speed;
        id1 += deltaTime * rend_speed;
        id2 += deltaTime * rend_speed;
        if (id0 > 1) {
            id0 = 1;
            id1 = 1;
            id2 = 1;
            rend_speed = -rend_speed;
        }
        if (id0 < 0) {
            id0 = 0;
            id1 = 0;
            id2 = 0;
            rend_speed = -rend_speed;
        }

        ourShader.setFloat("id0", id0);
        ourShader.setFloat("id1", id1);
        ourShader.setFloat("id2", id2);

        //glm::vec3 coordshift = glm::vec3(0.0f, 0.0f, 0.0f);
        //ourShader.setVec3("coordshift", coordshift);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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
