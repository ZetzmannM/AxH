#ifndef _SURFACE_H
#define _SURFACE_H

#include "env.h"
#include "dtypes.h"
#include "ptr.h"
#include "math.h"
#include "errhndl.h"

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#ifdef _ENV_LINUX
#include <GLFW/glfw3.h>
#endif
#ifdef _ENV_WIN
#include <Windows.h>
#endif


namespace Win {
#ifdef _ENV_WIN
#define __CLOCK steady_clock
#endif
#ifdef _ENV_LINUX
#define __CLOCK system_clock
#endif

	struct TimeHandler {
		struct __TimeMark {
			std::chrono::duration<double, std::nano> elapsedTime;
			std::chrono::time_point<std::chrono::__CLOCK> tstart;
			bool running = false;

			__TimeMark();

			void start();
			void stop();
			void erase();

			std::chrono::duration<double, std::nano> giveMeasurement();
		};
	private:

		uint64 printInterludeMaximum = 1;
		uint64 printInterludeCount = 1;
		double ticksPerSecond = 20;
		uint64 fps = 60;
		bool limit = false;
		std::map<std::string, __TimeMark> marks;

		std::chrono::duration<double, std::nano> elapsedTime = std::chrono::duration<double, std::nano>(0);
		__TimeMark cycMeasure;
		std::chrono::duration<double, std::nano> cycVal;

		std::chrono::time_point<std::chrono::__CLOCK> tstart;
		bool running = false;

	public:

		///<summary>Creates a new TimeHandler interface. The interface keeps track of FPS limits (if desired) and tick handling</summary>
		TimeHandler(bool limitFPS, uint64 fps, double ticksPerFrame);

		///<summary>Default TimeHandler, with a limit of 60 FPS, and 1 tick per supposed Frame</summary>
		TimeHandler();

		///<summary>Returns the amount of tickes to be calculated for the LAST MEASURED elapsed time</summary>
		uint64 getTickCount();

		///<summary>blocks, until the elapsed time is at least the required amount of time for the FPS to be 60</summary>
		void vsync();

		///<summmary>Start taking time measurements. Usually done BEFORE a render pass</summary>
		void start();

		///<summary>Stops taking time measurements. Usually done AFTER a render pass</summary>
		void stop();

		///<summary>Starts the time mark.</summary>
		///<param name='in'>The mark id</param>
		void startMark(const std::string& in);

		///<summary>Stops the time mark.</summary>
		///<param name='in'>The mark id</param>
		void stopMark(const std::string& in);

		///<summary>Returns the measurement for the given mark in NANO seconds</summary>
		///<param name='in'>The mark id</param>
		std::chrono::duration<double, std::nano> getMarkMeasurement(const std::string& in);

		/// <summary>
		/// Used to set the frequency of prints for 'printMeasurements'
		/// If set to <i>n</i> printMeasurements will (only) every <i>n</i> calls, the other calls are ignored
		/// </summary>
		/// <param name="freqRecip"></param>
		void setPrintFrequency(uint64 freqRecip);

		///<summary>
		/// Prints all Measurements taken (for every mark). 
		/// Use setPrintFrequency to set the frequency of the outputs
		/// </summary>
		void printMeasurements(const std::string& ref = "", Stream::print_channel channel = CHANNEL_GENERAL_DEBUG);

	private:
		///<summary>This method returns the time that has elapsed since the last function call, or 0 if there was none yet</summary>
		std::chrono::duration<double, std::nano> _cyclicMeasurement();

	};

	struct WIN_BOUNDS {
		long left, right, bottom, top;

#ifdef _ENV_WIN
		WIN_BOUNDS(RECT e);
#endif
		WIN_BOUNDS(long left, long right, long bottom, long top);
	};

	struct WindowProperties {
		int16 width, height;
		uint8 depthBits = 24, stencilBits = 8, colorBits = 32;
		uint8 msaaCount = 1; //@WIN: IS IGNORED
		bool decorated = true;
		bool resizeable = true;

		WindowProperties(int16 width, int16 height);

#ifdef _ENV_WIN
		///<summary>
		/// Returns the PixelDescriptor for the window
		///</summary>
		PIXELFORMATDESCRIPTOR getPixelDescriptor() const;

		///<summary>
		/// Returns the Hwnd Style parameter for the window
		///</summary>
		UINT getHWNDStyle() const;
#endif
	};
	struct WindowManager;

	struct WindowEventCallBackSet {
		std::function<void()> mouseLeave = nullptr;
		std::function<void()> mouseEnter = nullptr;
	};

	struct Window {
		friend struct WindowManager;
		WindowEventCallBackSet set;
	public:
		WindowProperties properties;
		TimeHandler timeHandl;
		std::string title = "";
		bool fullscreen = false;

#ifdef _ENV_WIN
		///<summary>
		/// The position coordinates of the window
		///</summary>
		int16 x = 0, y = 0;
		///<summary>
		/// Whether the frame is currently focused
		///</summary>
		bool focused = false;
#endif

		///<summary>
		/// Whether the frame is currently being rendered
		///</summary>
		bool active = false; // @Linux Leave and Enter events?

	private:
#ifdef _ENV_WIN
		///<summary>
		/// Device hndl
		///</summary>
		HDC hdc = 0;

		///<summary>
		/// Window Handle
		///</summary>
		HWND hWnd = 0;

#endif
#ifdef _ENV_LINUX
		GLFWwindow* hWnd;
#endif

#ifdef _ENV_WIN
		Window(const WindowProperties& prop, HWND hWnd);
#endif
#ifdef _ENV_LINUX
		Window(const WindowProperties& prop, GLFWwindow* hWnd);
#endif

		///<summary>
		///Destroys the window
		///</summary>
		void _destroy();

	public:
		~Window();

		///<summary>
		/// Shows the window and binds the glContext
		///</summary>
		void show();

		///<summary>
		/// Hides the window and unbinds the glContext
		///</summary>
		void hide();

		math::vector<int32, 2> getWindowPosition() const;

		math::vector<int32, 2> toWindowRelativeCoordinates(const math::vector<int32, 2>& ref) const;
		math::vector<int32, 2> toScreenRelativeCoordinates(const math::vector<int32, 2>& ref) const;

#ifdef _ENV_WIN
		///<summary>
		/// Returns the Window Handle
		///</summary>
		HWND getHWnd();

		///<summary>
		/// Returns the Device Context
		///</summary>
		HDC getDeviceContext();

		LRESULT wndProcCall(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
#ifdef _ENV_LINUX
		/**
		 * Returns the Window Handle
		 */
		GLFWwindow* getHWnd();
#endif
	};

	struct WindowManager {
	private:
		static wrap_ptr<Win::Window> handleWindow;
#ifdef _ENV_WIN
		static bool registeredClass;
		static HCURSOR tmpCursor;
#endif

	public:

		///<summary>
		/// Returns the current Window hndl
		///</summary>
		static Win::Window& getCurrentFocusedWindow();

		///<summary>
		/// Creates a new window
		///</summary>
		///<param name='rect'>The Rectangle in which the Window will be centered</param>
		///<param name='hInstance'>hInstance parameter</param>
		///<param name='properties'>Properties of the window</param>
		///<param name='wgl'>Whether wgl should be used for initialization</param>
		///<param name='debugHook'>Whether a debug hook should be placed</param>
#ifdef _ENV_WIN
		static pass_ptr<Window> createNewWindow(WIN_BOUNDS rect, const HINSTANCE hInstance, const WindowProperties& properties, const std::string& title, WNDPROC wndProc);
#endif
#ifdef _ENV_LINUX
		static pass_ptr<Window> createNewWindow(WIN_BOUNDS rect, GLFWmonitor* hmon, const WindowProperties& properties, const std::string& title);
#endif

		///<summary>
		///Sets the current window (and discards the already present)
		///</summar y>
		static void giveFocus(pass_ptr<Win::Window>& ref);
		static void giveFocus(pass_ptr<Win::Window>&& ref);

		static void hideCursor();
		static void showCursor();

		///<summary>
		///Destroys the current window
		///</summary>
		static void destroyFocused();

#ifdef _ENV_WIN
		///<summary>
		/// Gets the current focused deviced context
		///</summary>
		static HDC getFocusedDeviceContext();
#endif	

	};


#ifdef _ENV_WIN
	LRESULT CALLBACK DefWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
}
#endif
