#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Shader/shader.h"
#include "stb/stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

float mixValue = 0.2f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGl Test", NULL, NULL);
    if(window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Shaders
    Shader ourShader("C:/Users/jackm/Documents/C++/OpenGL_test/src/shaders/3.3.shader.vs", "C:/Users/jackm/Documents/C++/OpenGL_test/src/shaders/3.3.shader.fs");

    //VERTEX INPUT - store verticies in a buffer for OpenGL to use
    //Single Square
    /*float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };*/
    //Cube Vertices
    float vertices[] = {
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right front
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right front
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left front
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left front

         0.5f,  0.5f, 1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right back
         0.5f, -0.5f, 1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right back
        -0.5f, -0.5f, 1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left back
        -0.5f,  0.5f, 1.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left back
    };

    unsigned int indices[] = {
        //front
        0, 1, 2,
        0, 2, 3,
        //back
        4, 5, 6,
        4, 6, 7,
        //top
        4, 0, 3,
        4, 3, 7,
        //bottom
        5, 1, 2,
        5, 2, 6,
        //right
        4, 5, 1,
        4, 1, 0,
        //left
        7, 6, 2,
        7, 2, 3
    };

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

    //VBO - Variable to represent vertices in GPU memory (Vertex buffer object)
    //EBO - Variable to represent indices in GPU memory (Element buffer object)
    //VAO - Vertex Array Object, stores vertex attribute contributions and VBOs linked to these attributes to ensure easy drawing
    unsigned int VBO, VAO, EBO;
    //Assigning id to array object
    glGenVertexArrays(1, &VAO);
    //Assigning id to vertex buffer
    glGenBuffers(1, &VBO);
    //Assigning id to element buffer
    glGenBuffers(1, &EBO);
    //Bind Vertex Array Object first, then bind and set vertex buffer(s) and then configure vertex attribute(s)
    glBindVertexArray(VAO);
    //Assigning VBO to represent and manage data stored on GPU via GL_ARRAY_BUFFER target
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //Adds vertex data and how to manage the data to the GPU, which is managed by the variable VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //Same process as VAO for EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //LINKING VERTEX ATTRIBUTES - Connects the vertex input data to the vertex shader attributes
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    //TEXTURE GENERATION - loads and binds image data to texture object
    stbi_set_flip_vertically_on_load(true);
    int width, width2, height, height2, nrChannels, nrChannels2;
    unsigned int texture;
    //Creation to texture id
    glGenTextures(1, &texture);
    //Binding of texture refereneced by id to the object GL_TEXTURE_2D
    glBindTexture(GL_TEXTURE_2D, texture);
    //Set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char *data = stbi_load("C:/Users/jackm/Documents/C++/OpenGL_test/assets/container.jpg", &width, &height, &nrChannels, 0);
    if(data){
        //Applies the actual texture image in data to the GL_TEXTURE_2D object, in turn, linking the texture data to the texture id
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        //Generates required mipmaps for the currently bound texture
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        std::cout << "Failed to load texture" << std::endl;
    }
    //Free the image memory
    stbi_image_free(data);

    //Second texture
    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    data = stbi_load("C:/Users/jackm/Documents/C++/OpenGL_test/assets/awesomeface.png", &width2, &height2, &nrChannels2, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        std::cout << "Failed to load texture2" << std::endl;
    }
    stbi_image_free(data);

    //Link shader samplers to texture units
    ourShader.use();
    glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
    ourShader.setInt("texture2", 1);

    
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    while(!glfwWindowShouldClose(window)){
        //Input
        processInput(window);

        //Rendering Commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Bind texture variable which contains image data to texture unit, in turn, linking texture data to shader samplers
        //Now, in the fragment shader, each texture coordinate is linked with the appropriate color
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.setFloat("opacity", mixValue);

        //transformations
        glm::mat4 model = glm::mat4(1.0f);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(ourShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        int viewLoc = glGetUniformLocation(ourShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //Draw Triangle
        glBindVertexArray(VAO);
        for(unsigned int i = 0; i < 10; i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            //if(i % 3 == 0){
            //    model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
            //}else{
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            //}
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        /*transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(-0.5f, 0.5f, 0.0f));
        transform = glm::scale(transform, glm::vec3(std::abs(sin((float)glfwGetTime())), std::abs(sin((float)glfwGetTime())), std::abs(sin((float)glfwGetTime()))));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/

        //Check and call events and swap buffers (Update pixels of screen)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        mixValue += 0.001f;
        if(mixValue >= 1.0f){
            mixValue = 1.0f;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        mixValue -= 0.001f;
        if(mixValue <= 0.0f){
            mixValue = 0.0f;
        }
    }
}