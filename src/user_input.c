#include "user_input.h"
#include <windowsx.h>
#include <stdio.h>
void UserInput_Init(UserInput* input) {
    if (!input) return;
    for (int i = 0; i < 256; ++i) {
        input->keys[i] = false;
    }
    input->mousePosition.x = 0;
    input->mousePosition.y = 0;
    for (int i = 0; i < 3; ++i) {
        input->mouseButtons[i] = false;
    }
}

void UserInput_ProcessKeyboard(UserInput* input, WPARAM wParam, bool pressed) {
    if (!input) return;
    if (wParam < 256) {
        input->keys[wParam] = pressed;
    }
}

void UserInput_ProcessMouse(UserInput* input, LPARAM lParam, WPARAM wParam) {
    if (!input) return;
    
    input->mousePosition.x = GET_X_LPARAM(lParam);
    input->mousePosition.y = GET_Y_LPARAM(lParam);

    
    input->mouseButtons[0] = (wParam & MK_LBUTTON) != 0;
    input->mouseButtons[1] = (wParam & MK_RBUTTON) != 0;
    input->mouseButtons[2] = (wParam & MK_MBUTTON) != 0;
}

void UserInput_UpdateMousePosition(UserInput* input, HWND hwnd) {
    if (!input) return;

    POINT pos;
    if (GetCursorPos(&pos)) {
        ScreenToClient(hwnd, &pos);
        input->mousePosition = pos;
    }
}

void UserInput_Reset(UserInput* input) {
    if (!input) return;
    for (int i = 0; i < 256; ++i) {
        input->keys[i] = false;
    }
    for (int i = 0; i < 3; ++i) {
        input->mouseButtons[i] = false;
    }
}

void UserInput_GetMousePosition(UserInput* input, int* x, int* y) {
    if (!input) return;
    if (x) *x = input->mousePosition.x;
    if (y) *y = input->mousePosition.y;
}

void UserInput_SetMousePosition(UserInput* input, HWND hwnd, int x, int y) {
    if (!input) return;
    POINT pt = { x, y };
    ClientToScreen(hwnd, &pt);
    SetCursorPos(pt.x, pt.y);
    input->mousePosition.x = x;
    input->mousePosition.y = y;
}

