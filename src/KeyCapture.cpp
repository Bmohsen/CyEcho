#include "KeyCapture.hpp"
#include "Play.hpp"
#include <iostream>
#include <set>

namespace KeyCapture {
	std::unique_ptr<Play::Player> global_player = nullptr;
	std::set<unsigned short> pressed_keys;
	AudioConfig config;

	std::string get_key_name(unsigned short code) {
		static const std::map<unsigned short, std::string> m = {
		{VC_ESCAPE, "Escape"}, {VC_F1, "F1"}, {VC_F2, "F2"}, {VC_F3, "F3"}, {VC_F4, "F4"},
		{VC_F5, "F5"}, {VC_F6, "F6"}, {VC_F7, "F7"}, {VC_F8, "F8"}, {VC_F9, "F9"},
		{VC_F10, "F10"}, {VC_F11, "F11"}, {VC_F12, "F12"}, {VC_BACKQUOTE, "BackQuote"},
		{VC_1, "Num1"}, {VC_2, "Num2"}, {VC_3, "Num3"}, {VC_4, "Num4"}, {VC_5, "Num5"},
		{VC_6, "Num6"}, {VC_7, "Num7"}, {VC_8, "Num8"}, {VC_9, "Num9"}, {VC_0, "Num0"},
		{VC_MINUS, "Minus"}, {VC_EQUALS, "Equal"}, {VC_BACKSPACE, "Backspace"},
		{VC_TAB, "Tab"}, {VC_Q, "KeyQ"}, {VC_W, "KeyW"}, {VC_E, "KeyE"}, {VC_R, "KeyR"},
		{VC_T, "KeyT"}, {VC_Y, "KeyY"}, {VC_U, "KeyU"}, {VC_I, "KeyI"}, {VC_O, "KeyO"},
		{VC_P, "KeyP"}, {VC_OPEN_BRACKET, "LeftBracket"}, {VC_CLOSE_BRACKET, "RightBracket"},
		{VC_BACK_SLASH, "BackSlash"}, {VC_CAPS_LOCK, "CapsLock"}, {VC_A, "KeyA"},
		{VC_S, "KeyS"}, {VC_D, "KeyD"}, {VC_F, "KeyF"}, {VC_G, "KeyG"}, {VC_H, "KeyH"},
		{VC_J, "KeyJ"}, {VC_K, "KeyK"}, {VC_L, "KeyL"}, {VC_SEMICOLON, "SemiColon"},
		{VC_QUOTE, "Quote"}, {VC_ENTER, "Return"}, {VC_SHIFT_L, "ShiftLeft"},
		{VC_Z, "KeyZ"}, {VC_X, "KeyX"}, {VC_C, "KeyC"}, {VC_V, "KeyV"}, {VC_B, "KeyB"},
		{VC_N, "KeyN"}, {VC_M, "KeyM"}, {VC_COMMA, "Comma"}, {VC_PERIOD, "Dot"},
		{VC_SLASH, "Slash"}, {VC_SHIFT_R, "ShiftRight"}, {VC_CONTROL_L, "ControlLeft"},
		{VC_META_L, "MetaLeft"}, {VC_ALT_L, "Alt"}, {VC_SPACE, "Space"},
		{VC_ALT_R, "AltGr"}, {VC_META_R, "MetaRight"}, {VC_CONTROL_R, "ControlRight"},
		{VC_PRINTSCREEN, "PrintScreen"}, {VC_SCROLL_LOCK, "ScrollLock"}, {VC_PAUSE, "Pause"},
		{VC_INSERT, "Insert"}, {VC_HOME, "Home"}, {VC_PAGE_UP, "PageUp"},
		{VC_DELETE, "Delete"}, {VC_END, "End"}, {VC_PAGE_DOWN, "PageDown"},
		{VC_UP, "UpArrow"}, {VC_LEFT, "LeftArrow"}, {VC_DOWN, "DownArrow"}, {VC_RIGHT, "RightArrow"},
		{VC_NUM_LOCK, "NumLock"}, {VC_KP_DIVIDE, "KpDivide"}, {VC_KP_MULTIPLY, "KpMultiply"},
		{VC_KP_SUBTRACT, "KpMinus"}, {VC_KP_7, "Kp7"}, {VC_KP_8, "Kp8"}, {VC_KP_9, "Kp9"},
		{VC_KP_ADD, "KpPlus"}, {VC_KP_4, "Kp4"}, {VC_KP_5, "Kp5"}, {VC_KP_6, "Kp6"},
		{VC_KP_1, "Kp1"}, {VC_KP_2, "Kp2"}, {VC_KP_3, "Kp3"}, {VC_KP_ENTER, "KpReturn"},
		{VC_KP_0, "Kp0"}, {VC_KP_SEPARATOR, "KpDelete"},
		{61000, "UpArrow"},
		{61008, "DownArrow"},
		{61003, "LeftArrow"},
		{61005, "RightArrow"}
		};
		return m.count(code) ? m.at(code) : "";
	}

	void dispatch_proc(uiohook_event* event) {
		if (event->type == EVENT_KEY_PRESSED) {
			if (global_player != nullptr) {
				unsigned short code = event->data.keyboard.keycode;
				if (pressed_keys.find(code) == pressed_keys.end()) {
					pressed_keys.insert(code);
					global_player->playKey(code, config.volume.load());
				}
			}
			
		}
		else if (event->type == EVENT_KEY_RELEASED) {
			unsigned short code = event->data.keyboard.keycode;
			pressed_keys.erase(code);
		}
	}
}