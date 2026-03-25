#include "auto_loot.h"

namespace features
{
	void AutoLoot::DrawUI() {
		ImGuiEx::Checkbox("Enable autoloot", config.auto_loot.enabled);

		if (config.auto_loot.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Loot range", config.auto_loot.loot_range, 1.f, 50.f);

			ImGuiEx::Checkbox("Treasures", config.auto_loot.treasures);
			if (config.auto_loot.treasures) {
				ImGui::Indent();
				ImGuiEx::SliderFloat("Treasures pickup range", config.auto_loot.treasure_range, 1.f, 10.f);
				ImGui::Unindent();
			}

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

		if (type == MoleMole::EntityType::GatherObject ||
			type == MoleMole::EntityType::GatherPoint ||
			type == MoleMole::EntityType::HomeGatherObject ||
			type == MoleMole::EntityType::DropItem ||
			type == MoleMole::EntityType::EnvAnimal) {

			result = distance <= config.auto_loot.loot_range;
			return;
		}

		result = false;
		return;
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
				type == MoleMole::EntityType::GatherPoint ||
				type == MoleMole::EntityType::HomeGatherObject ||
				type == MoleMole::EntityType::DropItem ||
				type == MoleMole::EntityType::EnvAnimal) {

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

	void AutoLoot::OnUpdate() {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();

		for (auto* entity : entities) {
			if (!entity) continue;

			if (entity->GetType() != MoleMole::EntityType::Chest) continue;

			Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
			Unity::Vector3 entity_pos = entity->GetRelativePosition();

			float distance = avatar_pos.Distance(entity_pos);
			if (distance > config.auto_loot.treasure_range) continue;

			MoleMole::ItemModule* item_module = MoleMole::ItemModule::Instance();
			if (!item_module) return;

			item_module->PickItem(entity->GetRuntimeID());
		}
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