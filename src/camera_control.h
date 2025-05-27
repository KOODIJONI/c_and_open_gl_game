#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

void CameraControl_Init();
void CameraControl_Update(float deltaTime, float* viewMatrix);
void CameraControl_Cleanup();
void CameraControl_SetPosition(float x, float y, float z);
void CameraControl_SetRotation(float pitch, float yaw, float roll);
void CameraControl_GetPosition(float* x, float* y, float* z);
void CameraControl_GetRotation(float* pitch, float* yaw, float* roll);



#endif