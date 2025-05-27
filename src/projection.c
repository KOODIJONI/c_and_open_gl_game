#include "projection.h"
#include <math.h>

void CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far, float outMatrix[16]) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    outMatrix[0] = 2.0f / rl;
    outMatrix[1] = 0;
    outMatrix[2] = 0;
    outMatrix[3] = 0;

    outMatrix[4] = 0;
    outMatrix[5] = 2.0f / tb;
    outMatrix[6] = 0;
    outMatrix[7] = 0;

    outMatrix[8] = 0;
    outMatrix[9] = 0;
    outMatrix[10] = -2.0f / fn;
    outMatrix[11] = 0;

    outMatrix[12] = -(right + left) / rl;
    outMatrix[13] = -(top + bottom) / tb;
    outMatrix[14] = -(far + near) / fn;
    outMatrix[15] = 1;
}

void CreatePerspectiveProjection(float fovY, float aspect, float near, float far, float outMatrix[16]) {
    float f = 1.0f / tanf(fovY * 0.5f);
    float nf = 1.0f / (near - far);

    outMatrix[0] = f / aspect;
    outMatrix[1] = 0;
    outMatrix[2] = 0;
    outMatrix[3] = 0;

    outMatrix[4] = 0;
    outMatrix[5] = f;
    outMatrix[6] = 0;
    outMatrix[7] = 0;

    outMatrix[8] = 0;
    outMatrix[9] = 0;
    outMatrix[10] = (far + near) * nf;
    outMatrix[11] = -1;

    outMatrix[12] = 0;
    outMatrix[13] = 0;
    outMatrix[14] = (2.0f * far * near) * nf;
    outMatrix[15] = 0;
}
