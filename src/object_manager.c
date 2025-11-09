
#include "object_manager.h"
#include <gl/gl.h>
#include "matrix_utils.h"
void ObjectVector_Init(ObjectVector* vec){
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}
void ObjectVector_Push(ObjectVector* vec, RenderableObject obj){
    if (vec->size >= vec->capacity) {
        int newCapacity = (vec->capacity == 0) ? 4 : vec->capacity * 2;
        RenderableObject* newData = realloc(vec->data, newCapacity * sizeof(RenderableObject));
        if (!newData) {
            return;
        }
        vec->data = newData;
        vec->capacity = newCapacity;
    }
    vec->data[vec->size++] = obj;
}
void ObjectVector_Free(ObjectVector* vec){
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}
RenderableObject CreateRenderableObject(GLuint vao, int vertexCount, float x, float y, float z) {
    RenderableObject obj;
    obj.vao = vao;
    obj.vertexCount = vertexCount;
    CreateTranslationMatrix(x, y, z, obj.modelMatrix); 
    return obj;
}