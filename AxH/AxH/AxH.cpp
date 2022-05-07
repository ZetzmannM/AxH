#ifdef _ENV_WIN
#include <tchar.h>
#endif

#include "AxH.h"

#include "env.h"
#include "inc_settings.h"
#include "errhndl.h"
#include "utils.h"
#include "input.h"

#ifdef _ENV_WIN
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	Win::WindowManager::getCurrentFocusedWindow().wndProcCall(hwnd, message, wParam, lParam);
	return (DefWindowProc(hwnd, message, wParam, lParam));
}

#endif

#ifdef _ENV_WIN
int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
#endif

#ifdef _ENV_LINUX
	int main() {
#endif

#ifdef __DEBUG
#ifdef _ENV_WIN
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
#endif
#endif
		if (_Compat()) {

			Win::WIN_BOUNDS window = Win::WIN_BOUNDS(0, 0, 0, 0);

#ifdef _ENV_WIN
			const HWND hDesktop = GetDesktopWindow();
			RECT rect;
			GetWindowRect(hDesktop, &rect);
			window = Win::WIN_BOUNDS(rect);
#endif
#ifdef _ENV_LINUX
			GLFWmonitor* hmon = glfwGetPrimaryMonitor();
			int x, y, w, h;
			glfwGetMonitorWorkarea(hmon, &x, &y, &w, &h);
			window.left = x;
			window.right = x + w;
			window.bottom = y;
			window.top = y + h;
#endif

			Win::WindowProperties props(0, 0);
			if (_Init(window, props)) {
				Win::WindowManager::giveFocus(Win::WindowManager::createNewWindow(window,
#ifdef _ENV_WIN
					hInstance,
#endif
#ifdef _ENV_LINUX
					hmon,
#endif
					props, "Focus2"
#ifdef _ENV_WIN
					, WndProc
#endif
				));


				Win::WindowManager::getCurrentFocusedWindow().show();

				Win::Window& winR = Win::WindowManager::getCurrentFocusedWindow();
				Win::TimeHandler& timeHndl = winR.timeHandl;
				timeHndl.setPrintFrequency(100);

#ifdef _ENV_WIN
				bool done = false;
				MSG msg;
#endif

				while (
#ifdef _ENV_WIN
					!done
#endif
#ifdef _ENV_LINUX
					!glfwWindowShouldClose(winR.getHWnd())
#endif
					) {

#ifdef _ENV_WIN
					while (PeekMessage(&msg, winR.getHWnd(), 0, 0, PM_REMOVE)) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}

					if (!winR.active) {
						done = true;
					}
					else {
#endif
						timeHndl.start();
						uint64 tickC = timeHndl.getTickCount();

#ifdef __DEBUG
						timeHndl.startMark("render");
#endif
						IO::KeyBoard::update();

#ifdef __DEBUG
						timeHndl.stopMark("render");
						timeHndl.startMark("WindowStuff");
						timeHndl.startMark("SwapChain");
#endif			

#ifdef __DEBUG
						timeHndl.stopMark("SwapChain");
#endif
#ifdef _ENV_LINUX

						glfwPollEvents();
#endif
#ifdef __DEBUG
						timeHndl.stopMark("WindowStuff");
#endif 
						timeHndl.stop();
						timeHndl.printMeasurements("SwapChain");
#ifdef _ENV_WIN
					}
#endif
				}
#ifdef _ENV_WIN
				PostQuitMessage(0);
#endif
				Win::WindowManager::destroyFocused();
#ifdef _ENV_LINUX
				glfwTerminate();
#endif
#ifdef __DEBUG
				OutputDebugStringA("++++++++++++++++LEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEK++++++++++++++++\n");
				if (!_CrtDumpMemoryLeaks()) {
					OutputDebugStringA("_CrtDumpMemoryLeaks: No leaks found\n");
				}
				OutputDebugStringA("++++++++++++++++LEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEK++++++++++++++++\n");
#endif
				return 0;
			}
			else {
				PRINT_ERR("Failed Initialization (most likely because of incompatability)", PRIORITY_MESSAGE, CHANNEL_GENERAL_DEBUG); return -1;
				return 1;
			}
		}
		else {
			PRINT_ERR("Unable to start the program (most likely because of incompatability)", PRIORITY_MESSAGE, CHANNEL_GENERAL_DEBUG); return -1;
			return 1;
		}

#ifdef __DEBUG
		OutputDebugStringA("++++++++++++++++LEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEK++++++++++++++++\n");
		if (!_CrtDumpMemoryLeaks()) {
			OutputDebugStringA("_CrtDumpMemoryLeaks: No leaks found\n");
		}
		OutputDebugStringA("++++++++++++++++LEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEKLEEK++++++++++++++++\n");
#endif

		return 0;
	}

	bool _Compat() {
		bool flag = true;
#ifdef _ENV_LINUX
		flag &= glfwInit();
		flag &= glfwVulkanSupported();
#endif
#ifdef _ENV_WIN

#endif

		return flag;
	}
	bool _Init(Win::WIN_BOUNDS winSize, Win::WindowProperties & props) {
		props.width = (int16)(winSize.right - winSize.left) / 2; //!!HARD_CODED_OPTION!!
		props.height = (int16)std::abs(winSize.top - winSize.bottom) / 2; //!!HARD_CODED_OPTION!!
		props.width = 800;
		props.height = 800;

		props.msaaCount = 1; //!!HARD_CODED_OPTION!!

		return true;
	}

