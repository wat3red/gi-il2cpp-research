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

	namespace {
		// How this offset was found:
		//   1. Find the class that contains "public static List<RuntimePlatform>"
		//   2. Search for "protected notserialized {found_class_name}"
		// The pattern below is the field-load sequence that produces BaseEntity*.
		int32_t EntityPtrOffset() {
			static const int32_t offset =
				Mem::Signature("48 8B 41 ? 48 85 C0 74 ? 48 8B 80 ? ? ? ? 48 85 C0 74 ? 8B 50").FindDisp();
			return offset;
		}

		bool NeedsDestruction(Il2CppObject* component) {
			if (!config.auto_destroy.enabled)
				return false;

			MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
			if (!entity_manager)
				return false;

			auto* entity = *reinterpret_cast<MoleMole::BaseEntity**>(
				reinterpret_cast<uintptr_t>(component) + EntityPtrOffset());
			if (!entity)
				return false;

			const float distance = entity->GetAbsolutePosition()
				.Distance(entity_manager->GetAvatar()->GetAbsolutePosition());
			if (distance > config.auto_destroy.range)
				return false;

			if (entity->GetType() == MoleMole::EntityType::Avatar)
				return false;

			const char* name = entity->GetName()->ToCStr();
			if (name && strstr(name, "Clue"))
				return false;

			return true;
		}

		// MoleMole.LCAbilityElement : NOGDEOKCHDG
		// Locate via:
		//   private void [A-z]{11}\(int32 [A-z]{11}, single [A-z]{11}, Nullable<Single> [A-z]{11}
		// Hooking ReduceModifierDurability and inflating the drain is enough to
		// pop nearby breakable objects without walking the entity list.
		void (*LCAbilityElement_ReduceModifierDurability)(Il2CppObject* _this, int32_t modifierDurabilityIndex,
			float reduceDurability, System::Nullable<float> deltaTime);

		void hLCAbilityElement_ReduceModifierDurability(Il2CppObject* _this, int32_t modifierDurabilityIndex,
			float reduceDurability, System::Nullable<float> deltaTime) {
			if (NeedsDestruction(_this))
				reduceDurability = 1000.f;

			LCAbilityElement_ReduceModifierDurability(_this, modifierDurabilityIndex, reduceDurability, deltaTime);
		}
	}

	void AutoDestroy::OnUpdate() {
	}

	void AutoDestroy::OnInit() {
		// E8 ? ? ? ? 41 B7 ? 48 FF C3 49 8B 85  → xref into ReduceModifierDurability
		MH_CreateHook(
			Mem::Signature("E8 ? ? ? ? 41 B7 ? 48 FF C3 49 8B 85").ScanXref(),
			reinterpret_cast<LPVOID>(hLCAbilityElement_ReduceModifierDurability),
			reinterpret_cast<LPVOID*>(&LCAbilityElement_ReduceModifierDurability));
	}
}
