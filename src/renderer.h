#ifndef RENDERER_H
#define RENDERER_H

void Renderer_Init(void);
void Renderer_Draw(float deltaTime);
void Renderer_Cleanup(void);
float Get_Aspect_Ratio(void);
void UpdateProjectionMatrix(float aspect) ;
void set_vertices(float* newVertices, int count);
#endif
