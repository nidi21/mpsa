#pragma once
#include "plugin.h"

#include <Windows.h>
#include <iostream>

#include <d3d9.h>

#include "dependencies/kiero.h"
#include "dependencies/minhook/include/MinHook.h"

#include "dependencies/imgui/imgui.h"
#include "dependencies/imgui/imgui_impl_win32.h"
#include "dependencies/imgui/imgui_impl_dx9.h"
#include "dependencies/imgui/imgui_stdlib.h"

#include "CMenuManager.h"

namespace mpsa {
	void init();
	extern bool loaded;
}