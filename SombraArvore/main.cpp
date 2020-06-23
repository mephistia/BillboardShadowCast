#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"

#include <iostream>

// funções
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderGround(const Shader& shader, GLuint array, GLuint texture, glm::vec3 treeArray);
void renderTree(const Shader& shader, GLuint array, GLuint texture);
void renderLight(const Shader& shader, GLuint array);



// janela
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 5.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// frames (input suave)
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// luz
glm::vec3 lightPos(-0.2f, 1.5f, -2.0f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sombra Projetada", NULL, NULL);
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

   
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // shaders
    Shader groundShader("chaoVertex.shader", "chaoFragment.shader");
    Shader treeShader("arvoreVertex.shader", "arvoreFragment.shader", "arvoreGeometry.shader");
    Shader lightShader("luzVertex.shader", "luzFragment.shader");


    // vértices do plano
    float vertices[] = {
         // pos               // normal             // textura
         2.0f, -0.8f,  2.0f,  0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
         2.0f, -0.8f, -2.0f,  0.0f,  0.0f, -1.0f,   1.0f,  0.0f,
        -2.0f, -0.8f, -2.0f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f,
        -2.0f, -0.8f,  2.0f,  0.0f,  0.0f, -1.0f,   1.0f,  1.0f,
         2.0f, -0.8f,  2.0f,  0.0f,  0.0f, -1.0f,   0.0f,  1.0f,
        -2.0f, -0.8f, -2.0f,  0.0f,  0.0f, -1.0f,   0.0f,  0.0f
    };

 
    // centro da arvore
    float treeSpawn[] = {
        -0.2f, -0.65f, 0.5f
    };
    glm::vec3 treeVec(treeSpawn[0], treeSpawn[1], treeSpawn[2]);

    // chao
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    // textura
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // luz
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    // arvore
    unsigned int treeVAO, treeVBO;
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glBindVertexArray(treeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeSpawn), &treeSpawn, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    unsigned int groundTex = loadTexture("assets/ground.jpg");
    unsigned int treeTex = loadTexture("assets/tree.png");

    groundShader.use();
    groundShader.setInt("material.diffuse", 0);
    groundShader.setInt("tree.tex", 1);

    treeShader.use();
    treeShader.setInt("tex", 1);
     
    // loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
 
        // input
        processInput(window);

        // limpa buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render
        treeShader.use();
        renderTree(treeShader, treeVAO, treeTex);

        groundShader.use();
        renderGround(groundShader, VAO, groundTex, treeVec);

        lightShader.use();
        renderLight(lightShader, lightVAO);

        // troca buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &treeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &treeVBO);

    glfwTerminate();
    return 0;
}

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

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos -= glm::vec3(0.0, 0.0, 1.0 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos += glm::vec3(0.0, 0.0, 1.0 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos -= glm::vec3(1.0 * deltaTime, 0.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos += glm::vec3(1.0 * deltaTime, 0.0, 0.0);
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        lightPos += glm::vec3(0.0, 1.5 * deltaTime, 0.0);
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        lightPos -= glm::vec3(0.0, 1.5 * deltaTime, 0.0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void renderGround(const Shader& shader, GLuint array, GLuint texture, glm::vec3 treeArray) {
    shader.setVec3("light.position", lightPos);
    shader.setVec3("viewPos", camera.Position);


    // luz
    shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    // material
    shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
    shader.setFloat("material.shininess", 64.0f);

    // arvore
    shader.setVec3("tree.position", treeArray);
    float ratio = SCR_WIDTH / SCR_HEIGHT;
    glm::vec2 size(1, ratio * 1);
    shader.setVec2("tree.size", size);



    // matrizes
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    // renderiza
    glBindVertexArray(array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderLight(const Shader& shader, GLuint array) {
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    // dados
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // menor
    shader.setMat4("model", model);

    // render
    glBindVertexArray(array);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderTree(const Shader& shader, GLuint array, GLuint texture) {
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::mat4 viewProj = projection * view;

    // dados
    float ratio = SCR_WIDTH / SCR_HEIGHT;
    glm::vec2 size(1, ratio * 1);
    shader.setMat4("viewProj", viewProj);
    shader.setVec2("size", size);


    // render
    glBindVertexArray(array);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_POINTS, 0, 1);
}

