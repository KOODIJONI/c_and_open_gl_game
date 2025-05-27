#include <stdio.h>
#include <stdlib.h>

char* LoadShaderSource(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open shader file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* source = (char*)malloc(size + 1);
    if (!source) {
        fclose(file);
        return NULL;
    }

    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);
    return source;
}