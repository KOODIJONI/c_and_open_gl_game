#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

void CreateRotationMatrix(float angleX, float angleY, float angleZ, float* matrix);
void MultiplyMatrices(const float* a, const float* b, float* result);
void CreateTranslationMatrix(float tx, float ty, float tz, float* matrix);
void CrossProduct(const float* a, const float* b, float* result);
#endif
