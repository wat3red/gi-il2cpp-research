#include "quest_teleport.h"

#include <imgui/imgui.h>
#include <config/imgui_config.h>
#include <config/config.h>

namespace features
{
	bool is_teleporting = false;

	void QuestTeleport::DrawUI() {
		ImGuiEx::Checkbox("Quest Teleport", config.quest_teleport.enabled);
		if (config.quest_teleport.enabled) {
			ImGui::Indent();
			ImGuiEx::Hotkey("Hotkey", config.quest_teleport.teleport_hotkey);
			ImGui::Unindent();
		}
	}

	void QuestTeleport::OnUpdate() {
		MoleMole::MarkManager* mark_manager = MoleMole::MarkManager::Instance();
		if (!mark_manager) return;

		MoleMole::PlayerModule* player_module = MoleMole::PlayerModule::Instance();
		if (!player_module) return;

		MoleMole::GeneralMarkData* navigating_mark = mark_manager->GetNavigatingMark(player_module->GetCurSceneID());
		if (!navigating_mark) return;

		//Log("navigating_mark:%s\n", navigating_mark->_originPosition.ToString().c_str());

		if (is_teleporting) {
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			if (!entityManager) return;

			MoleMole::ActorUtils::SetAvatarPos(
				navigating_mark->_originPosition - Unity::Vector3(1.f)); // Slightly offset to prevent collision issues
			MoleMole::ActorUtils::SyncEntityPos(entityManager->GetAvatar(), 0, 0);
		}
	}

	void QuestTeleport::UpdateHotkeys() {
		is_teleporting = ImGui::IsKeyDown(config.quest_teleport.teleport_hotkey);
	}

	void QuestTeleport::OnInit() {}
}
