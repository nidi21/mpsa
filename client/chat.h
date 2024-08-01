#pragma once
#include "renderer.h"
#include "vector"


namespace chat {
	extern bool toggled;
	extern void draw();
	extern std::vector<std::string> messages;
	extern void send_message(std::string message);
}