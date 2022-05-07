#include "Env.h"

#ifndef __INPUT_H
#define __INPUT_H

#include <map>
#include <string>

#ifdef _ENV_WIN32
#include <windows.h>
#endif

#ifdef _ENV_LINUX
#include <GLFW/glfw3.h>
#endif

#include "Math.h"

#define MOUSE_LEFT 0x00 //0
#define MOUSE_RIGHT 0x01 //1
#define MOUSE_WHEEL 0x02 //2

#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39

#ifdef _ENV_WIN
#define VK_CONTROL 0x11
#endif

#ifdef _ENV_LINUX
#define VK_CONTROL_RIGHT GLFW_KEY_RIGHT_CONTROL
#define VK_CONTROL_LEFT GLFW_KEY_LEFT_CONTROL
#endif

namespace IO {
	struct KeyBoard {
	private:
		static std::vector<WORD> consumed;
	public:
		///<summary>
		///Returns whether the current key is pressed.
		///<param name='key'>Key Code</param>
		///<param name='mask'>Ignore consumed keys</param>
		///</summary>
		static bool isPressed(WORD key, bool mask = true);

		///<summary>
		///Marks the Key as consumed, that is to say it may be ignored
		///When queried by 'isPressed'
		///<param name='key'>Key Code</param>
		///</summary>
		static void consumeEvent(WORD key);

		///<summary>
		///Returns whether (a) Control key is pressed. (Ignores Consumption)
		///</summary>
		static bool isCtrlPressed();

		///<summary>
		///Returns whether the Key is consumed
		///</summary>
		static bool isConsumed(WORD key);

		///<summary>
		///Handles Consumption resets, to be called regularly inside of the render loop;
		///It resets consumption, if the key is released by the time this method is called.
		///</summary>
		static void update();
	};

	///<summary>
	///Very Basic Interface to Mouse Position
	///Use Win::Window's (in Surface.h Module) coordinate conversion methods to convert between Screen relative and window relative coordinates.
	///</summary>
	struct Mouse {
	public:
		///<summary>
		///Sets the Position of the Cursor relative to the current Screen
		///</summary>
		static void setPositionRelativeToWindow(const math::vector<int32, 2>& in);

		/// <summary>
		///Returns the coordinates of the Center of the screen (relative to the currently focused window)
		/// </summary>
		static math::vector<int32, 2> getWindowCenterRelativeToWindow();

		///<summary>
		///Returns the Position of the Mouse Cursor relative to the currently focused windows viewport
		///</summary>
		static math::vector<int32, 2> getPositionRelativeToWindow();

		///<summary>
		///Sets the Cursor Position to the center of the currently focused Window
		///</summary>
		static void setToCenter();

	};
}
#endif
