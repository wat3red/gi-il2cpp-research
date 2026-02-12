#pragma once

#include "config_var.h"
#include "config_manager.h"

#include <windows.h>
#include <imgui/imgui.h>
#include <map>
#include <vector>

class Config {
public:
	struct ESP {
		ConfigVar<bool> enabled{ ("esp"), ("enabled"), false };

		ConfigVar<bool> distance{ ("esp"), ("distance"), false }; // New: Draw distance number
		ConfigVar<float> maxDistance{ ("esp"), ("maxDistance"), 100.0f }; // New: Distance Limiter

		ConfigVar<bool> box2D{ ("esp"), ("box2d"), false };
		ConfigVar<ImColor> box2DColor{ ("esp"), ("box2DColor"), ImColor(255, 255, 255, 255) };

		ConfigVar<bool> box3D{ ("esp"), ("box3d"), false };
		ConfigVar<ImColor> box3DColor{ ("esp"), ("box3DColor"), ImColor(255, 255, 255, 255) };

		ConfigVar<bool> name{ ("esp"), ("name"), false };
		ConfigVar<ImColor> nameColor{ ("esp"), ("nameColor"), ImColor(255, 255, 255, 255) };
		ConfigVar<float> nameSize{ ("esp"), ("nameSize"), 16.f };

		ConfigVar<bool> line{ ("esp"), ("line"), false };
		ConfigVar<ImColor> lineColor{ ("esp"), ("lineColor"), ImColor(255, 255, 255, 255) };
		ConfigVar<int> lineBase{ ("esp"), ("lineBase"), 0 }; // 0: Bottom, 1: Center, 2: Top
		ConfigVar<int> lineTarget{ ("esp"), ("lineTarget"), 0 };
		ConfigVar<float> lineThickness{ ("esp"), ("lineThickness"), 2.f };

		// FILTERS
		// We map the Enum ID to a ConfigVar. 
		std::map<int32_t, ConfigVar<bool>> filters;

		ESP() {
			// Initialize filters for common entities. 
			// You can add more from your Enum list here.
			filters.emplace(2, ConfigVar<bool>(("esp_filter"), ("Monster"), true));
			filters.emplace(12, ConfigVar<bool>(("esp_filter"), ("NPC"), false));
			filters.emplace(26, ConfigVar<bool>(("esp_filter"), ("Chest"), true));
			filters.emplace(21, ConfigVar<bool>(("esp_filter"), ("GatherPoint"), true));
			filters.emplace(1, ConfigVar<bool>(("esp_filter"), ("Avatar"), false));
			filters.emplace(66, ConfigVar<bool>(("esp_filter"), ("BlackMud"), false));
		}
	} esp;

	struct AutoTalk {
		ConfigVar<bool> enabled{ "auto_talk", "enabled", false };
		ConfigVar<bool> auto_choose_reply{ "auto_talk", "auto_choose_reply", false };
		ConfigVar<bool> fast_dialog{ "auto_talk", "fast_dialog", false };
		ConfigVar<float> speed_modifier{ "auto_talk", "speed_modifier", 2.f };
	} auto_talk;

	struct GodMode {
		ConfigVar<bool> enabled{ "god_mode", "enabled", false };
	} god_mode;

	struct KillAura {
		ConfigVar<bool> enabled{ "kill_aura", "enabled", false };
		ConfigVar<float> range{ "kill_aura", "range", 5.f };
	} kill_aura;

	struct AutoLoot {
		ConfigVar<bool> enabled{ "auto_loot", "enabled", false };
		ConfigVar<float> loot_range{ "auto_loot", "loot_range", 5.f };
	} auto_loot;

	struct GameSpeed {
		ConfigVar<bool> enabled{ "game_speed", "enabled", false };
		ConfigVar<float> speed{ "game_speed", "speed", 1.f };
	} game_speed;

	struct MapTeleport {
		ConfigVar<bool> enabled{ "map_teleport", "enabled", false };
	} map_teleport;

	struct Noclip {
		ConfigVar<bool> enabled{ "noclip", "enabled", false };
		ConfigVar<float> speed{ "noclip", "speed", 5.f };
		ConfigVar<ImGuiKey> enable_hotkey{ "noclip", "enable_hotkey", ImGuiKey_V };
	} noclip;

	struct SkipCutscene {
		ConfigVar<bool> enabled{ "skip_cutscene", "enabled", false };
	} skip_cutscene;

	struct CostumeMapping {
		int id;
		uint32_t avatar_id;
		uint32_t costume_id;
		uint32_t flycloak_id;

		CostumeMapping(int _id, uint32_t _avatar_id, uint32_t _costume_id, uint32_t _flycloak_id)
			: id(_id), avatar_id(_avatar_id), costume_id(_costume_id), flycloak_id(_flycloak_id) {}
	};

	struct CostumeChanger {
		ConfigVar<bool> enabled{ "costume_changer", "enabled", false };
		ConfigVar<int>  mapping_count{ "costume_changer", "mapping_count", 0 };
		std::vector<CostumeMapping> mappings;

		void LoadMappings() {
			mappings.clear();
			const int count = mapping_count;
			for (int i = 0; i < count; i++) {
				const std::string section = "costume_changer.costumes.id_" + std::to_string(i);
				ConfigVar<uint32_t> av{ section, "avatar_id",   0u };
				ConfigVar<uint32_t> co{ section, "costume_id",  0u };
				ConfigVar<uint32_t> fl{ section, "flycloak_id", 0u };
				mappings.emplace_back(i, (uint32_t)av, (uint32_t)co, (uint32_t)fl);
			}
		}

		void SaveMappings() {
			mapping_count = (int)mappings.size();
			for (int i = 0; i < (int)mappings.size(); i++) {
				mappings[i].id = i;
				const std::string section = "costume_changer.costumes.id_" + std::to_string(i);
				ConfigVar<uint32_t>{ section, "avatar_id", 0u } = mappings[i].avatar_id;
				ConfigVar<uint32_t>{ section, "costume_id", 0u } = mappings[i].costume_id;
				ConfigVar<uint32_t>{ section, "flycloak_id", 0u } = mappings[i].flycloak_id;
			}
		}

		void SaveMapping(uint32_t avatar_id, uint32_t costume_id, uint32_t flycloak_id) {
			for (auto& m : mappings) {
				if (m.avatar_id == avatar_id) {
					m.costume_id = costume_id;
					m.flycloak_id = flycloak_id;
					SaveMappings();
					return;
				}
			}
			mappings.emplace_back((int)mappings.size(), avatar_id, costume_id, flycloak_id);
			SaveMappings();
		}

		void RemoveMapping(uint32_t avatar_id) {
			auto it = std::find_if(mappings.begin(), mappings.end(),
				[avatar_id](const CostumeMapping& m) { return m.avatar_id == avatar_id; });
			if (it != mappings.end()) {
				mappings.erase(it);
				SaveMappings();
			}
		}

		CostumeMapping* GetMapping(uint32_t avatar_id) {
			for (auto& m : mappings)
				if (m.avatar_id == avatar_id) return &m;
			return nullptr;
		}
	} costume_changer;

	struct SettingsUI {
		ConfigVar<int> theme{ ("settingsUI"), ("theme"), 0 };
		ConfigVar<float> menuScale{ ("settingsUI"), ("menuScale"), 1.f };

		struct Hotkeys {
			ConfigVar<int> open_menu{ ("settingsUI.hotkeys"), ("openMenu"), VK_INSERT };
			ConfigVar<int> esp{ ("settingsUI.hotkeys"), ("esp"), 0 };

		} hotkeys;
	} ui_settings;
};

extern Config config;
