#include "esp.h"
#include "autotalk.h"

#include "features.h"
#include "../sdk/functions/resolve_funcs.h"
#include "../logger.h"

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

	MH_CreateHook((LPVOID)(g_game_base_addr + 0x1422F430), (LPVOID)hMiHoYo_SDK_Dll_Update, (LPVOID*)&MiHoYo_SDK_Dll_Update);

	all_features.push_back(new ESP());
	all_features.push_back(new Autotalk());

	for (Feature* feature : all_features) {
		feature->OnInit();
	}

	MH_EnableHook(MH_ALL_HOOKS);

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
