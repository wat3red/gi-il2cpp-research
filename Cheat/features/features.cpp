#include "esp.h"
#include "auto_talk.h"
#include "auto_loot.h"
#include "map_teleport.h"
#include "noclip.h"
#include "god_mode.h"
#include "skip_cutscene.h"
#include "kill_aura.h"
#include "game_speed.h"
#include "network_analyzer.h"

#include "features.h"
#include <game_api/include.h>

#include <minhook/include/MinHook.h>

bool features::is_initialized = false;
std::vector<Feature*> features::all_features = {};

void (*MiHoYo_SDK_Dll_Update)(void* _this);
void hMiHoYo_SDK_Dll_Update(void* _this) {
	features::UpdateAllFeatures();
	MiHoYo_SDK_Dll_Update(_this);
}

void features::InitAllFeatures() {
	if (is_initialized) return;

	GameSpeed::MarkLocalThread();

	void* MiHoYo_SDK_Dll_Update_ptr = Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MiHoYo.SDK", "Dll", "Update", 0));
	MH_CreateHook(MiHoYo_SDK_Dll_Update_ptr, (LPVOID)hMiHoYo_SDK_Dll_Update, (LPVOID*)&MiHoYo_SDK_Dll_Update);

	all_features.push_back(new ESP());
	all_features.push_back(new AutoTalk());
	all_features.push_back(new MapTeleport());
	all_features.push_back(new AutoLoot());
	all_features.push_back(new Noclip());
	all_features.push_back(new GodMode());
	all_features.push_back(new SkipCutscene());
	all_features.push_back(new KillAura());
	all_features.push_back(new GameSpeed());
	all_features.push_back(new NetworkAnalyzer());

	for (Feature* feature : all_features) {
		feature->OnInit();
	}

	MH_EnableHook(MH_ALL_HOOKS);

	Log("All features are inited!\n");

	is_initialized = true;
}

void features::UpdateAllFeatures() {
	for (Feature* feature : all_features) {
		feature->OnUpdate();
	}
}

void features::DrawAllUI() {
	for (Feature* feature : all_features) {
		feature->DrawUI();
	}
}

void features::DrawAllBackgroundUI() {
	for (Feature* feature : all_features) {
		feature->DrawBackgroundUI();
	}
}

void features::UpdateAllHotkeys() {
	for (Feature* feature : all_features) {
		feature->UpdateHotkeys();
	}
}
