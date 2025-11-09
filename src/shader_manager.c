// shader_manager.c

#include "shader_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

// Utility to load a file into memory
static char* LoadShaderSource(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open shader file: %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "ERROR: Failed to allocate memory for shader file: %s\n", filepath);
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

static GLuint CompileShader(GLenum type, const char* source, const char* shaderName) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "ERROR: Shader compilation failed (%s)\n%s\n", shaderName, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint ShaderManager_CreateProgram(const char* vertexPath, const char* fragmentPath) {
    char* vertexSrc = LoadShaderSource(vertexPath);
    char* fragmentSrc = LoadShaderSource(fragmentPath);

    if (!vertexSrc || !fragmentSrc) {
        free(vertexSrc);
        free(fragmentSrc);
        return 0;
    }

    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc, vertexPath);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc, fragmentPath);
    free(vertexSrc);
    free(fragmentSrc);

    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linkSuccess;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "ERROR: Shader program linking failed\n%s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}
GLint ShaderManager_GetUniformLocation(GLuint program, const char* name) {
    GLint location = glGetUniformLocation(program, name);
    if (location == -1) {
        fprintf(stderr, "Warning: Could not find uniform '%s' in program %u\n", name, program);
    }
    return location;
}
