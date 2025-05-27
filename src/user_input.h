#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool keys[256];
    POINT mousePosition; 
    bool mouseButtons[3]; 
} UserInput;

void UserInput_Init(UserInput* input);
void UserInput_Update(UserInput* input, HWND hwnd, WPARAM wParam, LPARAM lParam);
void UserInput_ProcessKeyboard(UserInput* input, WPARAM wParam, bool pressed);
void UserInput_ProcessMouse(UserInput* input, LPARAM lParam, WPARAM wParam);
void UserInput_Reset(UserInput* input);
void UserInput_GetMousePosition(UserInput* input, int* x, int* y);
void UserInput_SetMousePosition(UserInput* input, HWND hwnd, int x, int y) ;
void UserInput_UpdateMousePosition(UserInput* input, HWND hwnd);




#endif