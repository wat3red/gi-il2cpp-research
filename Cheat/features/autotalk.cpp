#define IMGUI_DEFINE_MATH_OPERATORS

#include "autotalk.h"
#include "../sdk/types.h"
#include "../sdk/functions/resolve_funcs.h"
#include "../logger.h"
#include "../config/imgui_config.h"
#include "../config/config.h"

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

	// "class MoleMole.InLevelCutScenePageContext " found via "private MonoInLevelCutScenePage " HDIKLPILBAC
	// "class MoleMole.TalkDialogContext " found via "private MonoTalkDialog " KOCPCIJOAGC
	void (*MonoInLevelCutScenePageContext_UpdateView)(void* _this);
	void hMonoInLevelCutScenePageContext_UpdateView(void* _this) {
		void* talkDialogContext = *reinterpret_cast<void**>((uintptr_t)_this + 0x238);
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
							void* item = MonoReusableList_get_Item(monoReusableList, 0);
							if (item != nullptr)
							{
								MonoSelectItem_OnSelectItem(item);
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
		MH_CreateHook((LPVOID)(g_game_base_addr + 0xE6762F0), (LPVOID)hMonoInLevelCutScenePageContext_UpdateView, (LPVOID*)&MonoInLevelCutScenePageContext_UpdateView);
		MH_CreateHook((LPVOID)(g_game_base_addr + 0xA696820), (LPVOID)hMonoTypeWriter_Update, (LPVOID*)&MonoTypeWriter_Update);
	}

	void Autotalk::OnUpdate() {}
}