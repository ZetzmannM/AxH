#include "surface.h"

#include "errhndl.h"
#include "utils.h"

#ifdef _ENV_WIN
#include <tchar.h>
#endif

using namespace Win;

wrap_ptr<Win::Window> WindowManager::handleWindow = wrap_ptr<Win::Window>();
#ifdef _ENV_WIN
bool WindowManager::registeredClass = false;
HCURSOR WindowManager::tmpCursor = NULL;
#endif

#ifdef _ENV_WIN
PIXELFORMATDESCRIPTOR WindowProperties::getPixelDescriptor() const {
	PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),
	(BYTE)1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	(BYTE)colorBits,
	(BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0,
	(BYTE)0,
	(BYTE)0,
	(BYTE)0,
	(BYTE)0, (BYTE)0, (BYTE)0, (BYTE)0,
	(BYTE)depthBits,
	(BYTE)stencilBits,
	(BYTE)0,
	PFD_MAIN_PLANE,
	(BYTE)0,
	(BYTE)0, (BYTE)0, (BYTE)0
	};
	return pfd;
}
UINT WindowProperties::getHWNDStyle() const {
	if (decorated) {
		return WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX & ~WS_CAPTION;
	}
	else {
		return WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
	}
}
#endif

WindowProperties::WindowProperties(int16 width, int16 height) {
	this->height = height;
	this->width = width;
}

#ifdef _ENV_WIN
Window::Window(const WindowProperties& prop, HWND hwnd) : properties(prop) {
	this->hWnd = hwnd;
	this->hdc = GetDC(hWnd);

}
void Window::show() {
	active = true;
	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);
}
void Window::hide() {
	ShowWindow(hWnd, SW_HIDE);
}
void Window::_destroy() {
	DestroyWindow(hWnd);

	hWnd = 0;
}
HWND Window::getHWnd() {
	return hWnd;
}
LRESULT Window::wndProcCall(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DESTROY:
	case WM_CLOSE:
		this->active = false;
		PostQuitMessage(0);
		return 0;
		break;
	case WM_MOVE:
		this->x = (int)(short)LOWORD(lParam);   // horizontal position 
		this->y = (int)(short)HIWORD(lParam);   // vertical position 
		return 0;
		break;
	case WM_MOUSELEAVE: {
		if (this->set.mouseLeave) {
			this->set.mouseLeave();
		}
		this->focused = false;

		return 0;
	}break;
	case WM_MOUSEMOVE:
		if (!this->focused) {
			if (this->set.mouseEnter) {
				this->set.mouseEnter();
			}
			this->focused = true;
			TRACKMOUSEEVENT evt = {};
			evt.cbSize = sizeof(TRACKMOUSEEVENT);
			evt.hwndTrack = hWnd;
			evt.dwFlags = TME_LEAVE;
			evt.dwHoverTime = 0;

			TrackMouseEvent(&evt);

		}
		break;
	case WM_SIZE:
		this->properties.width = (int)(short)LOWORD(lParam);
		this->properties.height = (int)(short)HIWORD(lParam);
		return 0;
		break;
	}

	return 1;
}

HDC Window::getDeviceContext() {
	return hdc;
}
math::vector<int32, 2> Win::Window::getWindowPosition() const {
	return { this->x, this->y };
}
#endif
#ifdef _ENV_LINUX
Win::Window::Window(const WindowProperties& prop, GLFWwindow* hWnd) : properties(prop) {
	this->hWnd = hWnd;
}
void Win::Window::show() {
	this->active = true;
	glfwShowWindow(this->getHWnd());
}
void Win::Window::hide() {
	this->active = false;
	glfwHideWindow(this->getHWnd());
}
void Win::Window::_destroy() {
	this->active = false,
		glfwDestroyWindow(this->getHWnd());
}
GLFWwindow* Win::Window::getHWnd() {
	return this->hWnd;
}
Math::Point<int32, 2> Win::Window::getWindowPosition() const {
	int x = 0, y = 0;
	glfwGetWindowPos(this->hWnd, &x, &y);
	return { x, y };
}
#endif

math::vector<int32, 2> Win::Window::toWindowRelativeCoordinates(const math::vector<int32, 2>& ref) const {
	math::vector<int32, 2> pos2 = this->getWindowPosition();
	return ref - math::vector<int32, 2>({ pos2[0], pos2[1] });
}
math::vector<int32, 2> Win::Window::toScreenRelativeCoordinates(const math::vector<int32, 2>& ref) const {
	math::vector<int32, 2> pos2 = this->getWindowPosition();
	return ref + math::vector<int32, 2>({ pos2[0], pos2[1] });
}

Win::Window::~Window() {
	_destroy();
}

Win::Window& WindowManager::getCurrentFocusedWindow() {
	return *handleWindow.operator->();
}
void WindowManager::giveFocus(pass_ptr<Window>& ref) {
	destroyFocused();
	handleWindow = ref;
}
void WindowManager::giveFocus(pass_ptr<Window>&& ref) {
	giveFocus(ref);
}
void WindowManager::destroyFocused() {
	if (handleWindow.valid()) {
		handleWindow.discard();
	}
}

#ifdef _ENV_WIN
void WindowManager::hideCursor() {
	tmpCursor = SetCursor(NULL);
}
void WindowManager::showCursor() {
	if (tmpCursor) {
		SetCursor(tmpCursor);
	}
}
#endif

#ifdef _ENV_WIN
pass_ptr<Window> WindowManager::createNewWindow(WIN_BOUNDS rect, const HINSTANCE hInstance, const WindowProperties& properties, const std::string& title, WNDPROC wndProc) {
	TCHAR szWindowClass[] = _T("TESTFORMS");
	std::wstring wtitle = Util::StringUtils::stringToWideString(title);
	const TCHAR* szTitle = title.c_str();

	if (!registeredClass) {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = wndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		wcex.hCursor = NULL; //LoadCursor(NULL, IDC_ARROW)
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = szWindowClass;
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&wcex)) {
			PRINT_ERR("Failed to register the window class", PRIORITY_HALT, CHANNEL_WIN32);
		}
		else {
			registeredClass = true;
		}
	}

	DWORD style = WS_OVERLAPPEDWINDOW;

	if (!properties.resizeable) {
		style ^= WS_THICKFRAME;
		style ^= WS_MINIMIZE;
	}

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		style,
		(rect.right - rect.left) / 2 - properties.width / 2, std::abs(rect.top - rect.bottom) / 2 - properties.height / 2,
		properties.width, properties.height,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd) {
		PRINT_ERR("Failed Window Creation!", PRIORITY_HALT, CHANNEL_WIN32);
	}


	PIXELFORMATDESCRIPTOR pfd = properties.getPixelDescriptor();
	int piFormats[] = { 0 };

	piFormats[0] = ChoosePixelFormat(GetDC(hWnd), &pfd);
	SetPixelFormat(GetDC(hWnd), piFormats[0], &pfd);

	//"enable" WM_MOUSE_LEAVE
	TRACKMOUSEEVENT evt = {};
	evt.cbSize = sizeof(TRACKMOUSEEVENT);
	evt.hwndTrack = hWnd;
	evt.dwFlags = TME_LEAVE;
	evt.dwHoverTime = 0;

	TrackMouseEvent(&evt);

	Window* win = new Window(properties, hWnd);
	return pass_ptr<Window>(win);
}
HDC WindowManager::getFocusedDeviceContext() {
	return handleWindow->hdc;
}
#endif
#ifdef _ENV_LINUX
pass_ptr<Win::Window> WindowManager::createNewWindow(WIN_BOUNDS rect, GLFWmonitor* hmon, const WindowProperties& properties, const std::string& title) {
	GLFWwindow* handle = 0;
	Win::Window* win = 0;

	glfwWindowHint(GLFW_RED_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_GREEN_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_BLUE_BITS, properties.colorBits / 3);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, properties.depthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, properties.stencilBits);
	glfwWindowHint(GLFW_SAMPLES, properties.msaaCount);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	if (!properties.resizeable) {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}
	else {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}

	handle = glfwCreateWindow(properties.width, properties.height, title.c_str(), NULL, NULL);
	if (handle) {
		win = new Win::Window(properties, handle);
	}
	else {
		PRINT_ERR("Window Creation Failed! (OGL_FAIL|GLFW_FAIL)", PRIORITY_HALT, CHANNEL_WIN32);
	}
	return pass_ptr<Win::Window>(win);
}
#endif

Win::TimeHandler::TimeHandler() : TimeHandler(false, V_SYNC_FREQ, 20) {}
Win::TimeHandler::TimeHandler(bool limitFPS, uint64 fps, double ticksPerFrame) {
	this->fps = fps;
	this->limit = limitFPS;
	this->ticksPerSecond = ticksPerFrame;
}
uint64 Win::TimeHandler::getTickCount() {
	return (uint64)((elapsedTime.count() / 1000000000) * ticksPerSecond);
}

void Win::TimeHandler::vsync() {
	if (limit) {
		if (!running) {
			auto f = std::chrono::high_resolution_clock::now();
			double diff = 0;
			double fixValue = 1000000000L / fps - elapsedTime.count();
			while (diff < fixValue) {
				diff = static_cast<double>((std::chrono::high_resolution_clock::now() - f).count());
			}
		}
		else {
			PRINT_ERR("Measurement still running!", 0, CHANNEL_WIN32);
		}
	}
}
void Win::TimeHandler::start() {
	if (!this->running) {
		this->cycVal = _cyclicMeasurement();

		auto it = this->marks.begin();
		while (it != this->marks.end()) {
			it->second.erase();
			it++;
		}

		this->running = true;
		this->tstart = std::chrono::high_resolution_clock::now();
	}
	else {
		PRINT_ERR("Measurement already running!", 0, CHANNEL_WIN32);
	}
}
void Win::TimeHandler::stop() {
	if (this->running) {
		this->elapsedTime = std::chrono::high_resolution_clock::now() - this->tstart;
		this->running = false;

		auto it = this->marks.begin();
		while (it != this->marks.end()) {
			if (it->second.running) {
				(*it).second.stop();
			}
			it++;
		}
	}
	else {
		PRINT_ERR("No measurement running!", 0, CHANNEL_WIN32);
	}
}
void Win::TimeHandler::startMark(const std::string& in) {
	if (this->running) {
		if (!this->marks.count(in)) {
			this->marks[in] = __TimeMark();
		}
		this->marks[in].start();
	}
	else {
		PRINT_ERR("No measurement running!", 0, CHANNEL_WIN32);
	}
}
std::chrono::duration<double, std::nano> Win::TimeHandler::_cyclicMeasurement() {
	if (this->cycMeasure.running) {
		this->cycMeasure.stop();
		std::chrono::duration<double, std::nano> rt = this->cycMeasure.elapsedTime;
		this->cycMeasure.start();
		return rt;
	}
	else {
		this->cycMeasure.start();
		return std::chrono::duration<double, std::nano>(0);
	}
}
void Win::TimeHandler::stopMark(const std::string& in) {
	if (this->running) {
		if (this->marks.count(in)) {
			this->marks[in].stop();
		}
		else {
			PRINT_ERR("No such mark! (_MKEY:" + in + ")", 0, CHANNEL_WIN32);
		}
	}
	else {
		PRINT_ERR("No measurement running!", 0, CHANNEL_WIN32);
	}
}
std::chrono::duration<double, std::nano> Win::TimeHandler::getMarkMeasurement(const std::string& in) {
	if (this->running) {
		if (this->marks.count(in)) {
			return this->marks[in].giveMeasurement();
		}
		else {
			PRINT_ERR("No such mark! (_MKEY:" + in + ")", 0, CHANNEL_WIN32);
		}
	}
	else {
		PRINT_ERR("No measurement running!", 0, CHANNEL_WIN32);
	}
	return std::chrono::duration<double, std::nano>(0);
}
void Win::TimeHandler::__TimeMark::start() {
	if (!this->running) {
		this->running = true;
		erase();
		this->tstart = std::chrono::high_resolution_clock::now();
	}
	else {
		PRINT_ERR("Mark already running!", 0, CHANNEL_WIN32);
	}
}
void Win::TimeHandler::__TimeMark::stop() {
	if (this->running) {
		this->running = false;
		this->elapsedTime = this->elapsedTime + (std::chrono::high_resolution_clock::now() - this->tstart);
	}
	else {
		PRINT_ERR("Mark not running!", 0, CHANNEL_WIN32);
	}
}
void Win::TimeHandler::__TimeMark::erase() {
	this->elapsedTime = std::chrono::duration<double, std::nano>(0);
}
std::chrono::duration<double, std::nano> Win::TimeHandler::__TimeMark::giveMeasurement() {
	return this->elapsedTime;
}
Win::TimeHandler::__TimeMark::__TimeMark() {
	this->elapsedTime = std::chrono::duration<double, std::nano>(0);
	this->running = false;
}
void Win::TimeHandler::setPrintFrequency(uint64 freqRecip) {
	this->printInterludeMaximum = freqRecip;
}
void Win::TimeHandler::printMeasurements(const std::string& key, Stream::print_channel channel) {
	if (this->printInterludeCount % this->printInterludeMaximum == 0) {
		this->printInterludeCount = 1;
	}
	else {
		this->printInterludeCount++;
		return;
	}
	if (!this->running) {
		auto it = this->marks.begin();
		double total = this->elapsedTime.count() / 1000000;
		double val = 0;

		PRINTP("", 0, channel);
		while (it != this->marks.end()) {
			val = it->second.giveMeasurement().count() / 1000000;
			PRINTP("Mark \"" + it->first + "\": " + std::to_string(val) + "ms (" + std::to_string(val / total) + "%)", 0, channel);
			it++;
		}
		double cycDiff = this->cycVal.count();

		PRINTP("-----------------------------------------------------------------------", 0, channel);
		PRINTP("Total: " + std::to_string(total) + "ms (100%)" + "[ActFPS: " + std::to_string(1000000000.0 / cycDiff) + " ; PotFPS: " + std::to_string(1000.0 / (((!key.empty()) ? (total - this->marks[key].giveMeasurement().count() / 1000000) : total))) + "]", 0, channel);
		PRINTP("#######################################################################", 0, channel);

	}
	else {
		PRINT_ERR("Measurement still running!", 0, CHANNEL_WIN32);
	}
}

#ifdef _ENV_WIN
WIN_BOUNDS::WIN_BOUNDS(RECT a) {
	this->left = a.left;
	this->right = a.right;
	this->bottom = a.bottom;
	this->top = a.top;
}
#endif

WIN_BOUNDS::WIN_BOUNDS(long left, long right, long bottom, long top) {
	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;
}
