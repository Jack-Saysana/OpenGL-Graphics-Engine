#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader/shader.h"
#include "Camera/camera.h"
#include "Model/model.h"

//Handles any updates to window size
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);

Camera camera;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX, lastY;
bool firstMouse = true;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1920 / 1.5, 1080 / 1.5, "Shadow Test", NULL, NULL);
    if(window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 1920 / 1.5, 1080 / 1.5);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glEnable(GL_DEPTH_TEST);

    //Shaders
    Shader shader("C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.shader.vs", "C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.shader.fs");
    Shader dShader("C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.depth.shader.vs", "C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.depth.shader.fs");
    Shader omniShader("C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.omni.depth.shader.vs", "C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.omni.depth.shader.gs", "C:/Users/jackm/Documents/General/OpenGL/src/shaders/4.6.omni.depth.shader.fs");

    //Models
    Model cube("C:/Users/jackm/Documents/General/OpenGL/resources/Cube/cube.obj");
    Model floor("C:/Users/jackm/Documents/General/OpenGL/resources/Floor/Floor.obj");

    /*======DIRECTIONAL SHADOW MAPPING======*/
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;
    //Creating a depth map
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    //Since we're only using depth values, the formats are GL_DEPTH_COMPONENT
    //1024 x 1024 is the resolution of the depth map
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //Creation of a framebuffer for loading depth info into the depth map
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //We only need to use the depth attachment of the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    //Explicitly defining we wont be using any color rendering in this framebuffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /*=====================================*/

    /*========OMNIDIRECTIONAL SHADOW MAPPING========*/
    unsigned int omniDepthMapFBO;
    glGenFramebuffers(1, &omniDepthMapFBO);

    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);

    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, omniDepthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    /*==============================================*/

    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Input
        processInput(window);

        /*====RENDER TO DEPTH MAP (DIRECTIONAL)====*/
        //Viewport for depthMapFBO accomodates the depthMap texture resolution
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

        //Rendering Commands
        glClear(GL_DEPTH_BUFFER_BIT);

        //Transformations and Drawing
        //Use a much simpler and cheaper shader for rendering to depth buffer
        dShader.use();
        
        //Because light rays are parallel, orthographic projection is used when rendering
        //to the depthMap
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        dShader.setMat4("projection", projection);

        //Change the view matrix to look at the scene from the light's perspective
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(glm::vec3(3.0f, 3.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        dShader.setMat4("view", view);

        //Matrix that represents the scene viewed from the perspective of the light
        glm::mat4 lightSpaceMatrix = projection * view;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.501f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        dShader.setMat4("model", model);

        cube.Draw(dShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, -1.0f));
        model = glm::scale(model, glm::vec3(0.25f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(-0.3, 0.4, 0.6));
        dShader.setMat4("model", model);

        cube.Draw(dShader);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));
        dShader.setMat4("model", model);

        floor.Draw(dShader);
        /*===========================*/

        /*=======RENDER TO DEPTH MAP (OMNI-DIRECTIONAL)======*/
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, omniDepthMapFBO);

        //Rendering Commands
        glClear(GL_DEPTH_BUFFER_BIT);

        //Transformations and Drawing
        omniShader.use();
        projection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        omniShader.setMat4("projection", projection);

        glm::vec3 lightPos = glm::vec3(-3.0f, 1.0f, 0.0f);
        glm::mat4 lightSpaceMatrices[] = {
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
            projection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))
        };
        for (unsigned int i = 0; i < 6; i++) 
        {
            std::string uniform = "shadowMatrices[";
            uniform += (i + 48);
            uniform += "]";
            omniShader.setMat4(uniform, lightSpaceMatrices[i]);
        }
        omniShader.setVec3("lightPos", lightPos);
        omniShader.setFloat("far_plane", far_plane);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.501f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        omniShader.setMat4("model", model);

        cube.Draw(omniShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, -1.0f));
        model = glm::scale(model, glm::vec3(0.25f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(-0.3, 0.4, 0.6));
        omniShader.setMat4("model", model);

        cube.Draw(omniShader);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));
        omniShader.setMat4("model", model);

        floor.Draw(omniShader);
        /*===================================================*/

        /*=======RENDER SCENE AS NORMAL W/SHADOW MAPPING=======*/
        glViewport(0, 0, 1920 / 1.5, 1080 / 1.5);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //Rendering Commands
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Transformations and Drawing
        shader.use();
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        
        //Pass the lightSpaceMatrix into the scene's vertex shader in order to determine
        //which fragments will be a shadow
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setInt("depthMap", 4);

        projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.01f, 100.0f);
        shader.setMat4("projection", projection);

        view = glm::mat4(1.0f);
        view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.pos);
        shader.setFloat("far_plane", far_plane);

        model = glm::mat4(1.0f);

        /*=====Directional light=====*/
        glm::vec4 dirLightCol = glm::vec4(1.0f);
        shader.setVec3("dirLight.direction", glm::vec3(-1.0f, -1.0f, 0.0f));
        shader.setVec4("dirLight.color", dirLightCol);
        shader.setVec4("dirLight.ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        shader.setVec4("dirLight.diffuse", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        shader.setVec4("dirLight.specular", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        /*==========================*/

        /*=======Point lights=======*/
        // glm::vec4 pointLightCol = glm::vec4(1.0f);
        // shader.setInt("numPointLights", 1);
        // shader.setVec3("pointLights[0].position", lightPos);
        // shader.setVec4("pointLights[0].color", pointLightCol);
        // shader.setVec4("pointLights[0].ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        // shader.setVec4("pointLights[0].diffuse", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        // shader.setVec4("pointLights[0].specular", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        // shader.setFloat("pointLights[0].constant", 1.0f);
        // shader.setFloat("pointLights[0].linear", 0.09f);
        // shader.setFloat("pointLights[0].quadratic", 0.032f);
        /*==========================*/

        model = glm::translate(model, glm::vec3(0.0f, 0.501f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);

        cube.Draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, -1.0f));
        model = glm::scale(model, glm::vec3(0.25f));
        model = glm::rotate(model, glm::radians(40.0f), glm::vec3(-0.3, 0.4, 0.6));
        shader.setMat4("model", model);

        cube.Draw(shader);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));
        shader.setMat4("model", model);

        floor.Draw(shader);
        /*=====================================================*/

        //Check events and update screen buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera.buttonInput(SPRINT, deltaTime);
    }
    else
    {
        camera.buttonInput(WALK, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.buttonInput(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.buttonInput(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.buttonInput(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.buttonInput(RIGHT, deltaTime);
    }
}

void mouseCallback(GLFWwindow *, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.mouseInput(xoffset, yoffset);
}