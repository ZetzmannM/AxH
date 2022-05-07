#ifndef __H_PARABOL
#define __H_PARABOL

#include "inc_settings.h"

#ifdef _ENV_WIN
#ifdef __DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#endif

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include "Env.h"


#ifdef _ENV_LINUX
#include <GLFW/glfw3.h>
#endif
#ifdef _ENV_WIN
#include <Windows.h>
#endif

#include "ptr.h"
#include "surface.h"

#ifdef _ENV_WIN
///<summary>Defines the entry point of the program</summary>
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow);

///<summary>This method processes the window events</summary>
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

#ifdef _ENV_LINUX
///<summary>Defines the entry point of the program</summary>
int main();
#endif


///<summary>
/// Checks the devices compatability
///</summary>
///<param name='hInstance'>The instance hndl</param>
bool _Compat();

///<summary>
/// Initializes the Game
///</summary>
///<param name='winSize'>the Screens size</param>
///<param name='prop'>The window properties</param>
///<param name='prop'>The window properties</param>
bool _Init(Win::WIN_BOUNDS winSize, Win::WindowProperties& prop);

#endif
