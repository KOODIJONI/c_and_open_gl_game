#ifndef GL_SETUP_H
#define GL_SETUP_H
#include <GL/gl.h>
#include <stddef.h>
#include "gl_loader.h"

GLuint GLSetup_CreateVAO(const float* data, int vertexCount, int componentsPerVertex);
GLuint GLSetup_CreateDynamicVAO(GLuint* outVBO);

#endif // GL_SETUP_H