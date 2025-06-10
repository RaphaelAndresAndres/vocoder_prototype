#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "GLMethods.h"

#define WIDTH 1500
#define HEIGHT 900
#define FRAMES_PER_BUFFER 2048

GLuint shaderProgram;

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

GLuint loadShader(const char *filePath, GLenum shaderType)
{
    FILE *file = fopen(filePath, "r");
    if (!file)
    {
        fprintf(stderr, "Could not open shader file: %s\n", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *source = (char *)malloc(length + 1);
    fread(source, 1, length, file);
    source[length] = '\0';
    fclose(file);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation error: %s\n", infoLog);
    }

    free(source);
    return shader;
}

void draw(float time, float *source)
{
    const float limits = 0.92;
    const int numElements = 3;
    int numPoints = FRAMES_PER_BUFFER / 2 + 1;

    GLfloat waveVertices[numPoints * 2];
    GLfloat xAxisVertices[4] = {
        -limits - 0.03, -limits,
        limits, -limits};
    GLfloat yAxisVertices[4] = {
        -limits, limits,
        -limits, -limits - 0.03};
    GLfloat *
        vertices[numElements];
    int sizes[numElements];
    int nums[numElements];

    vertices[0] = waveVertices;
    sizes[0] = sizeof(waveVertices);
    nums[0] = numPoints;

    vertices[1] = xAxisVertices;
    sizes[1] = sizeof(xAxisVertices);
    nums[1] = 2;

    vertices[2] = yAxisVertices;
    sizes[2] = sizeof(yAxisVertices);
    nums[2] = 2;

    for (int i = 0; i < numPoints; ++i)
    {
        float t = log((float)i + 1) / logf(numPoints + 1);
        float x = t * 2 * limits - limits;
        waveVertices[2 * i] = x;
        waveVertices[2 * i + 1] = source[i] * 5.;
    }

    GLuint VBO[numElements], VAO[numElements];
    glGenVertexArrays(numElements, VAO);
    glGenBuffers(numElements, VBO);

    for (int i = 0; i < numElements; ++i)
    {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizes[i], vertices[i], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *)0);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINE_STRIP, 0, nums[i]);
        glDisableVertexAttribArray(0);
        glDeleteBuffers(1, &VBO[i]);
    }
}

void setupShaders()
{
    GLuint vertexShader = loadShader("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader("shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void *initGL(void *arg)
{

    float *staticOutputBuffer = (float *)arg;
    printf("GL has been called\n");
    glfwSetErrorCallback(error_callback);
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Animated Sine Wave", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();
    setupShaders();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);

        float time = (float)glfwGetTime();
        draw(time, staticOutputBuffer);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
}
