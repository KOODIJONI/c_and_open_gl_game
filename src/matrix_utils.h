#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

void CreateRotationMatrix(float angleX, float angleY, float angleZ, float* matrix);
void MultiplyMatrices(const float* a, const float* b, float* result);
void CreateTranslationMatrix(float tx, float ty, float tz, float* matrix);
void CrossProduct(const float* a, const float* b, float* result);
void TransformVertex(const float m[16], const float in[3], float out[3]);
void CreateIdentityMatrix(float* matrix);
void CreateScaleMatrix(float sx, float sy, float sz, float* matrix);
#endif
