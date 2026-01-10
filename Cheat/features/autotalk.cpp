#define IMGUI_DEFINE_MATH_OPERATORS

#include "autotalk.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features {
	void Autotalk::DrawUI() {
		ImGuiEx::Checkbox("Enable autotalk", config.autotalk.enabled);

		if (config.autotalk.enabled) {
			ImGui::Indent();
			ImGuiEx::Checkbox("Automatically choose reply", config.autotalk.auto_choose_reply);
			ImGui::Unindent();
		}
	}

	void Autotalk::DrawBackgroundUI() {}

	// "class MoleMole.InLevelCutScenePageContext " found via "private MonoInLevelCutScenePage " 
	// "class MoleMole.TalkDialogContext " found via "private MonoTalkDialog " 
	void (*MonoInLevelCutScenePageContext_UpdateView)(void* _this);
	void hMonoInLevelCutScenePageContext_UpdateView(void* _this) {
		void* talkDialogContext = *reinterpret_cast<void**>((uintptr_t)_this + 0x220);
		if (talkDialogContext != nullptr)
		{
			float* protectTime = reinterpret_cast<float*>((uintptr_t)talkDialogContext + 0x2F0);
			if (config.autotalk.enabled)
				*protectTime = 0.0f;
			else
				*protectTime = 0.4f;

			HDIKLPILBAC_CFDHCLDPCHI(_this);

			if (config.autotalk.auto_choose_reply && config.autotalk.enabled)
			{
				void* monoTalkDialog = *reinterpret_cast<void**>((uintptr_t)talkDialogContext + 0x2A8);
				if (monoTalkDialog != nullptr)
				{
					void* monoselectgrp = *reinterpret_cast<void**>((uintptr_t)monoTalkDialog + 0x48);
					if (monoselectgrp != nullptr)
					{
						void* monoReusableList = *reinterpret_cast<void**>((uintptr_t)monoselectgrp + 0x28);
						if (monoReusableList != nullptr)
						{
							auto item = Il2Cpp::Method::Call<void*>("MoleMole", "MonoReusableList", "get_Item", 1, monoReusableList, 0);
							if (item != nullptr)
							{
								Il2Cpp::Method::Call<void>("MoleMole", "MonoSelectItem", "OnSelectItem", 0, item);
							}
						}
					}
				}
			}
		}

		MonoInLevelCutScenePageContext_UpdateView(_this);
	}

	void (*MonoTypeWriter_Update)(void* this_);
	void hMonoTypeWriter_Update(void* this_)
	{
		float* _secondPerChar = reinterpret_cast<float*>((uintptr_t)this_ + 0x28);

		if (config.autotalk.enabled)
		{
			*_secondPerChar = 0.000001f;
		}
		else
		{
			*_secondPerChar = 0.03f;
		}

		return MonoTypeWriter_Update(this_);
	}

	void Autotalk::OnInit() {
		// "private MonoInLevelCutScenePage " 
		Mem::Signature sig("56 57 53 48 83 EC ? 0F 29 7C 24 ? 0F 29 74 24 ? 48 89 CE 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 48 8B BE");
		MH_CreateHook((LPVOID)(sig.Scan()), (LPVOID)hMonoInLevelCutScenePageContext_UpdateView, (LPVOID*)&MonoInLevelCutScenePageContext_UpdateView);
		
		auto MonoTypeWriter_Update_addr = (uintptr_t)Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MoleMole", "MonoTypewriter", "Update", 0));
		MH_CreateHook((LPVOID)(MonoTypeWriter_Update_addr), (LPVOID)hMonoTypeWriter_Update, (LPVOID*)&MonoTypeWriter_Update);
	}

	void Autotalk::OnUpdate() {}
}