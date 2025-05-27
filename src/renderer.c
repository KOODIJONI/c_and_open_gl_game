// renderer.c
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "renderer.h"
#include "gl_loader.h"
#include <math.h>
#include "projection.h"
#include "matrix_utils.h"
#include "shader.h"
#include "camera_control.h"
#include "OBJ_file_loader.h"
#include <stdbool.h>
static ObjMesh mesh = {0}; 

static GLuint shaderProgram = 0;
static GLuint vao = 0;
static GLuint vbo = 0;
static GLint uniformProjectionLoc = -1;
static GLint uniformModelLoc = -1;
static GLint uniformViewLoc = -1;

GLuint dynamicVbo = 0;
GLuint dynamicVao = 0;
float directionalLight [3] = {0.3f, -0.2f, 1.0f}; 
float lightIntensity = 0.2f;       // scalar light intensity (0..1)
float lightColor[3] = {1.0f, 0.01f, 1.0f};


float vertices[] = {
    -0.5f, -0.5f,  0.5f,  // bottom-left
     0.5f, -0.5f,  0.5f,  // bottom-right
     0.5f,  0.5f,  0.5f,  // top-right

     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f,  

    -0.5f, -0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f,  
     0.5f,  0.5f, -0.5f,  

     0.5f,  0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f,  // bottom-left

    -0.5f, -0.5f,  0.5f,  // bottom-front
    -0.5f,  0.5f,  0.5f,  // top-front
    -0.5f,  0.5f, -0.5f,  // top-back

    -0.5f,  0.5f, -0.5f,  // top-back
    -0.5f, -0.5f, -0.5f,  // bottom-back
    -0.5f, -0.5f,  0.5f,  // bottom-front

     0.5f, -0.5f,  0.5f,  
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f, 

     0.5f,  0.5f, -0.5f, 
     0.5f,  0.5f,  0.5f,  
     0.5f, -0.5f,  0.5f,  

    -0.5f,  0.5f,  0.5f, 
     0.5f,  0.5f,  0.5f,  
     0.5f,  0.5f, -0.5f,  

     0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f,  0.5f,  

    -0.5f, -0.5f,  0.5f,  
    -0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f, -0.5f,  

     0.5f, -0.5f, -0.5f, 
     0.5f, -0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f 

};
float* vertices_ptr = vertices; 

     
float projectionMatrix[16];   
float viewMatrix[16];   

static int verticesCount = sizeof(vertices) / sizeof(vertices[0]);


void Renderer_Init(void) {
    bool error = LoadOBJ("assets/largeTerrain.obj", &mesh);

    printf("original");
    for (int i = 0; i < verticesCount/3; i++) {
        int baseIndex = i * 3;
        printf("Vertex %d: (%f, %f, %f)\n", i, vertices_ptr[baseIndex], vertices_ptr[baseIndex + 1], vertices_ptr[baseIndex + 2]);
    }
    if (!error) {
        printf("setting pointer\n");
        vertices_ptr = mesh.triangle_vertices;
        verticesCount = mesh.triangle_vertex_count;  
        printf("OBJ file loaded successfully: %s\n");
        for (int i = 0; i < verticesCount/3; i++) {
            int baseIndex = i * 3;
            printf("Vertex %d: (%f, %f, %f)\n", i, vertices_ptr[baseIndex], vertices_ptr[baseIndex + 1], vertices_ptr[baseIndex + 2]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(float), vertices_ptr, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else {
        printf("Failed to load OBJ file: %s\n", "assets/terrain.obj");
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    }
        
    CameraControl_Init();

    glGenVertexArrays(1, &dynamicVao);
    glBindVertexArray(dynamicVao);

    glGenBuffers(1, &dynamicVbo);
    glBindBuffer(GL_ARRAY_BUFFER, dynamicVbo);

    // Setup vertex attribute pointer ONCE here for dynamic VAO
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    
    printf("OpenGL functions loaded successfully\n");
    MessageBoxA(
        0, "OpenGL functions loaded successfully", "Info", MB_ICONINFORMATION);

    char* vertexShaderSrc = LoadShaderSource("shaders/vertex_shader.glsl");
    if (!vertexShaderSrc) {
        printf("ERROR::Failed to load vertex shader source\n");
        MessageBoxA(0, "log", "ERROR::Failed to load vertex shader source\n", MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    char* fragmentShaderSrc = LoadShaderSource("shaders/fragment_shader.glsl");
    if (!fragmentShaderSrc) {
        printf("ERROR::Failed to load fragment shader source\n");
        MessageBoxA(0, "log", "ERROR::Failed to load fragment shader source\n", MB_ICONERROR);
        free(vertexShaderSrc);
        exit(EXIT_FAILURE);
    }

    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    if (vs == 0) {
        printf("ERROR::Failed to create vertex shader\n");
        MessageBoxA(0, "log", "ERROR::Failed to create vertex shader\n", MB_ICONERROR);

        exit(EXIT_FAILURE);
    }
    printf("Vertex shader created successfully\n");
    glShaderSource(vs, 1, (const GLchar * const*)&vertexShaderSrc, NULL);
    glCompileShader(vs);

    GLint compiled;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(vs, 512, NULL, infoLog);
        printf("ERROR::VERTEX_SHADER_COMPILATION_FAILED\n%s\n", infoLog);
        MessageBoxA(0, "log", "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n%s\n", MB_ICONERROR);

        glDeleteShader(vs);
        exit(EXIT_FAILURE);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (fs == 0) {
        printf("ERROR::Failed to create fragment shader\n");
        MessageBoxA(0, "log", "ERROR::fragment_SHADER_COMPILATION_FAILED\n%s\n", MB_ICONERROR);

        glDeleteShader(vs);
        exit(EXIT_FAILURE);
    }
    printf("Fragment shader created successfully\n");
    glShaderSource(fs, 1, (const GLchar * const*)&fragmentShaderSrc, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(fs, 512, NULL, infoLog);
        printf( "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n%s\n", infoLog);
        MessageBoxA(0, "log", "ERROR::fragment_SHADER_COMPILATION_FAILED\n%s\n", MB_ICONERROR);

        glDeleteShader(vs);
        glDeleteShader(fs);
        exit(EXIT_FAILURE);
    }

    shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        printf( "ERROR::Failed to create shader program\n");
        MessageBoxA(0, "log", "ERROR::Failed to create shader program\n", MB_ICONERROR);

        glDeleteShader(vs);
        glDeleteShader(fs);
        exit(EXIT_FAILURE);
    }
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    printf("Shader program linked successfully\n");

    uniformProjectionLoc = glGetUniformLocation(shaderProgram, "uProjection");
    if (uniformProjectionLoc == -1) {
        printf("Warning: Could not find uniform 'uProjection'\n");
    }

    uniformModelLoc = glGetUniformLocation(shaderProgram, "uModel");
    if (uniformModelLoc == -1) {
        printf("Warning: Could not find uniform 'uModel'\n");
    }
    uniformViewLoc = glGetUniformLocation(shaderProgram, "uView");
    if ( uniformViewLoc== -1){
        printf("Warning: Could not find uniform 'uView'\n");

    }

    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf( "ERROR::SHADER_PROGRAM_LINKING_FAILED\n\n");
        MessageBoxA(0, "log", "ERROR::SHADER_PROGRAM_LINKING_FAILED\n\n", MB_ICONERROR);
        printf("ERROR::SHADER_PROGRAM_LINKING_FAILED\n\n");

        glDeleteProgram(shaderProgram);
        glDeleteShader(vs);
        glDeleteShader(fs);
        exit(EXIT_FAILURE);
    }
    printf("Shader program linked successfully2\n");
    
    
    glGenVertexArrays(1, &vao);
    if (vao == 0) {
        printf( "ERROR::Failed to generate VAO\n");
        MessageBoxA(0, "log", "ERROR::Failed to generate VAO\n", MB_ICONERROR);
        glDeleteProgram(shaderProgram);
        exit(EXIT_FAILURE);
    }
    printf("VAO generated successfully\n");
    glGenBuffers(1, &vbo);
    if (vbo == 0) {
        printf( "ERROR::Failed to generate VBO\n");
        MessageBoxA(0, "log", "ERROR::Failed to generate VBO\n", MB_ICONERROR);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(shaderProgram);
        exit(EXIT_FAILURE);
    }
    printf("VBO generated successfully\n");
    MessageBoxA(0, "log", "VBO generated successfully\n", MB_ICONINFORMATION);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("OpenGL version: %s\n", version);

    glBindVertexArray(vao);

    
    

    glBindVertexArray(0);
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        printf("OpenGL Error: %x\n", err);
    }
    float fovY = 45.0f * (3.1415926f / 180.0f); // radians
    float aspect = 800.0f / 600.0f;
    float nearr = 0.1f;
    float farr = 10000.0f;
    
    CreatePerspectiveProjection(fovY, aspect, nearr, farr, projectionMatrix);

    glUseProgram(shaderProgram);

    if (uniformProjectionLoc != -1) {
        glUniformMatrix4fv(uniformProjectionLoc, 1, GL_FALSE, projectionMatrix);
        printf("Projection matrix set successfully\n");
    }
}
void TransformVertex(const float m[16], const float in[3], float out[3]) {
    out[0] = m[0]*in[0] + m[4]*in[1] + m[8]*in[2] + m[12];
    out[1] = m[1]*in[0] + m[5]*in[1] + m[9]*in[2] + m[13];
    out[2] = m[2]*in[0] + m[6]*in[1] + m[10]*in[2] + m[14];
}
float angle = 0.0f;
static float elapsedTime = 0.0f;

void Renderer_Draw(float deltaTime) {
   
    angle += deltaTime;
    
    float rotation[16];
    float translation[16];
    elapsedTime += deltaTime; 
    CreateRotationMatrix(0, 0, 0, rotation);
    
    
    CreateTranslationMatrix(0.0f, 0.0f,  -10, translation);

    CameraControl_Update(deltaTime, viewMatrix);

    MultiplyMatrices(rotation, translation, rotation);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth test if not enabled elsewhere
    glEnable(GL_DEPTH_TEST);

    float cameraPos[3] = {0.0f, 0.0f, 0.0f};

    static float visibleData[1000000]; 
    int visibleCount = 0;
    for (int i = 0; i < verticesCount; i += 9) {
        float A_obj[3] = {vertices_ptr[i], vertices_ptr[i+1], vertices_ptr[i+2]};
        float B_obj[3] = {vertices_ptr[i+3], vertices_ptr[i+4], vertices_ptr[i+5]};
        float C_obj[3] = {vertices_ptr[i+6], vertices_ptr[i+7], vertices_ptr[i+8]};

        float A[3], B[3], C[3];
        TransformVertex(rotation, A_obj, A);
        TransformVertex(rotation, B_obj, B);
        TransformVertex(rotation, C_obj, C);

        // Compute edges
        float AB[3] = {B[0]-A[0], B[1]-A[1], B[2]-A[2]};
        float AC[3] = {C[0]-A[0], C[1]-A[1], C[2]-A[2]};

        // Compute normal = cross(AB, AC)
        float Nx = AB[1]*AC[2] - AB[2]*AC[1];
        float Ny = AB[2]*AC[0] - AB[0]*AC[2];
        float Nz = AB[0]*AC[1] - AB[1]*AC[0];

        // Normalize normal
        float len = sqrtf(Nx*Nx + Ny*Ny + Nz*Nz);
        if (len == 0) continue;
        Nx /= len; Ny /= len; Nz /= len;

        // Compute view vector from camera to A (camera is at origin)
        CameraControl_GetPosition(&cameraPos[0],&cameraPos[1],&cameraPos[2]);
        float Vx = A[0] - cameraPos[0];
        float Vy = A[1] - cameraPos[1];
        float Vz = A[2] - cameraPos[2];

        float vlen = sqrtf(Vx*Vx + Vy*Vy + Vz*Vz);
        if (vlen == 0) continue;
        Vx /= vlen; Vy /= vlen; Vz /= vlen;

        // Dot product normal · view vector
        float dot = Nx*Vx + Ny*Vy + Nz*Vz;
        if (dot) {
            
            float len = sqrtf(directionalLight[0]*directionalLight[0] + directionalLight[1]*directionalLight[1] + directionalLight[2]*directionalLight[2]);
            directionalLight[0] /= len; directionalLight[1] /= len; directionalLight[2] /= len;

            float diffuseIntensity = Nx*directionalLight[0] + Ny*directionalLight[1] + Nz*directionalLight[2];
            if (diffuseIntensity < 0) diffuseIntensity = 0;

            // Example: simple grayscale color scaled by diffuse
            float r = diffuseIntensity*lightColor[0]; 
            float g = diffuseIntensity*lightColor[1];
            float b = diffuseIntensity*lightColor[2];

            // Print or store color per face
            
            // Face is front-facing
            // Push original *object-space* vertices (OpenGL will transform by model matrix uniform)
           
            for (int j = 0; j < 3; ++j) {
            int idx = i + j * 3;
            visibleData[visibleCount++] = vertices_ptr[idx];
            visibleData[visibleCount++] = vertices_ptr[idx + 1];
            visibleData[visibleCount++] = vertices_ptr[idx + 2];
            visibleData[visibleCount++] = r;
            visibleData[visibleCount++] = g;
            visibleData[visibleCount++] = b;
            }
        }
    }

    int visibleVertexCount = visibleCount / 6; // each vertex = 6 floats (pos + color)

    glBindBuffer(GL_ARRAY_BUFFER, dynamicVbo);
    glBufferData(GL_ARRAY_BUFFER, visibleCount * sizeof(float), visibleData, GL_DYNAMIC_DRAW);

    glUseProgram(shaderProgram);
    if (uniformModelLoc != -1) {
        glUniformMatrix4fv(uniformModelLoc, 1, GL_FALSE, rotation);
    }
    if (uniformProjectionLoc != -1) {
        glUniformMatrix4fv(uniformProjectionLoc, 1, GL_FALSE, projectionMatrix);
    }
    if (uniformViewLoc != -1) {
        glUniformMatrix4fv(uniformViewLoc, 1, GL_FALSE, viewMatrix);
    }

    // Setup VAO attributes for position (0) and color (1):
    glBindVertexArray(dynamicVao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, visibleVertexCount);
}


float Get_Aspect_Ratio(void) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];
    if (height == 0) return 1.0f;
    return (float)width / (float)height;
}


void Renderer_Cleanup(void) {
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}
void UpdateProjectionMatrix(float aspect) {
    float fovY = 45.0f * (3.1415926f / 180.0f); // radians
    float nearr = 0.1f;
    float farr = 10000.0f;
    CreatePerspectiveProjection(fovY, aspect, nearr, farr, projectionMatrix);
    
    if (uniformProjectionLoc != -1) {
        glUniformMatrix4fv(uniformProjectionLoc, 1, GL_FALSE, projectionMatrix);
    }
}
void set_vertices(float* newVertices, int count) {
    if (newVertices == NULL || count <= 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), newVertices, GL_STATIC_DRAW);
    
    verticesCount = count / 3; // assuming each vertex has 3 components (x, y, z)
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}