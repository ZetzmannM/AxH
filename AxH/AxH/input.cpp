#include "input.h"

#include "surface.h"
#include "env.h"

using namespace IO;

std::vector<WORD> IO::KeyBoard::consumed = std::vector<WORD>();

bool KeyBoard::isConsumed(WORD key) {
	for (size_t i = 0; i < consumed.size(); i++) {
		if (consumed.at(i) == key) {
			return true;
		}
	}
	return false;
}
void KeyBoard::update() {
	for (size_t i = 0; i < consumed.size(); i++) {
		if (!isPressed(consumed.at(i), false)) {
			consumed.erase(consumed.begin() + i);
			i--;
		}
	}
}
void KeyBoard::consumeEvent(WORD key) {
	consumed.push_back(key);
}
bool KeyBoard::isCtrlPressed() {
#ifdef _ENV_LINUX
	return isPressed(VK_CONTROL_LEFT, false) || isPressed(VK_CONTROL_RIGHT, false);
#else
	return isPressed(VK_CONTROL);
#endif
}

#ifdef _ENV_WIN
bool KeyBoard::isPressed(WORD key, bool mask) {
	if (mask && isConsumed(key)) {
		return false;
	}
	return GetAsyncKeyState(key);
}
void Mouse::setPositionRelativeToWindow(const math::vector<int32, 2>& in) {
	math::vector<int32, 2> pos = in + math::vector<int32, 2>({ Win::WindowManager::getCurrentFocusedWindow().x, Win::WindowManager::getCurrentFocusedWindow().y });
	SetCursorPos(pos[0], pos[1]);
}
math::vector<int32, 2> Mouse::getPositionRelativeToWindow() {
	POINT ptnt = { 0,0 };
	GetCursorPos(&ptnt);
	math::vector<int32, 2> a = { ptnt.x, ptnt.y };
	math::vector<int32, 2> winPos = math::vector<int32, 2>({ -Win::WindowManager::getCurrentFocusedWindow().x, -Win::WindowManager::getCurrentFocusedWindow().y });
	return a + winPos;
}
#endif

#ifdef _ENV_LINUX
bool KeyBoard::isPressed(WORD key, bool mask) {
	if (mask && isConsumed(key)) {
		return false;
	}
	return glfwGetKey(Win::WindowManager::getCurrentFocusedWindow().getHWnd(), key) == GLFW_PRESS;
}
void Mouse::setPositionRelativeToWindow(const Math::Point<int32, 2>& in) {
	glfwSetCursorPos(Win::WindowManager::getCurrentFocusedWindow().getHWnd(), in[0], in[1]);
}
Math::Point<int32, 2> Mouse::getPositionRelativeToWindow() {
	double x = 0, y = 0;
	glfwGetCursorPos(Win::WindowManager::getCurrentFocusedWindow().getHWnd(), &x, &y);
	int32 xp = std::floor(x);
	int32 yp = std::floor(y);
	return { xp, yp };
}
#endif


void Mouse::setToCenter() {
	setPositionRelativeToWindow(getWindowCenterRelativeToWindow());
}
math::vector<int32, 2> Mouse::getWindowCenterRelativeToWindow() {
	return math::vector<int32, 2>({ Win::WindowManager::getCurrentFocusedWindow().properties.width / 2, Win::WindowManager::getCurrentFocusedWindow().properties.height / 2 });
}
