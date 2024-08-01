#pragma once
#include "main.h"

#include "CBike.h"
#include "CStreaming.h"
#include "CAutomobile.h"
#include "CWorld.h"
#include "CTheScripts.h"
#include "CModelInfo.h"
#include "extensions/ScriptCommands.h"
#include "CCamera.h"
#include "CStats.h"
#include "CGame.h"
#include "CCutsceneMgr.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer {
	extern void init();

	extern bool show;
	extern bool in_login;

	using f_EndScene = HRESULT(CALLBACK*)(IDirect3DDevice9*);
	using f_Reset = HRESULT(CALLBACK*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

	static inline f_EndScene oEndScene;
	static inline f_Reset oReset;
	static inline WNDPROC oWndProc;

	static LRESULT CALLBACK wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static HRESULT CALLBACK endscene(IDirect3DDevice9* pDevice);
	static HRESULT CALLBACK reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

}