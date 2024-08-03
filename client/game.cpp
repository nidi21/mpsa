#include "game.h"
#include "scm.h"
#include "utils.h"
#include "network.h"
#include <intrin.h>
#include "chat.h"
#include "renderer.h"

#pragma intrinsic(_ReturnAddress)

SafetyHookInline CPopulationAddToPopulation_{};
SafetyHookInline CRoadBlocksInit_{};
SafetyHookInline RegisterScriptRoadBlock_{};
SafetyHookInline CTheCarGeneratorsProcess_{};
SafetyHookInline GenerateRandomCars_{};
SafetyHookInline GenerateRoadBlocks_{};
SafetyHookInline GenerateRoadBlockCopsForCar_{};
SafetyHookInline CanPlayerStartMission_{};
SafetyHookInline ReportCrime_{};
SafetyHookInline OpenFile_{};
SafetyHookInline CGameShutdown_{};
SafetyHookInline CPedIntelligenceProcess_{};
SafetyHookInline CPopulationRemovePed_{};
SafetyHookInline CPopulationManagePed_{};

char __cdecl CPopulationAddToPopulation(float a1, float a2, float a3, float a4) {
	return 0;
}

void __cdecl CTheCarGeneratorsProcess() {
	return;
}

void __cdecl GenerateRandomCars() {
	return;
}

void __cdecl GenerateRoadBlocks() {
	return;
}

void __cdecl CRoadBlocksInit() {
	return;
}

char __cdecl GenerateRoadBlockCopsForCar(CVehicle* a1, int ped_pos_type, int type) {
	return 0;
}

void __cdecl RegisterScriptRoadBlock(RwV3d cornerA, RwV3d cornerB, unsigned __int8 type) {
	return;
}

bool __fastcall CanPlayerStartMission(void* _this) {
	return false;
}

void __cdecl ReportCrime(int crime, CPed* victim, CPed* suspect) {
	return;
}

void __cdecl CGameShutdown() {
	if (network::is_connected())
		network::disconnect();
	network::shutdown();

	static auto path = get_documents_path();
	if (!path.string().contains("main.scm"))
		path.append("main.scm");

	if (std::filesystem::exists(path))
		std::filesystem::remove(path);
	CGameShutdown_.call<void>();
}

void __cdecl CPopulationRemovePed(CPed* ped) {
	CPopulationRemovePed_.call<void>(ped);
}
void __cdecl CPopulationManagePed(CPed* ped, CVector* playerPos) {

	if (ped == FindPlayerPed()) {
		CPopulationManagePed_.call<void>(ped);
		return;
	}

	if (get_distance(ped->GetPosition()) > 249) {
		printf("manage ped\n");
		CPopulationManagePed_.call<void>(ped);
	}
}

std::chrono::time_point<std::chrono::system_clock> sinceLastCall;

std::filesystem::path write_scm_to_docs()
{
	static auto path = get_documents_path();
	path.append("main.scm");

	auto file = fopen(path.string().c_str(), "wb");
	fwrite(strippedSCM, 1, 2011, file);
	fclose(file);

	return path;
}

FILE* __cdecl _OpenFile(const char* file, const char* mode) {
	if (std::string(file).contains("scm")) {

		static auto path = get_documents_path();
		if (!path.string().contains("main.scm"))
			path.append("main.scm");

		write_scm_to_docs();

		sinceLastCall = std::chrono::system_clock::now();
		return OpenFile_.call<FILE*>(path.string().c_str(), mode);
	}

	return OpenFile_.call<FILE*>(file, mode);
}

DWORD __fastcall CPedIntelligenceProcess(CPedIntelligence* _this) {
	if (_this->m_pPed == FindPlayerPed())
		return CPedIntelligenceProcess_.call<DWORD>(_this);
	return 0;
}

unsigned char ScanListMemory[8 * 20000];

DWORD dwPatchAddrScanReloc1USA[13] = {
0x5DC7AA,0x41A85D,0x41A864,0x408259,0x711B32,0x699CF8,
0x4092EC,0x408702,0x564220,0x564172,0x563845,
0x84E9C2,0x85652D };

DWORD dwPatchAddrScanReloc1EU[13] = {
0x5DC7AA,0x41A85D,0x41A864,0x408261,0x711B32,0x699CF8,
0x4092EC,0x408702,0x564220,0x564172,0x563845,
0x84EA02,0x85656D };

DWORD dwPatchAddrScanReloc2USA[56] = {
0x0040D68C,0x005664D7,0x00566586,0x00408706,0x0056B3B1,0x0056AD91,0x0056A85F,0x005675FA,
0x0056CD84,0x0056CC79,0x0056CB51,0x0056CA4A,0x0056C664,0x0056C569,0x0056C445,0x0056C341,
0x0056BD46,0x0056BC53,0x0056BE56,0x0056A940,0x00567735,0x00546738,0x0054BB23,0x006E31AA,
0x0040DC29,0x00534A09,0x00534D6B,0x00564B59,0x00564DA9,0x0067FF5D,0x00568CB9,0x00568EFB,
0x00569F57,0x00569537,0x00569127,0x0056B4B5,0x0056B594,0x0056B2C3,0x0056AF74,0x0056AE95,
0x0056BF4F,0x0056ACA3,0x0056A766,0x0056A685,0x0070B9BA,0x0056479D,0x0070ACB2,0x006063C7,
0x00699CFE,0x0041A861,0x0040E061,0x0040DF5E,0x0040DDCE,0x0040DB0E,0x0040D98C,0x01566855 };

DWORD dwPatchAddrScanReloc2EU[56] = {
0x0040D68C,0x005664D7,0x00566586,0x00408706,0x0056B3B1,0x0056AD91,0x0056A85F,0x005675FA,
0x0056CD84,0x0056CC79,0x0056CB51,0x0056CA4A,0x0056C664,0x0056C569,0x0056C445,0x0056C341,
0x0056BD46,0x0056BC53,0x0056BE56,0x0056A940,0x00567735,0x00546738,0x0054BB23,0x006E31AA,
0x0040DC29,0x00534A09,0x00534D6B,0x00564B59,0x00564DA9,0x0067FF5D,0x00568CB9,0x00568EFB,
0x00569F57,0x00569537,0x00569127,0x0056B4B5,0x0056B594,0x0056B2C3,0x0056AF74,0x0056AE95,
0x0056BF4F,0x0056ACA3,0x0056A766,0x0056A685,0x0070B9BA,0x0056479D,0x0070ACB2,0x006063C7,
0x00699CFE,0x0041A861,0x0040E061,0x0040DF5E,0x0040DDCE,0x0040DB0E,0x0040D98C,0x01566845 };

DWORD dwPatchAddrScanReloc3[11] = {
0x004091C5,0x00409367,0x0040D9C5,0x0040DB47,0x0040DC61,0x0040DE07,0x0040DF97,
0x0040E09A,0x00534A98,0x00534DFA,0x0071CDB0 };

DWORD dwPatchAddrScanRelocEnd[4] = { 0x005634A6, 0x005638DF, 0x0056420F, 0x00564283 };


//-----------------------------------------------------------

void RelocateScanListHack() // patch pt limita de cplayerinfo in cworld::players (furat din samp)
{
	memset(&ScanListMemory[0], 0, sizeof(ScanListMemory));
	unsigned char* aScanListMemory = &ScanListMemory[0];


	int x = 0;
	while (x != 13) {
		if (plugin::IsGameVersion10us()) {
			*reinterpret_cast<uint32_t*>(dwPatchAddrScanReloc1USA[x]) = (uint32_t)aScanListMemory;
		}
		else if (plugin::IsGameVersion10eu()) {
			*reinterpret_cast<uint32_t*>(dwPatchAddrScanReloc1EU[x]) = (uint32_t)aScanListMemory;

		}
		x++;
	}

	x = 0;
	while (x != 56) {
		if (plugin::IsGameVersion10us()) {
			*(PDWORD)(dwPatchAddrScanReloc2USA[x] + 3) = (DWORD)aScanListMemory;
		}
		else if (plugin::IsGameVersion10eu()) {
			*(PDWORD)(dwPatchAddrScanReloc2EU[x] + 3) = (DWORD)aScanListMemory;
		}
		x++;
	}

	x = 0;
	while (x != 11) {
		*(PDWORD)(dwPatchAddrScanReloc3[x] + 3) = (DWORD)(aScanListMemory + 4);
		x++;
	}

	x = 0;
	while (x != 4) {
		*(PDWORD)(dwPatchAddrScanRelocEnd[x]) = (DWORD)(aScanListMemory + sizeof(ScanListMemory));
		x++;
	}

	*(PDWORD)0x40936A = (DWORD)(aScanListMemory + 4);

	memset((BYTE*)0xB7D0B8, 0, 8 * 14400);
}

void game::init()
{
	CPopulationAddToPopulation_ = safetyhook::create_inline(reinterpret_cast<void*>(0x614720), reinterpret_cast<void*>(&CPopulationAddToPopulation));
	CRoadBlocksInit_ = safetyhook::create_inline(reinterpret_cast<void*>(0x461100), reinterpret_cast<void*>(&CRoadBlocksInit));
	RegisterScriptRoadBlock_ = safetyhook::create_inline(reinterpret_cast<void*>(0x460DF0), reinterpret_cast<void*>(&RegisterScriptRoadBlock));
	CTheCarGeneratorsProcess_ = safetyhook::create_inline(reinterpret_cast<void*>(0x6F3F40), reinterpret_cast<void*>(&CTheCarGeneratorsProcess));
	GenerateRandomCars_ = safetyhook::create_inline(reinterpret_cast<void*>(0x4341C0), reinterpret_cast<void*>(&GenerateRandomCars));
	GenerateRoadBlocks_ = safetyhook::create_inline(reinterpret_cast<void*>(0x4629E0), reinterpret_cast<void*>(&GenerateRoadBlocks));
	GenerateRoadBlockCopsForCar_ = safetyhook::create_inline(reinterpret_cast<void*>(0x461170), reinterpret_cast<void*>(&GenerateRoadBlockCopsForCar));
	CanPlayerStartMission_ = safetyhook::create_inline(reinterpret_cast<void*>(0x609590), reinterpret_cast<void*>(&CanPlayerStartMission));
	ReportCrime_ = safetyhook::create_inline(reinterpret_cast<void*>(0x532010), reinterpret_cast<void*>(&ReportCrime));
	OpenFile_ = safetyhook::create_inline(reinterpret_cast<void*>(0x538900), reinterpret_cast<void*>(&_OpenFile));
	CGameShutdown_ = safetyhook::create_inline(reinterpret_cast<void*>(0x53C900), reinterpret_cast<void*>(&CGameShutdown));
	CPedIntelligenceProcess_ = safetyhook::create_inline(reinterpret_cast<void*>(0x608260), reinterpret_cast<void*>(&CPedIntelligenceProcess));
	CPopulationRemovePed_ = safetyhook::create_inline(reinterpret_cast<void*>(0x610F20), reinterpret_cast<void*>(&CPopulationRemovePed));
	CPopulationManagePed_ = safetyhook::create_inline(reinterpret_cast<void*>(0x611FC0), reinterpret_cast<void*>(&CPopulationManagePed));

	plugin::patch::Nop(0x704E8A, 5); // DrawBlur
	plugin::patch::Nop(0x72DEC0, 5); // updateWantedLevel
	plugin::patch::Nop(0x60EBCC, 5); // CWanted::Update
	plugin::patch::Nop(0x575B0E, 5); // CSprite::DrawRect
	plugin::patch::Nop(0x575917, 5); // CSprite::DrawRect
	plugin::patch::Nop(0x57549D, 5); // CSprite::DrawRect
	plugin::patch::Nop(0x442AE5, 5); // CGameLogic::SetPlayerWantedLevelForForbiddenTerritories
	plugin::patch::Nop(0x53C127, 5); // CConversations::Update
	plugin::patch::Nop(0x53C12C, 5); // CPedToPlayerConversations::Update
	plugin::patch::Nop(0x53C090, 5); // CReplay::Update
	plugin::patch::Nop(0x53BFE2, 5); // CHeli::UpdateHelis
	plugin::patch::Nop(0x6F5900, 5); // CTrain::UpdateTrains
	plugin::patch::Nop(0x438480, 5); // CPad__AddToPCCheatString
	plugin::patch::Nop(0x50781E, 5); // CAEGlobalWeaponAudioEntity::ServiceAmbientGunFire
	plugin::patch::Nop(0x440F4E, 5); // C3dMarkers::PlaceMarkerCone
	plugin::patch::Nop(0x440836, 5); // CEntryExit::GenerateAmbientPeds
	plugin::patch::Nop(0x748C2B, 5); // CLoadingScreen::DoPCTitleFadeIn
	plugin::patch::Nop(0x748C9A, 5); // CLoadingScreen::DoPCTitleFadeOut
	plugin::patch::Nop(0x748C23, 5); // CLoadingScreen::Init
	plugin::patch::Nop(0x748C1C, 5); // ReleaseVideoPlayer
	plugin::patch::Nop(0x748B00, 5); // CreateVideoPlayer
	plugin::patch::Nop(0x58FBE9, 5); // CHud::DrawVehicleNames
	plugin::patch::Nop(0x58D542, 5); // CHud::DrawAreaNames
	plugin::patch::Nop(0x58FC45, 5); // CHud::DrawVitalStats
	plugin::patch::Nop(0x5E91A4, 5); // CPedIntelligence::ProcessFirst
	plugin::patch::Nop(0x6162DE, 5); // CPopulation::ManagePed
	
	plugin::patch::Nop(0x609C08, 39);
	plugin::patch::Nop(0x5E63A6, 19);
	plugin::patch::Nop(0x621AEA, 12);
	plugin::patch::Nop(0x62D331, 11);
	plugin::patch::Nop(0x741FFF, 27);
	plugin::patch::Nop(0x60F2C4, 25);
	*(PBYTE)0x60D64E = 0x84;

	plugin::patch::RedirectShortJump(0x58FC2C, (void*)0x58FC4C);


	for (int i = 0; i < 92; i++) {
		if (CCheat::m_aCheatFunctions[i])
			CCheat::m_aCheatFunctions[i] = nullptr;
	}
	plugin::Events::drawingEvent += []() {
		plugin::Call<0x58FAE0>();
		plugin::Call<0x58EAF0>();
		plugin::Call<0x58FBDB>();
		plugin::Call<0x588A50>(true);
	};

	plugin::Events::gameProcessEvent += []() {
		
		*reinterpret_cast<byte*>(0x6194A0) = (renderer::show || !network::is_connected() || chat::toggled) ? 0xC3 : 0xE9; // NOP RsMouseSetPos
		*reinterpret_cast<byte*>(0x53F3C0) = (renderer::show || !network::is_connected() || chat::toggled) ? 0xC3 : 0x83; // NOP UpdateMouse

		CTheScripts::bDisplayHud = true;
		if (!*reinterpret_cast<bool*>(0xB613E6))
			*reinterpret_cast<bool*>(0xB613E6) = true;

		if (!*reinterpret_cast<bool*>(0x96A7C8))
			*reinterpret_cast<bool*>(0x96A7C8) = true;

		static bool init = false;

		if (!init && FindPlayerPed()) {
			CStreaming::RequestModel(106, GAME_REQUIRED);
			CStreaming::LoadAllRequestedModels(false);
			FindPlayerPed()->SetModelIndex(106);

			for (size_t i = 69; i != 80; ++i)
			{
				CStats::SetStatValue((unsigned short)i, 1000);
			}

			CStats::SetStatValue(160, 1000);
			CStats::SetStatValue(223, 1000);
			CStats::SetStatValue(229, 1000);
			CStats::SetStatValue(230, 1000);
			CStats::SetStatValue(22,  1000);
			CStats::SetStatValue(225, 1000);
			CStats::SetStatValue(165, 1000);

			init = true;
		}

		plugin::Command<0x0777>("BARRIERS1");
		plugin::Command<0x0777>("BARRIERS2");

		static auto path = get_documents_path();
		if (!path.string().contains("main.scm"))
			path.append("main.scm");

		if (std::filesystem::exists(path)) {
			auto ellapsedtime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - sinceLastCall).count() / 1000.0;

			if (ellapsedtime > 3) 
				std::filesystem::remove(path);
		}

		if (!mpsa::loaded && FindPlayerPed()) {
			mpsa::loaded = true;
		}
	};
}