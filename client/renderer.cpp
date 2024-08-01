#include "renderer.h"
#include "input.h"
#include "network.h"
#include "chat.h"

inline renderer::f_EndScene oEndScene = nullptr;
inline renderer::f_Reset oReset = nullptr;
inline WNDPROC oWndProc = nullptr;
bool renderer::show = false;
bool renderer::in_login = true;

static CVehicle* SpawnVehicle(unsigned int modelIndex, CVector position, float orientation) { 
    unsigned char oldFlags = CStreaming::ms_aInfoForModel[modelIndex].m_nFlags;
    CStreaming::RequestModel(modelIndex, GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
    if (CStreaming::ms_aInfoForModel[modelIndex].m_nLoadState == LOADSTATE_LOADED) {
        if (!(oldFlags & GAME_REQUIRED)) {
            CStreaming::SetModelIsDeletable(modelIndex);
            CStreaming::SetModelTxdIsDeletable(modelIndex);
        }
        CVehicle* vehicle = nullptr;
        switch (reinterpret_cast<CVehicleModelInfo*>(CModelInfo::ms_modelInfoPtrs[modelIndex])->m_nVehicleType) {
        case VEHICLE_AUTOMOBILE:
            vehicle = new CAutomobile(modelIndex, 1, true);
            break;
        }
        if (vehicle) {
            vehicle->SetPosn(position);
            vehicle->SetOrientation(0.0f, 0.0f, orientation);
            vehicle->m_nStatus = 4;
            vehicle->m_eDoorLock = DOORLOCK_UNLOCKED;
            CWorld::Add(vehicle);
            CTheScripts::ClearSpaceForMissionEntity(position, vehicle); 
            if (vehicle->m_nVehicleClass == VEHICLE_BIKE)
                reinterpret_cast<CBike*>(vehicle)->PlaceOnRoadProperly();
            else if (vehicle->m_nVehicleClass != VEHICLE_BOAT)
                reinterpret_cast<CAutomobile*>(vehicle)->PlaceOnRoadProperly();
            return vehicle;
        }
    }
    return nullptr;
}

HRESULT CALLBACK renderer::endscene(IDirect3DDevice9* pDevice) {
    if (!ImGui::GetCurrentContext())
    {
        ImGui::CreateContext();
    }

    static bool init = false;
    static bool initGame = false;
    if (!initGame && *reinterpret_cast<DWORD*>(0xC8D4C0) == 7)
    {
        CGame::bMissionPackGame = 0;
        FrontEndMenuManager.DoSettingsBeforeStartingAGame();
        *reinterpret_cast<char*>((uint32_t)&FrontEndMenuManager + 0x32) = 1;
        *reinterpret_cast<char*>((uint32_t)&FrontEndMenuManager + 0xE9) = 0;
        *reinterpret_cast<char*>((uint32_t)&FrontEndMenuManager + 0x5C) = 0;
        *reinterpret_cast<char*>((uint32_t)&FrontEndMenuManager + 0x5D) = 0;
    }
    if (!init && *reinterpret_cast<DWORD*>(0xC8D4C0) == 9) {
        ImGui::GetIO().IniFilename = NULL;

        ImGui_ImplWin32_Init(RsGlobal.ps->window);
        ImGui_ImplDX9_Init(pDevice);

        ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
        ImGui::GetStyle().ButtonTextAlign = ImVec2(0.5f, 0.5f);

        oWndProc = (WNDPROC)SetWindowLongPtr(RsGlobal.ps->window, GWL_WNDPROC, (LRESULT)wndproc);
        init = true;
    }

    if (!init)
        return oEndScene(pDevice);

    if (FrontEndMenuManager.m_bMenuActive)
        return oEndScene(pDevice);


    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    if (!show && network::is_connected())
        chat::draw();
   
    if (show && network::is_connected()) {
        if (chat::toggled)
            chat::toggled = false;

        if (ImGui::GetIO().MouseDrawCursor == false)
           ImGui::GetIO().MouseDrawCursor = true;
        ImGui::Begin("mpsa - debug");
        auto pos = FindPlayerPed()->GetPosition();
        ImGui::Text("ingame stats");

        ImGui::Text("%f, %f, %f : pos", pos.x, pos.y, pos.z);

        ImGui::Spacing();

        ImGui::Text("network stats");
        ImGui::Text("initialized = %d", network::inited);
        ImGui::Text("connected = %d", network::is_connected());


        for (int i = 0; i < 1; i++) {
            if (network::id_in_map(i)) {
                ImGui::Text("player %d: ped = %d",i, (network::players[i].ped));
            }
        }

        ImGui::Spacing();        


        if (ImGui::Button("disconnect"))
            network::disconnect();

        if (ImGui::Button("teleport")) {
            FindPlayerPed()->SetPosn(135, -1551, 9);
        }

        if (ImGui::Button("spawn car")) {
            CVector position = FindPlayerPed(-1)->TransformFromObjectSpace(CVector(0.0f, 5.0f, 0.0f));
            SpawnVehicle(MODEL_INFERNUS, position, FindPlayerPed(-1)->m_fCurrentRotation + 1.5707964f);
        }

        if (ImGui::Button("spawn ped")) {
            auto pos = FindPlayerPed()->GetPosition();
            CStreaming::RequestModel(104, 2);
            CStreaming::LoadAllRequestedModels(false);
            auto ped = new CCivilianPed(PED_TYPE_CIVMALE, 104);
            ped->SetPosn(pos.x, pos.y, pos.z + 2.f);
            CWorld::Add(ped);
        }

        ImGui::End();

    } else if (!network::is_connected()) {
        if (chat::toggled)
            chat::toggled = false;

        static std::string address = "192.168.1.3:40000";
        static std::string name = "nidi";
        auto io = ImGui::GetIO();

        ImGui::GetBackgroundDrawList()->AddRectFilled({ 0,0 }, { ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y }, ImColor(0.f, 0.f, 0.f, 0.8f));

        ImGui::GetIO().MouseDrawCursor = !network::is_connected();

        ImGui::SetNextWindowSize({ 300, 300 });
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("mpsa - connect", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
       
        
        ImGui::InputText("address", &address);
        ImGui::InputText("nickname", &name);

        if (ImGui::Button("connect")) {
            
            network::connect(address);
            
            Sleep(300);
            
            if (network::is_connected()) {
                ImGui::GetIO().MouseDrawCursor = !network::is_connected();
                network::spawn_local_player(name);
            }
        }
        ImGui::End();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    return oEndScene(pDevice);
}

HRESULT CALLBACK renderer::reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
    ImGui_ImplDX9_InvalidateDeviceObjects();

    return oReset(pDevice, pPresentationParameters);
}

LRESULT CALLBACK renderer::wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (!mpsa::loaded && !FrontEndMenuManager.m_bMenuActive)
        return 1;

    if (FrontEndMenuManager.m_bMenuActive)
        return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);

    input::process_keys(uMsg, wParam, lParam);

    if (ImGui::GetIO().WantTextInput)
    {
        plugin::Call<0x53F1E0>(); // CPad::ClearKeyboardHistory
    }

    if (chat::toggled || !network::is_connected() || show) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        return 1;
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void renderer::init() {
	ImGui::CreateContext();

    if (init(kiero::RenderType::D3D9) == kiero::Status::Success)
    {
        kiero::bind(16, (void**)&oReset, reset);
        kiero::bind(42, (void**)&oEndScene, endscene);
    }
}