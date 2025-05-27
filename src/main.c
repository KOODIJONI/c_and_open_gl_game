#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include "renderer.h"
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "gl_loader.h"
#include "user_input.h"
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef unsigned int GLuint;
typedef int GLint;
 UserInput g_input; // Add this at the top if not already declared

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static BOOL(WINAPI * wglSwapIntervalEXT)(int) = NULL;
static LARGE_INTEGER frequency;
static LARGE_INTEGER prevTime;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitWGL(HDC hdc) {
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (BOOL(WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT");

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    WNDCLASS wc = {0};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OpenGLWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, wc.lpszClassName, "Minimal OpenGL Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    HDC hdc = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR) };
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) return -1;
    if (!SetPixelFormat(hdc, pf, &pfd)) return -1;

    InitWGL(hdc);
    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);
    if (!hglrc) return -1;

    wglMakeCurrent(hdc, hglrc);
    LoadGLFunctions();

    Renderer_Init();

    if (wglSwapIntervalEXT) wglSwapIntervalEXT(1); 

    MSG msg;
    BOOL running = TRUE;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prevTime);

    while (running) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        
        UserInput_UpdateMousePosition(&g_input, hwnd);

        double deltaTime = (double)(currentTime.QuadPart - prevTime.QuadPart) / frequency.QuadPart;
        prevTime = currentTime;


        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
       
        Renderer_Draw(deltaTime);

        

        SwapBuffers(hdc);
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_KEYDOWN:
        UserInput_ProcessKeyboard(&g_input, wParam, true);
        return 0;
    case WM_KEYUP:
        UserInput_ProcessKeyboard(&g_input, wParam, false);
        return 0;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        UserInput_ProcessMouse(&g_input, lParam, wParam);
        return 0;

    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);
        float aspect = (float)width / (float)height;
        UpdateProjectionMatrix(aspect);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}