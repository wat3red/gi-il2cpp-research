#include "skip_cutscene.h"

namespace features
{
	void SkipCutscene::DrawUI() {
		ImGuiEx::Checkbox("Skip cutscenes", config.skip_cutscene.enabled);
	}

	void SkipCutscene::DrawBackgroundUI() {}
	void SkipCutscene::OnUpdate() {}

	void (*CriwareMediaPlayer_Update)(void* _this);
	void hCriwareMediaPlayer_Update(void* _this) {
		if (config.skip_cutscene.enabled) Il2Cpp::Method::Call<void>("MoleMole", "CriwareMediaPlayer", "Skip", 0, _this);
		CriwareMediaPlayer_Update(_this);
	}

	void SkipCutscene::OnInit() {
		MH_CreateHook(Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MoleMole", "CriwareMediaPlayer", "Update", 0)),
			(LPVOID)hCriwareMediaPlayer_Update, (LPVOID*)&CriwareMediaPlayer_Update);
	}
}