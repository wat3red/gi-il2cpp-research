#define IMGUI_DEFINE_MATH_OPERATORS

#include "autotalk.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features
{
	void Autotalk::DrawUI() {
		ImGuiEx::Checkbox("Enable autotalk", config.autotalk.enabled);

		if (config.autotalk.enabled) {
			ImGui::Indent();
			ImGuiEx::Checkbox("Automatically choose reply", config.autotalk.auto_choose_reply);
			ImGui::Unindent();
		}
	}

	void Autotalk::DrawBackgroundUI() {}

	// "class MoleMole.InLevelCutScenePageContext " found via "private MonoInLevelCutScenePage " BPFLHGELEKM
	// "class MoleMole.TalkDialogContext " found via "private MonoTalkDialog " DBJCDAPMEPK
	void (*MonoInLevelCutScenePageContext_UpdateView)(void* _this);
	void hMonoInLevelCutScenePageContext_UpdateView(void* _this) {
		if (!_this) return;

		void* talkDialogContext = *(void**)((uintptr_t)_this + 0x250); // dynamic
		if (talkDialogContext != nullptr) {
			//Log("0x2B8 : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2B8));
			//Log("0x2D8 : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2D8));
			//Log("0x2DC : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2DC));
			//Log("0x2E8 : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2E8));
			//Log("0x2EC : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2EC));
			//Log("0x2F8 : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x2F8));
			//Log("0x308 : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x308)); 
			//Log("0x30C : %f\n", *(float*)((uintptr_t)talkDialogContext + 0x30C));

			// F3 0F 11 B6 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 98
			float* protectTime = (float*)((uintptr_t)talkDialogContext + 0x2D8); // dynamic
			if (config.autotalk.enabled) {
				*protectTime = 0.0f;
				HDIKLPILBAC_CFDHCLDPCHI(_this);
				if (config.autotalk.auto_choose_reply) {
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

		MonoInLevelCutScenePageContext_UpdateView(_this);
	}

	void (*MonoTypeWriter_Update)(void* this_);
	void hMonoTypeWriter_Update(void* this_) {
		float* _secondPerChar = (float*)((uintptr_t)this_ + 0x28);

		if (config.autotalk.enabled) {
			*_secondPerChar = 0.000001f;
		}
		else {
			*_secondPerChar = 0.03f;
		}

		return MonoTypeWriter_Update(this_);
	}

	void Autotalk::OnInit() {
		Mem::Signature sig("56 57 53 48 83 EC ? 0F 29 7C 24 ? 0F 29 74 24 ? 48 89 CE 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 48 8B BE");
		MH_CreateHook((LPVOID)(sig.Scan()), (LPVOID)hMonoInLevelCutScenePageContext_UpdateView, (LPVOID*)&MonoInLevelCutScenePageContext_UpdateView);

		auto MonoTypeWriter_Update_addr = (uintptr_t)Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MoleMole", "MonoTypewriter", "Update", 0));
		MH_CreateHook((LPVOID)(MonoTypeWriter_Update_addr), (LPVOID)hMonoTypeWriter_Update, (LPVOID*)&MonoTypeWriter_Update);
	}

	void Autotalk::OnUpdate() {}
}