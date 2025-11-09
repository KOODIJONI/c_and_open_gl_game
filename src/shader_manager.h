#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H
#include <GL/gl.h>
#include <stddef.h>
#include <stdbool.h>
#include "gl_loader.h"

GLint ShaderManager_CreateProgram(const char* vertexPath, const char* fragmentPath);
GLint ShaderManager_GetUniformLocation(GLuint program, const char* name);
#endif
