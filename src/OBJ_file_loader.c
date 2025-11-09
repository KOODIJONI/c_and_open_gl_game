#include "obj_file_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

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

// Helper to parse a single face vertex token of format "v", "v/vt", "v//vn" or "v/vt/vn"
static void parseFaceVertex(const char* token, int* vi, int* vti, int* vni) {
    *vi = -1; *vti = -1; *vni = -1;

    // Copy token to modifiable string
    char buf[64];
    strncpy(buf, token, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;

    char* ptr = buf;
    char* slash1 = strchr(ptr, '/');
    if (!slash1) {
        // Format: v
        *vi = atoi(ptr) - 1;
        return;
    }

    *slash1 = 0;
    *vi = atoi(ptr) - 1;

    char* slash2 = strchr(slash1 + 1, '/');
    if (!slash2) {
        // Format: v/vt
        *vti = atoi(slash1 + 1) - 1;
        return;
    }

    *slash2 = 0;
    if (slash1 + 1 != slash2) {
        // Format: v/vt/vn
        *vti = atoi(slash1 + 1) - 1;
    }
    *vni = atoi(slash2 + 1) - 1;
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
            int faceVertexCount = 0;
            int vis[3] = {-1, -1, -1};   // vertex indices
            int vnis[3] = {-1, -1, -1};  // normal indices
            int vtis[3] = {-1, -1, -1};  // texcoord indices (unused here)

            char* token = strtok(line + 2, " \t\r\n");
            while (token && faceVertexCount < 3) {
                int vi, vti, vni;
                parseFaceVertex(token, &vi, &vti, &vni);
                vis[faceVertexCount] = vi;
                vtis[faceVertexCount] = vti;
                vnis[faceVertexCount] = vni;
                faceVertexCount++;
                token = strtok(NULL, " \t\r\n");
            }

            if (faceVertexCount == 3) {
                for (int i = 0; i < 3; ++i) {
                    int vi = vis[i];
                    int vni = vnis[i];

                    if (vi < 0 || vi * 3 + 2 >= mesh->vertex_count) {
                        fprintf(stderr, "Invalid vertex index %d at line %d\n", vi, lineNumber);
                        continue;
                    }

                    float vx = mesh->vertices[vi * 3 + 0];
                    float vy = mesh->vertices[vi * 3 + 1];
                    float vz = mesh->vertices[vi * 3 + 2];

                    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
                    if (vni >= 0 && vni * 3 + 2 < mesh->normal_count) {
                        nx = mesh->normals[vni * 3 + 0];
                        ny = mesh->normals[vni * 3 + 1];
                        nz = mesh->normals[vni * 3 + 2];
                    }

                    // Default white color
                    float r = 1.0f, g = 1.0f, b = 1.0f;

                    float* tmp = realloc(mesh->triangle_vertices, (mesh->triangle_vertex_count + 9) * sizeof(float));
                    if (!tmp) {
                        fprintf(stderr, "Out of memory expanding triangle buffer at line %d\n", lineNumber);
                        fclose(file);
                        return 2;
                    }

                    mesh->triangle_vertices = tmp;

                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vx;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vy;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = vz;

                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = nx;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = ny;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = nz;

                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = r;
                    mesh->triangle_vertices[mesh->triangle_vertex_count++] = g;
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

    for (int i = 0; i < 16; ++i) {
        mesh->transform[i] = (i % 5 == 0) ? 1.0f : 0.0f; // Identity matrix
    }

    for (int i = 0; i < 3; ++i) {
        mesh->color[i] = 1.0f;
    }
    return 0;
}

void printVertices(const ObjMesh* mesh) {
    if (!mesh || !mesh->triangle_vertices) return;

    // Now each vertex has 9 floats: pos(3), normal(3), color(3)
    // Triangles = triangle_vertex_count / (3 vertices * 9 floats)
    size_t floatsPerTriangle = 3 * 9;
    for (size_t i = 0; i + floatsPerTriangle <= mesh->triangle_vertex_count; i += floatsPerTriangle) {
        printf("Triangle %zu:\n", i / floatsPerTriangle);
        for (int v = 0; v < 3; ++v) {
            size_t base = i + v * 9;
            printf("  Vertex %c: Pos(%f, %f, %f), Normal(%f, %f, %f), Color(%f, %f, %f)\n",
                'A' + v,
                mesh->triangle_vertices[base + 0], mesh->triangle_vertices[base + 1], mesh->triangle_vertices[base + 2],
                mesh->triangle_vertices[base + 3], mesh->triangle_vertices[base + 4], mesh->triangle_vertices[base + 5],
                mesh->triangle_vertices[base + 6], mesh->triangle_vertices[base + 7], mesh->triangle_vertices[base + 8]);
        }
    }
}
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Compare two vertices for near-equality

static inline int nearlyEqual(float a, float b, float eps) {
    return fabsf(a - b) < eps;
}

static inline int sameVertex(const float* v1, const float* v2, float eps) {
    return nearlyEqual(v1[0], v2[0], eps) &&
           nearlyEqual(v1[1], v2[1], eps) &&
           nearlyEqual(v1[2], v2[2], eps);
}

void ComputeSmoothNormals(ObjMesh* mesh) {
    if (!mesh || !mesh->triangle_vertices || mesh->triangle_vertex_count == 0)
        return;

    const float EPSILON = 1e-4f;
    const float ANGLE_THRESHOLD = cosf(45.0f * 3.14159f / 180.0f); // 45° edge limit
    const size_t vertexCount = mesh->triangle_vertex_count / 9;

    float* positions = (float*)malloc(vertexCount * 3 * sizeof(float));
    float* faceNormals = (float*)malloc(vertexCount * 3 * sizeof(float));

    // Step 1: Copy positions
    for (size_t i = 0; i < vertexCount; ++i) {
        positions[i*3 + 0] = mesh->triangle_vertices[i*9 + 0];
        positions[i*3 + 1] = mesh->triangle_vertices[i*9 + 1];
        positions[i*3 + 2] = mesh->triangle_vertices[i*9 + 2];
    }

    // Step 2: Compute per-face normals
    for (size_t i = 0; i + 3 <= vertexCount; i += 3) {
        float* v0 = &positions[(i+0)*3];
        float* v1 = &positions[(i+1)*3];
        float* v2 = &positions[(i+2)*3];
        float e1[3] = { v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2] };
        float e2[3] = { v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2] };
        float fn[3] = {
            e1[1]*e2[2] - e1[2]*e2[1],
            e1[2]*e2[0] - e1[0]*e2[2],
            e1[0]*e2[1] - e1[1]*e2[0]
        };
        float len = sqrtf(fn[0]*fn[0] + fn[1]*fn[1] + fn[2]*fn[2]);
        if (len > 1e-6f) { fn[0]/=len; fn[1]/=len; fn[2]/=len; }

        for (int j = 0; j < 3; ++j) {
            faceNormals[(i+j)*3 + 0] = fn[0];
            faceNormals[(i+j)*3 + 1] = fn[1];
            faceNormals[(i+j)*3 + 2] = fn[2];
        }
    }

    // Step 3: Smooth normals for shared vertices if their faces aren't too sharp
    for (size_t i = 0; i < vertexCount; ++i) {
        float sum[3] = {0, 0, 0};
        int count = 0;

        for (size_t j = 0; j < vertexCount; ++j) {
            if (sameVertex(&positions[i*3], &positions[j*3], EPSILON)) {
                // Check if faces are similar direction
                float* n1 = &faceNormals[i*3];
                float* n2 = &faceNormals[j*3];
                float dot = n1[0]*n2[0] + n1[1]*n2[1] + n1[2]*n2[2];
                if (dot >= ANGLE_THRESHOLD) { // only smooth if faces are close in angle
                    sum[0] += n2[0];
                    sum[1] += n2[1];
                    sum[2] += n2[2];
                    count++;
                }
            }
        }

        if (count > 0) {
            float len = sqrtf(sum[0]*sum[0] + sum[1]*sum[1] + sum[2]*sum[2]);
            if (len > 1e-6f) {
                sum[0] /= len; sum[1] /= len; sum[2] /= len;
            }
            mesh->triangle_vertices[i*9 + 3] = sum[0];
            mesh->triangle_vertices[i*9 + 4] = sum[1];
            mesh->triangle_vertices[i*9 + 5] = sum[2];
        }
    }

    free(positions);
    free(faceNormals);

    fprintf(stderr, "[ComputeSmoothNormals] Edge-aware smoothing done (%zu verts)\n", vertexCount);
}