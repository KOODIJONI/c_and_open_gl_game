#include "obj_file_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

static void printWorkingDir(void) {
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd))) {
        fprintf(stderr, "Current working directory: %s\n", cwd);
    } else {
        fprintf(stderr, "Warning: unable to get working directory: %s\n", strerror(errno));
    }
}

static void initMesh(ObjMesh* mesh) {
    mesh->vertices = NULL;
    mesh->vertex_count = 0;
    mesh->texcoords = NULL;
    mesh->texcoord_count = 0;
    mesh->normals = NULL;
    mesh->normal_count = 0;
    mesh->indices = NULL;
    mesh->index_count = 0;
    mesh->triangle_vertices = NULL;
    mesh->triangle_vertex_count = 0;
}

void freeMesh(ObjMesh* mesh) {
    if (!mesh) return;
    free(mesh->vertices);
    free(mesh->texcoords);
    free(mesh->normals);
    free(mesh->indices);
    free(mesh->triangle_vertices);
    initMesh(mesh);
}

int LoadOBJ(const char* filename, ObjMesh* mesh) {
    if (!filename || !mesh) {
        fprintf(stderr, "LoadOBJ error: null pointer provided\n");
        return 1;
    }

    printWorkingDir();

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: failed to open OBJ file '%s': %s\n", filename, strerror(errno));
        return 1;
    }

    initMesh(mesh);

    char line[512];
    int lineNumber = 0;

    while (fgets(line, sizeof(line), file)) {
        lineNumber++;

        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) {
                float* tmp = realloc(mesh->vertices, (mesh->vertex_count + 3) * sizeof(float));
                if (!tmp) {
                    fprintf(stderr, "Error: out of memory expanding vertices at line %d\n", lineNumber);
                    fclose(file);
                    return 2;
                }
                mesh->vertices = tmp;
                mesh->vertices[mesh->vertex_count++] = x;
                mesh->vertices[mesh->vertex_count++] = y;
                mesh->vertices[mesh->vertex_count++] = z;
            } else {
                fprintf(stderr, "Warning: malformed vertex at line %d: %s", lineNumber, line);
            }
        } else if (strncmp(line, "vt ", 3) == 0) {
            float u = 0.0f, v = 0.0f;
            if (sscanf(line + 3, "%f %f", &u, &v) >= 1) {
                float* tmp = realloc(mesh->texcoords, (mesh->texcoord_count + 2) * sizeof(float));
                if (!tmp) {
                    fprintf(stderr, "Error: out of memory expanding texcoords at line %d\n", lineNumber);
                    fclose(file);
                    return 2;
                }
                mesh->texcoords = tmp;
                mesh->texcoords[mesh->texcoord_count++] = u;
                mesh->texcoords[mesh->texcoord_count++] = v;
            } else {
                fprintf(stderr, "Warning: malformed texcoord at line %d: %s", lineNumber, line);
            }
        } else if (strncmp(line, "vn ", 3) == 0) {
            float nx, ny, nz;
            if (sscanf(line + 3, "%f %f %f", &nx, &ny, &nz) == 3) {
                float* tmp = realloc(mesh->normals, (mesh->normal_count + 3) * sizeof(float));
                if (!tmp) {
                    fprintf(stderr, "Error: out of memory expanding normals at line %d\n", lineNumber);
                    fclose(file);
                    return 2;
                }
                mesh->normals = tmp;
                mesh->normals[mesh->normal_count++] = nx;
                mesh->normals[mesh->normal_count++] = ny;
                mesh->normals[mesh->normal_count++] = nz;
            } else {
                fprintf(stderr, "Warning: malformed normal at line %d: %s", lineNumber, line);
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            int vertex_indices[3] = { 0 };
            int faceVertexCount = 0;

            char* token = strtok(line + 2, " \t\r\n");
            while (token && faceVertexCount < 3) {
                int vi = 0;
                sscanf(token, "%d", &vi);
                vertex_indices[faceVertexCount++] = vi - 1;
                token = strtok(NULL, " \t\r\n");
            }

            if (faceVertexCount == 3) {
                for (int i = 0; i < 3; ++i) {
                    int vi = vertex_indices[i];
                    if (vi < 0 || vi * 3 + 2 >= mesh->vertex_count) {
                        fprintf(stderr, "Invalid vertex index %d at line %d\n", vi, lineNumber);
                        continue;
                    }

                    float vx = mesh->vertices[vi * 3 + 0];
                    float vy = mesh->vertices[vi * 3 + 1];
                    float vz = mesh->vertices[vi * 3 + 2];

                    float* tmp = realloc(mesh->triangle_vertices, (mesh->triangle_vertex_count + 3) * sizeof(float));
                    if (!tmp) {
                        fprintf(stderr, "Out of memory expanding triangle buffer at line %d\n", lineNumber);
                        fclose(file);
                        return 2;
                    }

                    mesh->triangle_vertices = tmp;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vx;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vy;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vz;
                }
            } else {
                fprintf(stderr, "Warning: non-triangle face at line %d: %s", lineNumber, line);
            }
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "Error reading file '%s' at line %d: %s\n", filename, lineNumber, strerror(errno));
        fclose(file);
        return 3;
    }

    fclose(file);
    //printVertices(mesh);
    return 0;
}

void printVertices(const ObjMesh* mesh) {
    if (!mesh || !mesh->triangle_vertices) return;

    for (size_t i = 0; i < mesh->triangle_vertex_count; i += 9) {
        printf("Triangle %zu:\n", i / 9);
        printf("  A: (%f, %f, %f)\n", mesh->triangle_vertices[i+0], mesh->triangle_vertices[i+1], mesh->triangle_vertices[i+2]);
        printf("  B: (%f, %f, %f)\n", mesh->triangle_vertices[i+3], mesh->triangle_vertices[i+4], mesh->triangle_vertices[i+5]);
        printf("  C: (%f, %f, %f)\n", mesh->triangle_vertices[i+6], mesh->triangle_vertices[i+7], mesh->triangle_vertices[i+8]);
    }
}