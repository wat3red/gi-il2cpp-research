#include "auto_talk.h"

namespace features
{
	void AutoTalk::DrawUI() {
		ImGuiEx::Checkbox("Enable autotalk", config.auto_talk.enabled);

		if (config.auto_talk.enabled) {
			ImGui::Indent();
			ImGuiEx::Checkbox("Automatically choose reply", config.auto_talk.auto_choose_reply);
			ImGuiEx::Checkbox("Fast dialog", config.auto_talk.fast_dialog);
			if (config.auto_talk.fast_dialog) {
				ImGui::Indent();
				ImGuiEx::SliderFloat("Speed##auto_talk", config.auto_talk.speed_modifier, 1.f, 5.f);
				ImGui::Unindent();
			}
			ImGui::Unindent();
		}
	}

	// "class MoleMole.InLevelCutScenePageContext " found via "private MonoInLevelCutScenePage "
	// "class MoleMole.TalkDialogContext " found via "private MonoTalkDialog "
	void (*InLevelCutScenePageContext_UpdateView)(void* _this);
	void hInLevelCutScenePageContext_UpdateView(void* _this) {
		if (!_this) return;

		//Log("\n");

		void* talkDialogContext = *(void**)((uintptr_t)_this + 0x250); // dynamic
		if (talkDialogContext) {

			// F3 0F 11 B6 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 98
			float* protectTime = (float*)((uintptr_t)talkDialogContext + 0x2D8); // dynamic
			if (config.auto_talk.enabled) {
				*protectTime = 0.0f;
				InLevelCutScenePageContext_OnFreeClick(_this);

				if (config.auto_talk.auto_choose_reply) {
					void* monoTalkDialog = *(void**)((uintptr_t)talkDialogContext + 0x220); // dynamic
					if (monoTalkDialog != nullptr) {
						void* monoGrpSelect = *(void**)((uintptr_t)monoTalkDialog + 0x48);
						if (monoGrpSelect != nullptr) {
							void* monoReusableList = *(void**)((uintptr_t)monoGrpSelect + 0x28);
							if (monoReusableList != nullptr) {
								void* item = Il2Cpp::Method::Call<void*>("MoleMole", "MonoReusableList", "get_Item", 1, monoReusableList, 0);
								if (item != nullptr) {
									Il2Cpp::Method::Call<void>("MoleMole", "MonoSelectItem", "OnSelectItem", 0, item);
								}
							}
						}
					}
				}
			}
			else
				*protectTime = 0.1f;
		}

		InLevelCutScenePageContext_UpdateView(_this);
	}

	void (*MonoTypeWriter_Update)(void* _this);
	void hMonoTypeWriter_Update(void* _this) {
		float* _secondPerChar = (float*)((uintptr_t)_this + 0x28);

		if (config.auto_talk.enabled) {
			*_secondPerChar = 0.000001f;
		}
		else {
			*_secondPerChar = 0.03f;
		}

		MonoTypeWriter_Update(_this);
	}

	bool previous_game_speed_enabled = config.game_speed.enabled;
	float previous_game_speed_speed = config.game_speed.speed;

	void (*InLevelCutScenePageContext_SetupView)(void* _this);
	void hInLevelCutScenePageContext_SetupView(void* _this) {
		if (config.auto_talk.enabled && config.auto_talk.fast_dialog) {
			previous_game_speed_enabled = config.game_speed.enabled;
			previous_game_speed_speed = config.game_speed.speed;

			config.game_speed.enabled = true;
			config.game_speed.speed = config.auto_talk.speed_modifier;
		}
		InLevelCutScenePageContext_SetupView(_this);
	}

	void (*InLevelCutScenePageContext_ClearView)(void* _this);
	void hInLevelCutScenePageContext_ClearView(void* _this) {
		if (config.auto_talk.enabled && config.auto_talk.fast_dialog) {
			config.game_speed.enabled = previous_game_speed_enabled;
			config.game_speed.speed = previous_game_speed_speed;
		}
		InLevelCutScenePageContext_ClearView(_this);
	}

	void AutoTalk::OnInit() {
		MH_CreateHook(Mem::Signature(
			"56 57 53 48 83 EC ? 0F 29 7C 24 ? 0F 29 74 24 ? 48 89 CE 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 48 8B BE").Scan(),
			hInLevelCutScenePageContext_UpdateView, (LPVOID*)&InLevelCutScenePageContext_UpdateView);

		MH_CreateHook(Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MoleMole", "MonoTypewriter", "Update", 0)),
			hMonoTypeWriter_Update, (LPVOID*)&MonoTypeWriter_Update);

		MH_CreateHook(Mem::Signature(
			"41 57 41 56 56 57 53 48 83 EC ? 48 89 CE 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 48 8B BE ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 0D").Scan(),
			hInLevelCutScenePageContext_SetupView, (LPVOID*)&InLevelCutScenePageContext_SetupView);

		MH_CreateHook(Mem::Signature(
			"56 57 53 48 83 EC ? 48 89 CE 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 48 8B 96 ? ? ? ? 48 85 D2 74 ? 48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 0F 84 ? ? ? ? E8 ? ? ? ? 48 C7 86 ? ? ? ? 00 00 00 00 48 8B 86 ? ? ? ? 48 85 C0 74").Scan(),
			hInLevelCutScenePageContext_ClearView, (LPVOID*)&InLevelCutScenePageContext_ClearView);
	}
}