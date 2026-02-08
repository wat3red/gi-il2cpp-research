#define IMGUI_DEFINE_MATH_OPERATORS

#include "auto_loot.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features
{
	void AutoLoot::DrawUI() {
		ImGuiEx::Checkbox("Enable autoloot", config.auto_loot.enabled);

		if (config.auto_loot.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Loot range", config.auto_loot.loot_range, 1.f, 10.f);
			ImGui::Unindent();
		}
	}

	void OnCheckIsInPosition(bool& result, MoleMole::BaseEntity* entity) {
		if (!config.auto_loot.enabled || !entity) return;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		auto avatar = entity_manager->GetAvatar();
		if (!avatar) return;

		float distance = avatar->GetRelativePosition().Distance(entity->GetRelativePosition());
		auto type = entity->GetType();

		if (type != MoleMole::EntityType::GatherObject &&
			type != MoleMole::EntityType::GatherPoint &&
			type != MoleMole::EntityType::HomeGatherObject &&
			type != MoleMole::EntityType::DropItem &&
			type != MoleMole::EntityType::EnvAnimal) {

			result = false;
			return;
		}

		result = distance <= config.auto_loot.loot_range;
	}

	void (*LCSelectPickup_AddInteeBtnById)(void* _this, MoleMole::BaseEntity* entity);
	void hLCSelectPickup_AddInteeBtnById(void* _this, MoleMole::BaseEntity* entity) {
		auto type = entity->GetType();
		auto id = entity->GetRuntimeID();

		Log("[AutoLoot] Added btn item id=%u, type=%d\n", id, type);

		if (config.auto_loot.enabled) {
			MoleMole::ItemModule* item_module = MoleMole::ItemModule::Instance();
			if (!item_module) return;

			if (type == MoleMole::EntityType::GatherObject ||
				type == MoleMole::EntityType::EnvAnimal ||
				type == MoleMole::EntityType::DropItem) {

				Log("[AutoLoot] Picking item id=%u\n", id);
				MoleMole::ActorUtils::SyncEntityPos(entity, 0, 0);
				item_module->PickItem(id);

				return;
			}
		}

		LCSelectPickup_AddInteeBtnById(_this, entity);
	}

	bool (*LCSelectPickup_IsInPosition)(void* _this, MoleMole::BaseEntity* entity);
	bool hLCSelectPickup_IsInPosition(void* _this, MoleMole::BaseEntity* entity) {
		bool result = LCSelectPickup_IsInPosition(_this, entity);

		OnCheckIsInPosition(result, entity);

		return result;
	}

	bool (*LCSelectPickup_IsOutPosition)(void* _this, MoleMole::BaseEntity* entity);
	bool hLCSelectPickup_IsOutPosition(void* _this, MoleMole::BaseEntity* entity) {
		bool result = LCSelectPickup_IsOutPosition(_this, entity);

		OnCheckIsInPosition(result, entity);

		return result;
	}

	void AutoLoot::OnInit() {
		// MoleMole.LCSelectPickup: "public Void [A-Z]{11}\(Nullable<Single> [A-Z]{11}, Nullable<Single> [A-Z]{11}\)"
		// private boolean [A-Z]{11}\([A-Z]{11} [A-Z]{11}\)
		MH_CreateHook((LPVOID)(Mem::Signature("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? E9 ? ? ? ? 48 8B 83").ScanXref()),
			hLCSelectPickup_IsInPosition, (LPVOID*)&LCSelectPickup_IsInPosition);
		MH_CreateHook((LPVOID)(Mem::Signature("E8 ? ? ? ? 84 C0 74 ? 48 8B 8E ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 89 DA").ScanXref()),
			hLCSelectPickup_IsOutPosition, (LPVOID*)&LCSelectPickup_IsOutPosition);
		MH_CreateHook((LPVOID)(Mem::Signature("E8 ? ? ? ? E9 ? ? ? ? 48 8B 81 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74 ? E8 ? ? ? ? 48 85 C0 0F 85 ? ? ? ? E9").ScanXref()),
			hLCSelectPickup_AddInteeBtnById, (LPVOID*)&LCSelectPickup_AddInteeBtnById); // private void [A-Z]{11}\([A-Z]{11} [A-Z]{11}\)
	}
}