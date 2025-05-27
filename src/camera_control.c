#include "camera_control.h"
#include "matrix_utils.h"
#include "user_input.h"
#include <stdio.h>
#include <math.h>
#ifndef M_PI_2
#define M_PI_2 1.5707963267948966
#endif
extern UserInput g_input; 

static float cameraPosition[3] = {0.0f, 0.0f, 0.0f};
static float cameraRotation[3] = {0.0f, 0.0f, 0.0f};
static float rotation[16] = {0.0f}; 
static float translation[16] = {0.0f};
static float speed = 20.0f;
static int mouseX = 0, mouseY = 0;
void CameraControl_Init() {
    UserInput_Init(&g_input);
    cameraPosition[0] = 0.0f;
    cameraPosition[1] = 0.0f;
    cameraPosition[2] = 5.0f; 
    cameraRotation[0] = 0.0f;
    cameraRotation[1] = 0.0f; 
    cameraRotation[2] = 0.0f; 
}

void CameraControl_Update(float deltaTime, float* viewMatrix) {
    
    // WASD movement
    if (g_input.keys['A']) {
    float rotMat[16];
    CreateRotationMatrix(cameraRotation[0], cameraRotation[1], cameraRotation[2], rotMat);

    // Right vector = X axis (1st column of the matrix)
    float right[3] = {
        rotMat[0],
        rotMat[1],
        rotMat[2]
    };

    // Move left = negative of right vector
    cameraPosition[0] -= right[0] * speed * deltaTime;
    cameraPosition[2] -= right[2] * speed * deltaTime;

    printf("A key (Left) pressed (camera-relative)\n");
    }
    else if (g_input.keys['D']) {
        float rotMat[16];
        CreateRotationMatrix(cameraRotation[0], cameraRotation[1], cameraRotation[2], rotMat);

        // Right vector = X axis (1st column of the matrix)
        float right[3] = {
            rotMat[0],
            rotMat[1],
            rotMat[2]
        };

        // Move right = along right vector
        cameraPosition[0] += right[0] * speed * deltaTime;
        cameraPosition[2] += right[2] * speed * deltaTime;

        printf("D key (Right) pressed (camera-relative)\n");
    }

    else if (g_input.keys['W']) {
        float rotMat[16];
        CreateRotationMatrix(cameraRotation[0], cameraRotation[1], cameraRotation[2], rotMat); // POSITIVE angles

        // Forward = -Z axis (3rd column)
        float forward[3] = {
            -rotMat[8],
            -rotMat[9],
            -rotMat[10]
        };

        cameraPosition[0] += forward[0] * speed * deltaTime;
        cameraPosition[2] += forward[2] * speed * deltaTime;

        printf("W key (Forward) pressed (camera-relative)\n");
    }
    else if (g_input.keys['S']){
        float rotMat[16];
        CreateRotationMatrix(cameraRotation[0], cameraRotation[1], cameraRotation[2], rotMat);

        float forward[3] = {
            -rotMat[8],
            -rotMat[9],
            -rotMat[10]
        };

        cameraPosition[0] -= forward[0] * speed * deltaTime;
        cameraPosition[2] -= forward[2] * speed * deltaTime;

        printf("W key (Forward) pressed (camera-relative)\n");
    }
    else if(g_input.keys[VK_UP]) {
        cameraPosition[2] += speed* deltaTime;
        printf("Up key pressed\n");
    }
    else if(g_input.keys[VK_DOWN]) {
        cameraPosition[2] -= speed  * deltaTime;
        printf("Down key pressed\n");
    }
    static int lastMouseX = 0, lastMouseY = 0;

    UserInput_GetMousePosition(&g_input, &mouseX, &mouseY);

    if (g_input.mouseButtons[0]) {
        int dx = mouseX - lastMouseX;
        int dy = mouseY - lastMouseY;

        cameraRotation[0] += dy * 0.01f; // Pitch
        // Clamp pitch to avoid gimbal lock
        if (cameraRotation[0] > M_PI_2) cameraRotation[0] = M_PI_2;
        if (cameraRotation[0] < -M_PI_2) cameraRotation[0] = -M_PI_2;
        cameraRotation[1] += dx * 0.01f; // Yaw

        printf("Mouse moved: dx=%d, dy=%d\n", dx, dy);
    }

    // Update last position for the next frame
    lastMouseX = mouseX;
    lastMouseY = mouseY;




    if (!viewMatrix) return;

   

    CreateTranslationMatrix(-cameraPosition[0], -cameraPosition[1], -cameraPosition[2], translation);
    CreateRotationMatrix(-cameraRotation[0], -cameraRotation[1], -cameraRotation[2], rotation);
    //MultiplyMatrices(rotation, translation, viewMatrix);
    MultiplyMatrices(translation, rotation, viewMatrix);
}

void CameraControl_Cleanup() {
   
}

void CameraControl_SetPosition(float x, float y, float z) {
    cameraPosition[0] = x;
    cameraPosition[1] = y;
    cameraPosition[2] = z;
}

void CameraControl_SetRotation(float pitch, float yaw, float roll) {
    cameraRotation[0] = pitch;
    cameraRotation[1] = yaw;
    cameraRotation[2] = roll;
}

void CameraControl_GetPosition(float* x, float* y, float* z) {
    if (x) *x = cameraPosition[0];
    if (y) *y = cameraPosition[1];
    if (z) *z = cameraPosition[2];
}

void CameraControl_GetRotation(float* pitch, float* yaw, float* roll) {
    if (pitch) *pitch = cameraRotation[0];
    if (yaw)   *yaw   = cameraRotation[1];
    if (roll)  *roll  = cameraRotation[2];
}