#include "input.h"
#include "renderer.h"
#include "network.h"
#include "chat.h"


bool input::bKeyTable[256] = { false };

SafetyHookInline CPad_UpdatePads{};
SafetyHookInline psMouseSetPos{};

void __cdecl CPadUpdatePads() {
	if (renderer::show || !network::is_connected() || chat::toggled)
		return;
	CPad_UpdatePads.call<void>();
}

int __cdecl hkpsMouseSetPos(RwV2d* pos) {
	if (renderer::show || !network::is_connected() || chat::toggled)
		return 0;
	return psMouseSetPos.call<int>(pos);
}

void input::init() {
	CPad_UpdatePads = safetyhook::create_inline(reinterpret_cast<void*>(0x541DD0), reinterpret_cast<void*>(&CPadUpdatePads));
	psMouseSetPos = safetyhook::create_inline(reinterpret_cast<void*>(0x7453F0), reinterpret_cast<void*>(&hkpsMouseSetPos));
}

// respect stealth (probabil furat din sob)

void input::process_keys(UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		bKeyTable[VK_LBUTTON] = (msg == WM_LBUTTONDOWN);
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		bKeyTable[VK_RBUTTON] = (msg == WM_RBUTTONDOWN);
		break;

	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		bKeyTable[VK_MBUTTON] = (msg == WM_MBUTTONDOWN);
		break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		bool bDown = (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN);
		int	iKey = (int)wParam;
		uint32_t ScanCode = LOBYTE(HIWORD(lParam));

		bKeyTable[iKey] = bDown;

		switch (iKey)
		{
		case VK_SHIFT:
			if (ScanCode == MapVirtualKey(VK_LSHIFT, 0)) bKeyTable[VK_LSHIFT] = bDown;
			if (ScanCode == MapVirtualKey(VK_RSHIFT, 0)) bKeyTable[VK_RSHIFT] = bDown;
			break;

		case VK_CONTROL:
			if (ScanCode == MapVirtualKey(VK_LCONTROL, 0)) bKeyTable[VK_LCONTROL] = bDown;
			if (ScanCode == MapVirtualKey(VK_RCONTROL, 0)) bKeyTable[VK_RCONTROL] = bDown;
			break;

		case VK_MENU:
			if (ScanCode == MapVirtualKey(VK_LMENU, 0)) bKeyTable[VK_LMENU] = bDown;
			if (ScanCode == MapVirtualKey(VK_RMENU, 0)) bKeyTable[VK_RMENU] = bDown;
			break;
		}
		break;
	}
	}

	if (input::is_key_released(VK_ESCAPE) && (renderer::show || chat::toggled)){
		renderer::show = false;
		chat::toggled = false;
		ImGui::GetIO().MouseDrawCursor = false;

	}

	if (input::is_key_released(VK_INSERT)) {
		renderer::show = !renderer::show;
		ImGui::GetIO().MouseDrawCursor = renderer::show;
	}


	if (input::is_key_released(0x54) && !renderer::show && network::is_connected() && !ImGui::GetIO().WantTextInput) {
		chat::toggled = !chat::toggled;
		ImGui::GetIO().MouseDrawCursor = chat::toggled;
	}
}

bool input::is_key_down(uint8_t iKey)
{
	return bKeyTable[iKey];
}

bool input::is_key_released(uint8_t iKey)
{
	static bool bPressed[0xFF];
	if (!bKeyTable[iKey]) {
		if (bPressed[iKey])
			return !(bPressed[iKey] = false);
	}
	else bPressed[iKey] = true;

	return false;
}

bool input::is_key_pressed(uint8_t iKey)
{
	static bool bPressed[0xFF];
	if (bKeyTable[iKey]) {
		if (!bPressed[iKey])
			return bPressed[iKey] = true;
	}
	else bPressed[iKey] = false;

	return false;
}