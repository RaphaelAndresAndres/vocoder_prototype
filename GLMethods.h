#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef GL_FUNCTIONS_H_INCLUDED
#define GL_FUNCTIONS_H_INCLUDED
void error_callback(int error, const char *description);
GLuint loadShader(const char *filePath, GLenum shaderType);
void setupShaders();
void draw(float *source);
void *initGL(void *arg);
#endif
