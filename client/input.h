#include "main.h"

#pragma once


namespace input {
	extern void init();

	extern void process_keys(UINT msg, WPARAM wParam, LPARAM lParam);
	extern bool bKeyTable[256];
	extern bool is_key_down(uint8_t key);
	extern bool is_key_pressed(uint8_t key);
	extern bool is_key_released(uint8_t vkey);
}