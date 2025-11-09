#include <math.h>
#include <string.h>
#include <stdio.h>
void MultiplyMatrices(const float* a, const float* b, float* result) {
    float temp[16];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            temp[row * 4 + col] =
                a[row * 4 + 0] * b[0 * 4 + col] +
                a[row * 4 + 1] * b[1 * 4 + col] +
                a[row * 4 + 2] * b[2 * 4 + col] +
                a[row * 4 + 3] * b[3 * 4 + col];
        }
    }
    memcpy(result, temp, sizeof(float) * 16);
}

void CreateRotationMatrix(float angleX, float angleY, float angleZ, float* matrix) {
    float cx = cosf(angleX), sx = sinf(angleX);
    float cy = cosf(angleY), sy = sinf(angleY);
    float cz = cosf(angleZ), sz = sinf(angleZ);

    float rx[16] = {
        1, 0,  0, 0,
        0, cx, -sx, 0,
        0, sx, cx, 0,
        0, 0,  0, 1
    };
    float ry[16] = {
        cy, 0, sy, 0,
        0,  1, 0,  0,
        -sy, 0, cy, 0,
        0,  0, 0,  1
    };
    float rz[16] = {
        cz, -sz, 0, 0,
        sz, cz,  0, 0,
        0,  0,   1, 0,
        0,  0,   0, 1
    };

    float temp[16];
    MultiplyMatrices(rz, ry, temp);      // temp = Rz * Ry
    MultiplyMatrices(temp, rx, matrix);  // matrix = temp * Rx
}

void CreateTranslationMatrix(float tx, float ty, float tz, float* matrix) {
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = 1.0f; matrix[5] = 1.0f; matrix[10] = 1.0f; matrix[15] = 1.0f;
    matrix[12] = tx; matrix[13] = ty; matrix[14] = tz;
}
void CrossProduct(const float* a, const float* b, float* result) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

void TransformVertex(const float m[16], const float in[3], float out[3]) {
    out[0] = m[0]*in[0] + m[4]*in[1] + m[8]*in[2] + m[12];
    out[1] = m[1]*in[0] + m[5]*in[1] + m[9]*in[2] + m[13];
    out[2] = m[2]*in[0] + m[6]*in[1] + m[10]*in[2] + m[14];
}
void PrintMatrix4(const float* m) {
    for (int row = 0; row < 4; row++) {
        printf("| ");
        for (int col = 0; col < 4; col++) {
            printf("%8.3f ", m[col * 4 + row]);  // OpenGL uses column-major order
        }
        printf("|\n");
    }
    printf("\n");
}

// Helper to print a 3-component vector
void PrintVec3(const float* v) {
    printf("[%.3f, %.3f, %.3f]\n", v[0], v[1], v[2]);
}


void CreateIdentityMatrix(float* matrix) {
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = 1.0f; matrix[5] = 1.0f; matrix[10] = 1.0f; matrix[15] = 1.0f;
}
void CreateScaleMatrix(float sx, float sy, float sz, float* matrix) {
    memset(matrix, 0, sizeof(float) * 16);
    matrix[0] = sx;
    matrix[5] = sy;
    matrix[10] = sz;
    matrix[15] = 1.0f;
}