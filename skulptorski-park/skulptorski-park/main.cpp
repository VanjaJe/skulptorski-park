
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);
void writeName(unsigned int nameVAO, unsigned int nameVBO, unsigned int nameTexture, unsigned int nameShaderProgram, glm::mat4 MVP);
void key_space_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

float cameraX = 0.0f, cameraZ = 5.0f, angle = 0.0f;
bool perspective = true;
bool rotateFigures = true;
const unsigned int SCR_W = 500, SCR_H = 500;

int main(void)
{
    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1300;
    unsigned int wHeight = 850;
    const char wTitle[] = "Skulptorski park";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ PROMJENLJIVE I BAFERI +++++++++++++++++++++++++++++++++++++++++++++++++

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int nameShader = createShader("name.vert", "name.frag");
    glUseProgram(unifiedShader);

    float nameVertices[] = {
        // x, y,     tex coords
        0.0f,  0.0f,   0.0f, 0.0f,
        200.0f, 0.0f,   1.0f, 0.0f,
        200.0f, 50.0f,  1.0f, 1.0f,

        0.0f,  0.0f,  0.0f, 0.0f,
        200.0f, 50.0f,  1.0f, 1.0f,
        0.0f,  50.0f, 0.0f, 1.0f
    };

    unsigned int nameVAO, nameVBO;
    glGenVertexArrays(1, &nameVAO);
    glGenBuffers(1, &nameVBO);

    glBindVertexArray(nameVAO);
    glBindBuffer(GL_ARRAY_BUFFER, nameVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nameVertices), nameVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int nameTexture = loadImageToTexture("images/name.jpg");

    glBindTexture(GL_TEXTURE_2D, nameTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    float vertices[] = {
        // Cube vertices
        -0.5f,-0.5f,-0.5f,   1,0,0,
         0.5f,-0.5f,-0.5f,   0,1,0,
         0.5f, 0.5f,-0.5f,   0,0,1,
        -0.5f, 0.5f,-0.5f,   1,1,0,
        -0.5f,-0.5f, 0.5f,   1,0,1,
         0.5f,-0.5f, 0.5f,   0,1,1,
         0.5f, 0.5f, 0.5f,   1,1,1,
        -0.5f, 0.5f, 0.5f,   .5,.5,.5,

        // pyramid
         0.0f, 0.8f, 0.0f,   1,1,1
    };

    // Cube indices (12 triangles = 36 indices)
    unsigned int cubeIndices[] = {
        0,1,2,  2,3,0, // Front
        4,5,6,  6,7,4, // Back
        0,1,5,  5,4,0, // Bottom
        2,3,7,  7,6,2, // Top
        0,3,7,  7,4,0, // Left
        1,2,6,  6,5,1  // Right
    };

    // Pyramid indices (4 triangular sides + 2 for base = 6 triangles = 18 indices)
    // Base uses vertices 4,5,6,7 (the back face of the cube from 'vertices' array,
    // which effectively acts as the bottom of the pyramid on top of the cube)
    // Apex is vertex 8
    unsigned int pyramidIndices[] = {
        // Sides
        8,5,4, // Note: Order matters for culling. Assuming counter-clockwise from outside.
        8,6,5,
        8,7,6,
        8,4,7,
        // Base (same as back face of cube, or top of the lower part of the cube)
        4,5,6,
        6,7,4
    };

    float groundVertices[] = {
        // positions           // colors
        -10.0f, -0.5f, -10.0f,  0.0f, 0.8f, 0.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 0.8f, 0.0f,
         10.0f, -0.5f,  10.0f,  0.0f, 0.8f, 0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 0.8f, 0.0f
    };
    unsigned int groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };


    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    unsigned int VBO, VAO, EBOc, EBOp;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBOc);
    glGenBuffers(1, &EBOp);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Load cube indices into EBOc once
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOc);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // Load pyramid indices into EBOp once
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOp);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    // Vertex attribs for VAO (applies to both cube and pyramid as they share VBO)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Uniform locations
    int uM = glGetUniformLocation(unifiedShader, "uM");
    int uV = glGetUniformLocation(unifiedShader, "uV");
    int uP = glGetUniformLocation(unifiedShader, "uP");

    // Model
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(uM, 1, GL_FALSE, glm::value_ptr(model)); //(Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)

    // Projections
    glm::mat4 projP = glm::perspective(glm::radians(90.0f), (float)wWidth / wHeight, 0.1f, 100.0f);
    glm::mat4 projO = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -10.0f, 100.0f);
    glUniformMatrix4fv(uP, 1, GL_FALSE, glm::value_ptr(projP));

    glClearColor(0.0, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f;
    float rotSpeed = 0.5f;
    const float cameraSpeed = 0.05f;

    const double target_time = 1.0 / 60.0;
    double last_frame_time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        double current_frame_time = glfwGetTime();
        double delta_time = current_frame_time - last_frame_time;

        if (delta_time >= target_time)
        {

            glUseProgram(unifiedShader);

            glm::vec3 flatForward = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));      //kod orth nema perspektive pa je kretanje napred nazad drugacije, ide po XZ osama

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
                cameraPos += cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
                cameraPos -= cameraSpeed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                yaw += rotSpeed;

                cameraFront.x = cos(glm::radians(yaw));
                cameraFront.z = sin(glm::radians(yaw));
            }
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                yaw -= rotSpeed;

                cameraFront.x = cos(glm::radians(yaw));
                cameraFront.z = sin(glm::radians(yaw));
            }

            glm::vec3 camPosFront;
            if (perspective) 
                camPosFront = glm::vec3(cameraFront.x + cameraPos.x, cameraFront.y + cameraPos.y, cameraPos.z + cameraFront.z);
            else
                camPosFront = glm::vec3(cameraFront.x + cameraPos.x, cameraFront.y + cameraPos.y, cameraPos.z + cameraFront.z);

            glm::mat4 view = glm::lookAt(cameraPos, camPosFront, cameraUp);
            glUniformMatrix4fv(uV, 1, GL_FALSE, glm::value_ptr(view));

            glfwSetKeyCallback(window, key_space_callback);
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { perspective = true; glUniformMatrix4fv(uP, 1, GL_FALSE, glm::value_ptr(projP)); }
            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
                perspective = false; glUniformMatrix4fv(uP, 1, GL_FALSE, glm::value_ptr(projO)); 
            }

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 groundModel = glm::mat4(1.0f);
            glUniformMatrix4fv(uM, 1, GL_FALSE, glm::value_ptr(groundModel));

            glBindVertexArray(groundVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            float cubeHeight = 1.0f;     
            float pyramidHeight = 1.0f;  
            glBindVertexArray(VAO);

            for (int i = 0; i < 4; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i * 1.5f - 2.0f, 0.0f, 0.0f));

                static float angle = 0.0f;
                if (rotateFigures)
                    angle += 0.005f;

                model = glm::rotate(model, angle + i, glm::vec3(0.0f, 1.0f, 0.0f));

                // Crtanje kocke
                glUniformMatrix4fv(uM, 1, GL_FALSE, glm::value_ptr(model));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOc);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                // Crtanje piramide na vrhu kocke
                glm::mat4 modelPyramid = glm::translate(model, glm::vec3(0.0f, cubeHeight, 0.0f));
                glUniformMatrix4fv(uM, 1, GL_FALSE, glm::value_ptr(modelPyramid));
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOp); // Bind pyramid EBO
                glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
            }

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, wHeight - 50.0f, 0.0f));
            glm::mat4 ortho = glm::ortho(0.0f, (float)wWidth, 0.0f, (float)wHeight);

            glm::mat4 MVP = ortho * model;


            glDisable(GL_DEPTH_TEST);
            writeName(nameVAO, nameVBO, nameTexture, nameShader, MVP);
            glEnable(GL_DEPTH_TEST);
            last_frame_time = current_frame_time;

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}

void key_space_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        rotateFigures = !rotateFigures;
}


void writeName(unsigned int nameVAO, unsigned int nameVBO, unsigned int nameTexture, unsigned int nameShaderProgram, glm::mat4 MVP) {

    glUseProgram(nameShaderProgram);
    unsigned uTexLoc = glGetUniformLocation(nameShaderProgram, "uTex");
    glUniform1i(uTexLoc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, nameTexture);

    GLint mvpLoc = glGetUniformLocation(nameShaderProgram, "MVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(nameVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
