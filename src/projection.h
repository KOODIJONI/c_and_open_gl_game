#ifndef PROJECTION_H
#define PROJECTION_H

void CreateOrthographicProjection(float left, float right, float bottom, float top, float near, float far, float outMatrix[16]);

void CreatePerspectiveProjection(float fovY, float aspect, float near, float far, float outMatrix[16]);

#endif // PROJECTION_H
