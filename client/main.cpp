#include "plugin.h"
#include "main.h"

#include "renderer.h"
#include "input.h"
#include "game.h"
#include "network.h"

bool mpsa::loaded = false;

void mpsa::init() {
    *reinterpret_cast<DWORD*>(0xC8D4C0) = 5; // gGameState

    MH_Initialize();

    game::init();
    renderer::init();
    //input::init();
    network::init();
}

void MainThread() {    
    plugin::Events::initRwEvent += mpsa::init;
    ExitThread(0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        DisableThreadLibraryCalls(hModule);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, nullptr, 0, 0);
    }
    return 1;
}