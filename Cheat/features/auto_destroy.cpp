#include "auto_destroy.h"

namespace features
{
	void AutoDestroy::DrawUI() {
		ImGuiEx::Checkbox("Enable auto destroy", config.auto_destroy.enabled);

		if (config.auto_destroy.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Destroy range", config.auto_destroy.range, 1.f, 100.f);
			ImGui::Unindent();
		}
	}

	bool NeedsDestruction(Il2CppObject* _this) {
		if (!config.auto_destroy.enabled)
			return false;

		MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
		if (!entityManager) return false;

		// find class that contains "public static List<RuntimePlatform>", than search for 
		// "protected notserialized {found_class_name}"
		static int32_t offset = Mem::Signature("48 8B 41 ? 48 85 C0 74 ? 48 8B 80 ? ? ? ? 48 85 C0 74 ? 8B 50").FindDisp();
		MoleMole::BaseEntity* entity = *(MoleMole::BaseEntity**)((uintptr_t)_this + offset);

		if (entity->GetAbsolutePosition().Distance(entityManager->GetAvatar()->GetAbsolutePosition()) > config.auto_destroy.range)
			return false;

		if (entity->GetType() == MoleMole::EntityType::Avatar)
			return false;

		if (strstr(entity->GetName()->ToCStr(), "Clue"))
			return false;

		Log("entity name: %s, [type %d]\n", entity->GetName()->ToCStr(), entity->GetType());

		return true;
	}

	// MoleMole.LCAbilityElement : NOGDEOKCHDG

	// private void [A-z]{11}\(int32 [A-z]{11}, single [A-z]{11}, Nullable<Single> [A-z]{11}
	void (*LCAbilityElement_ReduceModifierDurability)(Il2CppObject* _this, int32_t modifierDurabilityIndex,
		float reduceDurability, System::Nullable<float> deltaTime);
	void hLCAbilityElement_ReduceModifierDurability(Il2CppObject* _this, int32_t modifierDurabilityIndex,
		float reduceDurability, System::Nullable<float> deltaTime) {

		if (NeedsDestruction(_this))
			reduceDurability = 1000.f;

		LCAbilityElement_ReduceModifierDurability(_this, modifierDurabilityIndex, reduceDurability, deltaTime);
	}

	void AutoDestroy::OnUpdate() {
		return;
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();

		for (auto* entity : entities) {
			if (!entity) continue;

			if (entity->GetType() != MoleMole::EntityType::Gear || strstr(entity->GetName()->ToCStr(), "ElemTablet")) continue;

			Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
			Unity::Vector3 entity_pos = entity->GetRelativePosition();

			float distance = avatar_pos.Distance(entity_pos);

			if (distance <= config.auto_destroy.range) {
				Log("[AutoDestroy] Entity ID: %d, type: %d\n", entity->GetRuntimeID(), (int)entity->GetType(), entity->GetName()->ToCStr());

				Il2CppObject* logic_component_manager = *(Il2CppObject**)((uintptr_t)entity + 0x200); // 0x110

				// LCBase является уникальным для всех сущностей


				if (!logic_component_manager) return;
				Log("[AutoDestroy] logic_component_manager: %p\n", logic_component_manager);

				//Il2CppObject* lc_base_combat = ComponentManager_GetComponent(logic_component_manager, Il2CppString::FromCStr("BCAPHHFEMJK"));
				//if (!lc_base_combat) return;
				//Log("[KillAura] lc_base_combat: %p\n", lc_base_combat);

				//LCBaseCombat_ChangeHP(lc_base_combat, 0.f);
				//LCBaseCombat_ChangeHP1(lc_base_combat, 0.f);
				//LCBaseCombat_UpdateCombatProp(lc_base_combat, 2, 0, 1);
				//Log("[KillAura] done\n");

				Il2CppObject* obj = ComponentManager_GetComponent(logic_component_manager, Il2CppString::FromCStr("IGMMANECCMN"));
				if (!obj) return;
				Log("[AutoDestroy] obj: %p\n", obj);

				///((void(*)(Il2CppObject * _this, int32_t type, float value, int32_t state))(g_game_base + 0x6E9B320))(obj, 2, 0, 1);
				//((void(*)(Il2CppObject * _this, float value))(g_game_base + 0x6E9B380))(obj, 0);
				//((void(*)(Il2CppObject * _this,  float value))(g_game_base + 0x6E9AEB0))(obj, 0);

				//((void(*)(Il2CppObject * _this, uint32_t killer, int32_t dieType))(g_game_base + 0x6E98B40))(obj, MoleMole::EntityManager::Instance()->GetAvatar()->GetRuntimeID(), 3);

				/*void* AttackResult = ((void* (*)(int32_t a, float b))(g_game_base + 0x80E9470))(1, 5.f);
				Il2CppClass* attackResultClass = Il2Cpp::Class::FromName("MoleMole", "AttackResult");
				Il2CppObject* o = il2cpp_object_new(attackResultClass);
				Log("[KillAura] done\n");*/
			}
		}
	}

	void AutoDestroy::OnInit() {
		MH_CreateHook(Mem::Signature("E8 ? ? ? ? 41 B7 ? 48 FF C3 49 8B 85").ScanXref(),
			(LPVOID)hLCAbilityElement_ReduceModifierDurability, (LPVOID*)&LCAbilityElement_ReduceModifierDurability);
	}
}