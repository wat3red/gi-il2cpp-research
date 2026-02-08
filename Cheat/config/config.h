#pragma once

#include "config_var.h"
#include "config_manager.h"

#include <windows.h>
#include <imgui/imgui.h>

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
